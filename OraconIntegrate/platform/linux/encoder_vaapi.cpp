#include "oracon/integrate/desktop/encoder.h"
#include "oracon/core/logger.h"

#if defined(ORACON_PLATFORM_LINUX) && defined(ORACON_HAVE_FFMPEG)

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_vaapi.h>
#include <libswscale/swscale.h>
}

#include <va/va.h>
#include <va/va_drm.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <chrono>

namespace oracon {
namespace integrate {
namespace desktop {

bool VAAPIEncoder::isAvailable() {
    // Try to open DRM device
    int fd = open("/dev/dri/renderD128", O_RDWR);
    if (fd < 0) {
        return false;
    }

    // Try to initialize VAAPI
    VADisplay va_display = vaGetDisplayDRM(fd);
    if (!va_display) {
        close(fd);
        return false;
    }

    int major, minor;
    VAStatus va_status = vaInitialize(va_display, &major, &minor);
    if (va_status != VA_STATUS_SUCCESS) {
        close(fd);
        return false;
    }

    vaTerminate(va_display);
    close(fd);
    return true;
}

VAAPIEncoder::VAAPIEncoder()
    : m_va_display(nullptr)
    , m_codec_context(nullptr)
    , m_hw_frames_ctx(nullptr)
    , m_sw_frame(nullptr)
    , m_hw_frame(nullptr)
    , m_sws_context(nullptr)
    , m_encoding(false)
    , m_frame_count(0)
{}

VAAPIEncoder::~VAAPIEncoder() {
    stop();

    if (m_sws_context) {
        sws_freeContext(static_cast<SwsContext*>(m_sws_context));
        m_sws_context = nullptr;
    }

    if (m_hw_frame) {
        av_frame_free(reinterpret_cast<AVFrame**>(&m_hw_frame));
    }

    if (m_sw_frame) {
        av_frame_free(reinterpret_cast<AVFrame**>(&m_sw_frame));
    }

    if (m_hw_frames_ctx) {
        av_buffer_unref(reinterpret_cast<AVBufferRef**>(&m_hw_frames_ctx));
    }

    if (m_codec_context) {
        AVCodecContext* ctx = static_cast<AVCodecContext*>(m_codec_context);
        avcodec_free_context(&ctx);
    }

    // Note: VAAPI display is managed by FFmpeg's hwcontext
}

bool VAAPIEncoder::initialize(const EncoderConfig& config) {
    m_config = config;

    // Find VAAPI encoder
    const char* encoder_name = nullptr;
    switch (config.codec) {
        case Codec::H264:
            encoder_name = "h264_vaapi";
            break;
        case Codec::H265:
            encoder_name = "hevc_vaapi";
            break;
        case Codec::VP8:
            encoder_name = "vp8_vaapi";
            break;
        case Codec::VP9:
            encoder_name = "vp9_vaapi";
            break;
        default:
            ORACON_LOG_ERROR("Unsupported codec for VAAPI");
            return false;
    }

    const AVCodec* codec = avcodec_find_encoder_by_name(encoder_name);
    if (!codec) {
        ORACON_LOG_ERROR("Failed to find VAAPI encoder: ", encoder_name);
        return false;
    }

    // Create codec context
    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        ORACON_LOG_ERROR("Failed to allocate codec context");
        return false;
    }

    m_codec_context = ctx;

    // Set basic parameters
    ctx->width = config.width;
    ctx->height = config.height;
    ctx->time_base = {1, static_cast<int>(config.fps)};
    ctx->framerate = {static_cast<int>(config.fps), 1};
    ctx->gop_size = config.gop_size;
    ctx->max_b_frames = config.use_b_frames ? config.num_b_frames : 0;
    ctx->pix_fmt = AV_PIX_FMT_VAAPI;

    // Set bitrate
    if (config.rate_control == RateControlMode::CBR) {
        ctx->bit_rate = config.bitrate;
        ctx->rc_max_rate = config.bitrate;
        ctx->rc_min_rate = config.bitrate;
        ctx->rc_buffer_size = config.bitrate;
    } else {
        ctx->bit_rate = config.bitrate;
        if (config.max_bitrate > 0) {
            ctx->rc_max_rate = config.max_bitrate;
        }
    }

    // Create hardware device context
    AVBufferRef* hw_device_ctx = nullptr;
    int ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI,
                                      "/dev/dri/renderD128", nullptr, 0);
    if (ret < 0) {
        ORACON_LOG_ERROR("Failed to create VAAPI device context");
        avcodec_free_context(&ctx);
        m_codec_context = nullptr;
        return false;
    }

    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    av_buffer_unref(&hw_device_ctx);

    // VAAPI-specific options
    if (config.low_latency) {
        av_opt_set_int(ctx->priv_data, "async_depth", 1, 0);
    }

    // Quality settings
    if (config.rate_control == RateControlMode::CQP) {
        av_opt_set_int(ctx->priv_data, "qp", config.crf, 0);
    }

    // Create hardware frames context
    AVBufferRef* hw_frames_ref = av_hwframe_ctx_alloc(ctx->hw_device_ctx);
    if (!hw_frames_ref) {
        ORACON_LOG_ERROR("Failed to allocate VAAPI frames context");
        av_buffer_unref(&ctx->hw_device_ctx);
        avcodec_free_context(&ctx);
        m_codec_context = nullptr;
        return false;
    }

    AVHWFramesContext* frames_ctx = reinterpret_cast<AVHWFramesContext*>(hw_frames_ref->data);
    frames_ctx->format = AV_PIX_FMT_VAAPI;
    frames_ctx->sw_format = AV_PIX_FMT_NV12;  // Internal format
    frames_ctx->width = config.width;
    frames_ctx->height = config.height;
    frames_ctx->initial_pool_size = 20;

    if (av_hwframe_ctx_init(hw_frames_ref) < 0) {
        ORACON_LOG_ERROR("Failed to initialize VAAPI frames context");
        av_buffer_unref(&hw_frames_ref);
        av_buffer_unref(&ctx->hw_device_ctx);
        avcodec_free_context(&ctx);
        m_codec_context = nullptr;
        return false;
    }

    ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
    m_hw_frames_ctx = hw_frames_ref;

    // Open codec
    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        ORACON_LOG_ERROR("Failed to open VAAPI codec");
        av_buffer_unref(&ctx->hw_frames_ctx);
        av_buffer_unref(&ctx->hw_device_ctx);
        avcodec_free_context(&ctx);
        m_codec_context = nullptr;
        return false;
    }

    // Allocate software frame for input
    AVFrame* sw_frame = av_frame_alloc();
    if (!sw_frame) {
        ORACON_LOG_ERROR("Failed to allocate software frame");
        return false;
    }
    m_sw_frame = sw_frame;

    // Allocate hardware frame for GPU
    AVFrame* hw_frame = av_frame_alloc();
    if (!hw_frame) {
        ORACON_LOG_ERROR("Failed to allocate hardware frame");
        return false;
    }

    if (av_hwframe_get_buffer(hw_frames_ref, hw_frame, 0) < 0) {
        ORACON_LOG_ERROR("Failed to allocate VAAPI hardware frame buffer");
        av_frame_free(&hw_frame);
        return false;
    }

    m_hw_frame = hw_frame;

    ORACON_LOG_INFO("Initialized VAAPI encoder: ", config.width, "x", config.height,
                    " @ ", config.fps, " fps");

    return true;
}

bool VAAPIEncoder::start() {
    if (m_encoding) {
        return true;
    }

    if (!m_codec_context) {
        ORACON_LOG_ERROR("VAAPI encoder not initialized");
        return false;
    }

    m_encoding = true;
    m_stats = EncoderStats{};
    m_frame_count = 0;

    ORACON_LOG_INFO("Started VAAPI encoding");
    return true;
}

void VAAPIEncoder::stop() {
    if (!m_encoding) {
        return;
    }

    m_encoding = false;
    ORACON_LOG_INFO("Stopped VAAPI encoding");
}

std::unique_ptr<EncodedPacket> VAAPIEncoder::encodeFrame(const Frame& frame) {
    if (!m_encoding || !frame.isValid()) {
        return nullptr;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    AVCodecContext* ctx = static_cast<AVCodecContext*>(m_codec_context);
    AVFrame* sw_frame = static_cast<AVFrame*>(m_sw_frame);
    AVFrame* hw_frame = static_cast<AVFrame*>(m_hw_frame);

    // Determine source pixel format
    AVPixelFormat src_format;
    switch (frame.format) {
        case PixelFormat::BGRA32:
            src_format = AV_PIX_FMT_BGRA;
            break;
        case PixelFormat::RGBA32:
            src_format = AV_PIX_FMT_RGBA;
            break;
        case PixelFormat::BGR24:
            src_format = AV_PIX_FMT_BGR24;
            break;
        case PixelFormat::RGB24:
            src_format = AV_PIX_FMT_RGB24;
            break;
        default:
            ORACON_LOG_ERROR("Unsupported pixel format for VAAPI");
            m_stats.frames_dropped++;
            return nullptr;
    }

    // Create SwsContext if needed for conversion to NV12
    if (!m_sws_context) {
        SwsContext* sws_ctx = sws_getContext(
            frame.width, frame.height, src_format,
            m_config.width, m_config.height, AV_PIX_FMT_NV12,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );

        if (!sws_ctx) {
            ORACON_LOG_ERROR("Failed to create SwsContext for VAAPI");
            m_stats.frames_dropped++;
            return nullptr;
        }

        m_sws_context = sws_ctx;
    }

    // Set up software frame with captured data
    sw_frame->width = frame.width;
    sw_frame->height = frame.height;
    sw_frame->format = AV_PIX_FMT_NV12;

    // Allocate buffer for NV12 conversion
    if (av_frame_get_buffer(sw_frame, 0) < 0) {
        ORACON_LOG_ERROR("Failed to allocate software frame buffer");
        m_stats.frames_dropped++;
        return nullptr;
    }

    // Prepare source data for conversion
    uint8_t* src_data[4] = {const_cast<uint8_t*>(frame.data.data()), nullptr, nullptr, nullptr};
    int src_linesize[4] = {static_cast<int>(frame.stride), 0, 0, 0};

    // Convert from BGRA/RGB to NV12
    sws_scale(
        static_cast<SwsContext*>(m_sws_context),
        src_data,
        src_linesize,
        0,
        frame.height,
        sw_frame->data,
        sw_frame->linesize
    );

    // Upload frame to GPU (transfer to VAAPI surface)
    if (av_hwframe_transfer_data(hw_frame, sw_frame, 0) < 0) {
        ORACON_LOG_ERROR("Failed to transfer frame to VAAPI");
        av_frame_unref(sw_frame);
        m_stats.frames_dropped++;
        return nullptr;
    }

    av_frame_unref(sw_frame);

    // Set timestamp
    hw_frame->pts = m_frame_count++;

    // Allocate packet
    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        ORACON_LOG_ERROR("Failed to allocate packet");
        m_stats.frames_dropped++;
        return nullptr;
    }

    // Send frame to encoder
    int ret = avcodec_send_frame(ctx, hw_frame);
    if (ret < 0) {
        ORACON_LOG_ERROR("Failed to send frame to VAAPI encoder");
        av_packet_free(&pkt);
        m_stats.frames_dropped++;
        return nullptr;
    }

    // Receive encoded packet
    ret = avcodec_receive_packet(ctx, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        av_packet_free(&pkt);
        return nullptr;
    } else if (ret < 0) {
        ORACON_LOG_ERROR("Failed to receive packet from VAAPI encoder");
        av_packet_free(&pkt);
        m_stats.frames_dropped++;
        return nullptr;
    }

    // Create encoded packet
    auto packet = std::make_unique<EncodedPacket>();
    packet->data.resize(pkt->size);
    std::memcpy(packet->data.data(), pkt->data, pkt->size);
    packet->timestamp_ms = frame.timestamp_ms;
    packet->is_keyframe = (pkt->flags & AV_PKT_FLAG_KEY) != 0;
    packet->frame_number = m_frame_count - 1;

    // Update stats
    m_stats.frames_encoded++;
    m_stats.bytes_encoded += pkt->size;

    auto end_time = std::chrono::high_resolution_clock::now();
    auto encode_time = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time).count() / 1000.0f;

    m_stats.avg_encode_time_ms = (m_stats.avg_encode_time_ms * (m_stats.frames_encoded - 1) +
                                   encode_time) / m_stats.frames_encoded;

    av_packet_free(&pkt);

    return packet;
}

bool VAAPIEncoder::encodeFrameAsync(const Frame& frame, PacketCallback callback) {
    // Synchronous for now
    auto packet = encodeFrame(frame);
    if (packet && callback) {
        callback(*packet);
        return true;
    }
    return false;
}

std::vector<std::unique_ptr<EncodedPacket>> VAAPIEncoder::flush() {
    std::vector<std::unique_ptr<EncodedPacket>> packets;

    if (!m_codec_context) {
        return packets;
    }

    AVCodecContext* ctx = static_cast<AVCodecContext*>(m_codec_context);
    AVPacket* pkt = av_packet_alloc();

    // Send NULL to flush
    avcodec_send_frame(ctx, nullptr);

    // Receive all packets
    while (true) {
        int ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
            break;
        } else if (ret < 0) {
            break;
        }

        auto packet = std::make_unique<EncodedPacket>();
        packet->data.resize(pkt->size);
        std::memcpy(packet->data.data(), pkt->data, pkt->size);
        packet->is_keyframe = (pkt->flags & AV_PKT_FLAG_KEY) != 0;

        packets.push_back(std::move(packet));
        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    return packets;
}

bool VAAPIEncoder::isEncoding() const {
    return m_encoding;
}

EncoderStats VAAPIEncoder::getStats() const {
    return m_stats;
}

} // namespace desktop
} // namespace integrate
} // namespace oracon

#endif // ORACON_PLATFORM_LINUX && ORACON_HAVE_FFMPEG
