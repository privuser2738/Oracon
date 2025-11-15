#include "oracon/integrate/desktop/encoder.h"
#include "oracon/core/logger.h"
#include <algorithm>
#include <cstring>
#include <chrono>

// FFmpeg headers (will be conditionally included)
#ifdef ORACON_HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#endif

namespace oracon {
namespace integrate {
namespace desktop {

// Factory methods
std::vector<AccelType> VideoEncoder::getSupportedAcceleration() {
    std::vector<AccelType> supported;

    // Always support software encoding
    supported.push_back(AccelType::None);

#ifdef ORACON_HAVE_NVENC
    if (NVENCEncoder::isAvailable()) {
        supported.push_back(AccelType::NVENC);
    }
#endif

#ifdef ORACON_HAVE_AMF
    if (AMFEncoder::isAvailable()) {
        supported.push_back(AccelType::AMF);
        supported.push_back(AccelType::VCE);
    }
#endif

#ifdef ORACON_HAVE_QSV
    if (QSVEncoder::isAvailable()) {
        supported.push_back(AccelType::QSV);
    }
#endif

#ifdef ORACON_PLATFORM_LINUX
    if (VAAPIEncoder::isAvailable()) {
        supported.push_back(AccelType::VAAPI);
    }
#endif

#ifdef ORACON_PLATFORM_MACOS
    if (VideoToolboxEncoder::isAvailable()) {
        supported.push_back(AccelType::VideoToolbox);
    }
#endif

    return supported;
}

bool VideoEncoder::isAccelAvailable(AccelType accel) {
    auto supported = getSupportedAcceleration();
    return std::find(supported.begin(), supported.end(), accel) != supported.end();
}

std::unique_ptr<VideoEncoder> VideoEncoder::create(AccelType preferred) {
    // Try preferred first
    if (preferred != AccelType::None && isAccelAvailable(preferred)) {
        auto encoder = createWithAccel(preferred);
        if (encoder) {
            return encoder;
        }
    }

    // Try hardware acceleration in priority order
    auto supported = getSupportedAcceleration();
    for (auto accel : supported) {
        if (accel != AccelType::None) {
            auto encoder = createWithAccel(accel);
            if (encoder) {
                ORACON_LOG_INFO("Using hardware acceleration: ", static_cast<int>(accel));
                return encoder;
            }
        }
    }

    // Fallback to software encoding
    ORACON_LOG_INFO("Using software encoding");
    return std::make_unique<SoftwareEncoder>();
}

std::unique_ptr<VideoEncoder> VideoEncoder::createWithAccel(AccelType accel) {
    switch (accel) {
        case AccelType::None:
            return std::make_unique<SoftwareEncoder>();

#ifdef ORACON_HAVE_NVENC
        case AccelType::NVENC:
            if (NVENCEncoder::isAvailable()) {
                return std::make_unique<NVENCEncoder>();
            }
            break;
#endif

#ifdef ORACON_HAVE_AMF
        case AccelType::AMF:
        case AccelType::VCE:
            if (AMFEncoder::isAvailable()) {
                return std::make_unique<AMFEncoder>();
            }
            break;
#endif

#ifdef ORACON_HAVE_QSV
        case AccelType::QSV:
            if (QSVEncoder::isAvailable()) {
                return std::make_unique<QSVEncoder>();
            }
            break;
#endif

#ifdef ORACON_PLATFORM_LINUX
        case AccelType::VAAPI:
            if (VAAPIEncoder::isAvailable()) {
                return std::make_unique<VAAPIEncoder>();
            }
            break;
#endif

#ifdef ORACON_PLATFORM_MACOS
        case AccelType::VideoToolbox:
            if (VideoToolboxEncoder::isAvailable()) {
                return std::make_unique<VideoToolboxEncoder>();
            }
            break;
#endif

        default:
            break;
    }

    return nullptr;
}

// Software encoder implementation
#ifdef ORACON_HAVE_FFMPEG

SoftwareEncoder::SoftwareEncoder()
    : m_codec_context(nullptr)
    , m_frame(nullptr)
    , m_packet(nullptr)
    , m_sws_context(nullptr)
    , m_src_frame(nullptr)
    , m_encoding(false)
    , m_frame_count(0)
{}

SoftwareEncoder::~SoftwareEncoder() {
    stop();

    if (m_sws_context) {
        sws_freeContext(static_cast<SwsContext*>(m_sws_context));
        m_sws_context = nullptr;
    }

    if (m_src_frame) {
        av_frame_free(reinterpret_cast<AVFrame**>(&m_src_frame));
    }

    if (m_packet) {
        av_packet_free(reinterpret_cast<AVPacket**>(&m_packet));
    }

    if (m_frame) {
        av_frame_free(reinterpret_cast<AVFrame**>(&m_frame));
    }

    if (m_codec_context) {
        avcodec_free_context(reinterpret_cast<AVCodecContext**>(&m_codec_context));
    }
}

bool SoftwareEncoder::initialize(const EncoderConfig& config) {
    m_config = config;

    // Find codec
    const AVCodec* codec = nullptr;
    switch (config.codec) {
        case Codec::H264:
            codec = avcodec_find_encoder_by_name("libx264");
            break;
        case Codec::H265:
            codec = avcodec_find_encoder_by_name("libx265");
            break;
        case Codec::VP8:
            codec = avcodec_find_encoder_by_name("libvpx");
            break;
        case Codec::VP9:
            codec = avcodec_find_encoder_by_name("libvpx-vp9");
            break;
        case Codec::AV1:
            codec = avcodec_find_encoder_by_name("libaom-av1");
            break;
    }

    if (!codec) {
        ORACON_LOG_ERROR("Failed to find encoder codec");
        return false;
    }

    // Create codec context
    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        ORACON_LOG_ERROR("Failed to allocate codec context");
        return false;
    }

    m_codec_context = ctx;

    // Set parameters
    ctx->width = config.width;
    ctx->height = config.height;
    ctx->time_base = {1, static_cast<int>(config.fps)};
    ctx->framerate = {static_cast<int>(config.fps), 1};
    ctx->gop_size = config.gop_size;
    ctx->max_b_frames = config.use_b_frames ? config.num_b_frames : 0;
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    // Set bitrate
    if (config.rate_control == RateControlMode::CBR) {
        ctx->bit_rate = config.bitrate;
        ctx->rc_max_rate = config.bitrate;
        ctx->rc_min_rate = config.bitrate;
        ctx->rc_buffer_size = config.bitrate;
    } else {
        ctx->bit_rate = config.bitrate;
    }

    // Set preset
    const char* preset = "medium";
    switch (config.preset) {
        case EncoderPreset::UltraFast: preset = "ultrafast"; break;
        case EncoderPreset::SuperFast: preset = "superfast"; break;
        case EncoderPreset::VeryFast: preset = "veryfast"; break;
        case EncoderPreset::Faster: preset = "faster"; break;
        case EncoderPreset::Fast: preset = "fast"; break;
        case EncoderPreset::Medium: preset = "medium"; break;
        case EncoderPreset::Slow: preset = "slow"; break;
        case EncoderPreset::Slower: preset = "slower"; break;
        case EncoderPreset::VerySlow: preset = "veryslow"; break;
        case EncoderPreset::Placebo: preset = "placebo"; break;
    }
    av_opt_set(ctx->priv_data, "preset", preset, 0);

    // Set CRF for quality
    if (config.rate_control == RateControlMode::CQP) {
        char crf_str[16];
        snprintf(crf_str, sizeof(crf_str), "%u", config.crf);
        av_opt_set(ctx->priv_data, "crf", crf_str, 0);
    }

    // Low latency options
    if (config.low_latency) {
        av_opt_set(ctx->priv_data, "tune", "zerolatency", 0);
    }

    // Open codec
    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        ORACON_LOG_ERROR("Failed to open codec");
        avcodec_free_context(&ctx);
        m_codec_context = nullptr;
        return false;
    }

    // Allocate frame
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        ORACON_LOG_ERROR("Failed to allocate frame");
        return false;
    }

    frame->format = ctx->pix_fmt;
    frame->width = ctx->width;
    frame->height = ctx->height;

    if (av_frame_get_buffer(frame, 0) < 0) {
        ORACON_LOG_ERROR("Failed to allocate frame buffer");
        av_frame_free(&frame);
        return false;
    }

    m_frame = frame;

    // Allocate packet
    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        ORACON_LOG_ERROR("Failed to allocate packet");
        return false;
    }

    m_packet = pkt;

    // Allocate source frame for input (will be filled from captured frames)
    AVFrame* src_frame = av_frame_alloc();
    if (!src_frame) {
        ORACON_LOG_ERROR("Failed to allocate source frame");
        return false;
    }
    m_src_frame = src_frame;

    ORACON_LOG_INFO("Initialized software encoder: ", config.width, "x", config.height,
                    " @ ", config.fps, " fps");

    return true;
}

bool SoftwareEncoder::start() {
    if (m_encoding) {
        return true;
    }

    if (!m_codec_context) {
        ORACON_LOG_ERROR("Encoder not initialized");
        return false;
    }

    m_encoding = true;
    m_stats = EncoderStats{};
    m_frame_count = 0;

    ORACON_LOG_INFO("Started software encoding");
    return true;
}

void SoftwareEncoder::stop() {
    if (!m_encoding) {
        return;
    }

    m_encoding = false;
    ORACON_LOG_INFO("Stopped software encoding");
}

std::unique_ptr<EncodedPacket> SoftwareEncoder::encodeFrame(const Frame& frame) {
    if (!m_encoding || !frame.isValid()) {
        return nullptr;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    AVCodecContext* ctx = static_cast<AVCodecContext*>(m_codec_context);
    AVFrame* av_frame = static_cast<AVFrame*>(m_frame);
    AVFrame* src_frame = static_cast<AVFrame*>(m_src_frame);
    AVPacket* pkt = static_cast<AVPacket*>(m_packet);

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
            ORACON_LOG_ERROR("Unsupported pixel format for conversion");
            m_stats.frames_dropped++;
            return nullptr;
    }

    // Create SwsContext if needed or format changed
    if (!m_sws_context) {
        SwsContext* sws_ctx = sws_getContext(
            frame.width, frame.height, src_format,
            ctx->width, ctx->height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );

        if (!sws_ctx) {
            ORACON_LOG_ERROR("Failed to create SwsContext");
            m_stats.frames_dropped++;
            return nullptr;
        }

        m_sws_context = sws_ctx;
    }

    // Set up source frame
    src_frame->width = frame.width;
    src_frame->height = frame.height;
    src_frame->format = src_format;

    // Fill source frame with captured data
    av_image_fill_arrays(
        src_frame->data,
        src_frame->linesize,
        frame.data.data(),
        src_format,
        frame.width,
        frame.height,
        1
    );

    // Convert pixel format from source to YUV420P
    sws_scale(
        static_cast<SwsContext*>(m_sws_context),
        src_frame->data,
        src_frame->linesize,
        0,
        frame.height,
        av_frame->data,
        av_frame->linesize
    );

    av_frame->pts = m_frame_count++;

    // Send frame to encoder
    int ret = avcodec_send_frame(ctx, av_frame);
    if (ret < 0) {
        ORACON_LOG_ERROR("Failed to send frame to encoder");
        m_stats.frames_dropped++;
        return nullptr;
    }

    // Receive encoded packet
    ret = avcodec_receive_packet(ctx, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return nullptr;
    } else if (ret < 0) {
        ORACON_LOG_ERROR("Failed to receive packet from encoder");
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

    av_packet_unref(pkt);

    return packet;
}

bool SoftwareEncoder::encodeFrameAsync(const Frame& frame, PacketCallback callback) {
    // For now, just encode synchronously and call callback
    // In a real implementation, this would use a worker thread
    auto packet = encodeFrame(frame);
    if (packet && callback) {
        callback(*packet);
        return true;
    }
    return false;
}

std::vector<std::unique_ptr<EncodedPacket>> SoftwareEncoder::flush() {
    std::vector<std::unique_ptr<EncodedPacket>> packets;

    if (!m_codec_context) {
        return packets;
    }

    AVCodecContext* ctx = static_cast<AVCodecContext*>(m_codec_context);
    AVPacket* pkt = static_cast<AVPacket*>(m_packet);

    // Send NULL frame to flush
    avcodec_send_frame(ctx, nullptr);

    // Receive all remaining packets
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
        packet->frame_number = m_frame_count++;

        packets.push_back(std::move(packet));
        av_packet_unref(pkt);
    }

    return packets;
}

bool SoftwareEncoder::isEncoding() const {
    return m_encoding;
}

EncoderStats SoftwareEncoder::getStats() const {
    return m_stats;
}

#else // !ORACON_HAVE_FFMPEG

// Stub implementation when FFmpeg is not available
SoftwareEncoder::SoftwareEncoder()
    : m_codec_context(nullptr)
    , m_frame(nullptr)
    , m_packet(nullptr)
    , m_sws_context(nullptr)
    , m_src_frame(nullptr)
    , m_encoding(false)
    , m_frame_count(0)
{}

SoftwareEncoder::~SoftwareEncoder() {}

bool SoftwareEncoder::initialize(const EncoderConfig& config) {
    ORACON_LOG_ERROR("Software encoder not available - FFmpeg not found");
    return false;
}

bool SoftwareEncoder::start() { return false; }
void SoftwareEncoder::stop() {}
std::unique_ptr<EncodedPacket> SoftwareEncoder::encodeFrame(const Frame& frame) { return nullptr; }
bool SoftwareEncoder::encodeFrameAsync(const Frame& frame, PacketCallback callback) { return false; }
std::vector<std::unique_ptr<EncodedPacket>> SoftwareEncoder::flush() { return {}; }
bool SoftwareEncoder::isEncoding() const { return false; }
EncoderStats SoftwareEncoder::getStats() const { return {}; }

#endif // ORACON_HAVE_FFMPEG

} // namespace desktop
} // namespace integrate
} // namespace oracon
