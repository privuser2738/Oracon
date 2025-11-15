#ifndef ORACON_INTEGRATE_DESKTOP_ENCODER_H
#define ORACON_INTEGRATE_DESKTOP_ENCODER_H

#include "oracon/integrate/desktop/capture.h"
#include "oracon/core/types.h"
#include <vector>
#include <memory>
#include <functional>

namespace oracon {
namespace integrate {
namespace desktop {

using core::u8;
using core::u16;
using core::u32;
using core::u64;
using core::f32;
using core::String;

// Video codec types
enum class Codec {
    H264,           // H.264/AVC
    H265,           // H.265/HEVC
    VP8,            // VP8
    VP9,            // VP9
    AV1             // AV1
};

// Hardware acceleration type
enum class AccelType {
    None,           // Software encoding
    NVENC,          // NVIDIA NVENC
    QSV,            // Intel Quick Sync Video
    VCE,            // AMD VCE (Video Coding Engine)
    AMF,            // AMD Advanced Media Framework
    VAAPI,          // Video Acceleration API (Linux)
    VideoToolbox,   // Apple VideoToolbox (macOS)
    ROCm            // AMD ROCm (Linux)
};

// Encoder preset (quality vs speed tradeoff)
enum class EncoderPreset {
    UltraFast,      // Fastest, lowest quality
    SuperFast,
    VeryFast,
    Faster,
    Fast,
    Medium,
    Slow,
    Slower,
    VerySlow,
    Placebo         // Slowest, highest quality
};

// Rate control mode
enum class RateControlMode {
    CQP,            // Constant Quantization Parameter
    CBR,            // Constant Bitrate
    VBR,            // Variable Bitrate
    ABR             // Average Bitrate
};

// Encoded packet
struct EncodedPacket {
    std::vector<u8> data;
    u64 timestamp_ms;
    bool is_keyframe;
    u32 frame_number;

    size_t getSize() const {
        return data.size();
    }

    bool isValid() const {
        return !data.empty();
    }
};

// Encoder configuration
struct EncoderConfig {
    Codec codec = Codec::H264;
    AccelType accel = AccelType::None;
    EncoderPreset preset = EncoderPreset::Medium;
    RateControlMode rate_control = RateControlMode::VBR;

    u32 width = 1920;
    u32 height = 1080;
    u32 fps = 30;
    u32 bitrate = 5000000;      // 5 Mbps
    u32 max_bitrate = 0;        // 0 = use bitrate value
    u32 gop_size = 60;          // Keyframe interval (frames)
    u32 crf = 23;               // Quality (0-51, lower = better)

    bool low_latency = false;
    bool use_b_frames = true;
    u32 num_b_frames = 2;
};

// Encoder statistics
struct EncoderStats {
    u64 frames_encoded = 0;
    u64 frames_dropped = 0;
    u64 bytes_encoded = 0;
    double avg_fps = 0.0;
    double avg_bitrate = 0.0;   // bits per second
    f32 avg_encode_time_ms = 0.0;
};

// Packet callback for streaming
using PacketCallback = std::function<void(const EncodedPacket& packet)>;

// Video encoder interface
class VideoEncoder {
public:
    virtual ~VideoEncoder() = default;

    // Initialize encoder with configuration
    virtual bool initialize(const EncoderConfig& config) = 0;

    // Start encoding
    virtual bool start() = 0;

    // Stop encoding
    virtual void stop() = 0;

    // Encode a single frame
    virtual std::unique_ptr<EncodedPacket> encodeFrame(const Frame& frame) = 0;

    // Encode frame asynchronously (calls callback when done)
    virtual bool encodeFrameAsync(const Frame& frame, PacketCallback callback) = 0;

    // Flush encoder (get remaining packets)
    virtual std::vector<std::unique_ptr<EncodedPacket>> flush() = 0;

    // Status
    virtual bool isEncoding() const = 0;
    virtual EncoderStats getStats() const = 0;
    virtual AccelType getAccelType() const = 0;

    // Get supported hardware acceleration
    static std::vector<AccelType> getSupportedAcceleration();

    // Check if specific acceleration is available
    static bool isAccelAvailable(AccelType accel);

    // Factory method - automatically selects best available encoder
    static std::unique_ptr<VideoEncoder> create(AccelType preferred = AccelType::None);

    // Factory method with explicit acceleration type
    static std::unique_ptr<VideoEncoder> createWithAccel(AccelType accel);
};

// Software encoder (x264/x265 via FFmpeg)
class SoftwareEncoder : public VideoEncoder {
public:
    SoftwareEncoder();
    ~SoftwareEncoder() override;

    bool initialize(const EncoderConfig& config) override;
    bool start() override;
    void stop() override;
    std::unique_ptr<EncodedPacket> encodeFrame(const Frame& frame) override;
    bool encodeFrameAsync(const Frame& frame, PacketCallback callback) override;
    std::vector<std::unique_ptr<EncodedPacket>> flush() override;
    bool isEncoding() const override;
    EncoderStats getStats() const override;
    AccelType getAccelType() const override { return AccelType::None; }

private:
    void* m_codec_context;      // AVCodecContext*
    void* m_frame;              // AVFrame*
    void* m_packet;             // AVPacket*
    void* m_sws_context;        // SwsContext* for pixel format conversion
    void* m_src_frame;          // AVFrame* for input frame
    bool m_encoding;
    EncoderConfig m_config;
    EncoderStats m_stats;
    u32 m_frame_count;
};

// NVIDIA NVENC encoder
#ifdef ORACON_HAVE_NVENC
class NVENCEncoder : public VideoEncoder {
public:
    NVENCEncoder();
    ~NVENCEncoder() override;

    bool initialize(const EncoderConfig& config) override;
    bool start() override;
    void stop() override;
    std::unique_ptr<EncodedPacket> encodeFrame(const Frame& frame) override;
    bool encodeFrameAsync(const Frame& frame, PacketCallback callback) override;
    std::vector<std::unique_ptr<EncodedPacket>> flush() override;
    bool isEncoding() const override;
    EncoderStats getStats() const override;
    AccelType getAccelType() const override { return AccelType::NVENC; }

    static bool isAvailable();

private:
    void* m_encoder;            // NV_ENC_ENCODER*
    void* m_cuda_context;       // CUcontext
    bool m_encoding;
    EncoderConfig m_config;
    EncoderStats m_stats;
};
#endif

// AMD VCE/AMF encoder
#ifdef ORACON_HAVE_AMF
class AMFEncoder : public VideoEncoder {
public:
    AMFEncoder();
    ~AMFEncoder() override;

    bool initialize(const EncoderConfig& config) override;
    bool start() override;
    void stop() override;
    std::unique_ptr<EncodedPacket> encodeFrame(const Frame& frame) override;
    bool encodeFrameAsync(const Frame& frame, PacketCallback callback) override;
    std::vector<std::unique_ptr<EncodedPacket>> flush() override;
    bool isEncoding() const override;
    EncoderStats getStats() const override;
    AccelType getAccelType() const override { return AccelType::AMF; }

    static bool isAvailable();

private:
    void* m_factory;            // AMFFactory*
    void* m_encoder;            // AMFComponent*
    bool m_encoding;
    EncoderConfig m_config;
    EncoderStats m_stats;
};
#endif

// Intel Quick Sync encoder
#ifdef ORACON_HAVE_QSV
class QSVEncoder : public VideoEncoder {
public:
    QSVEncoder();
    ~QSVEncoder() override;

    bool initialize(const EncoderConfig& config) override;
    bool start() override;
    void stop() override;
    std::unique_ptr<EncodedPacket> encodeFrame(const Frame& frame) override;
    bool encodeFrameAsync(const Frame& frame, PacketCallback callback) override;
    std::vector<std::unique_ptr<EncodedPacket>> flush() override;
    bool isEncoding() const override;
    EncoderStats getStats() const override;
    AccelType getAccelType() const override { return AccelType::QSV; }

    static bool isAvailable();

private:
    void* m_session;            // mfxSession
    void* m_encoder;            // MFXVideoENCODE*
    bool m_encoding;
    EncoderConfig m_config;
    EncoderStats m_stats;
};
#endif

// VAAPI encoder (Linux)
#ifdef ORACON_PLATFORM_LINUX
class VAAPIEncoder : public VideoEncoder {
public:
    VAAPIEncoder();
    ~VAAPIEncoder() override;

    bool initialize(const EncoderConfig& config) override;
    bool start() override;
    void stop() override;
    std::unique_ptr<EncodedPacket> encodeFrame(const Frame& frame) override;
    bool encodeFrameAsync(const Frame& frame, PacketCallback callback) override;
    std::vector<std::unique_ptr<EncodedPacket>> flush() override;
    bool isEncoding() const override;
    EncoderStats getStats() const override;
    AccelType getAccelType() const override { return AccelType::VAAPI; }

    static bool isAvailable();

private:
    void* m_va_display;         // VADisplay
    void* m_codec_context;      // AVCodecContext* with VAAPI
    void* m_hw_frames_ctx;      // AVBufferRef* hardware frames context
    void* m_sw_frame;           // AVFrame* software input frame
    void* m_hw_frame;           // AVFrame* hardware frame (GPU)
    void* m_sws_context;        // SwsContext* for pixel format conversion
    bool m_encoding;
    EncoderConfig m_config;
    EncoderStats m_stats;
    u32 m_frame_count;
};
#endif

// VideoToolbox encoder (macOS)
#ifdef ORACON_PLATFORM_MACOS
class VideoToolboxEncoder : public VideoEncoder {
public:
    VideoToolboxEncoder();
    ~VideoToolboxEncoder() override;

    bool initialize(const EncoderConfig& config) override;
    bool start() override;
    void stop() override;
    std::unique_ptr<EncodedPacket> encodeFrame(const Frame& frame) override;
    bool encodeFrameAsync(const Frame& frame, PacketCallback callback) override;
    std::vector<std::unique_ptr<EncodedPacket>> flush() override;
    bool isEncoding() const override;
    EncoderStats getStats() const override;
    AccelType getAccelType() const override { return AccelType::VideoToolbox; }

    static bool isAvailable();

private:
    void* m_session;            // VTCompressionSessionRef
    bool m_encoding;
    EncoderConfig m_config;
    EncoderStats m_stats;
};
#endif

} // namespace desktop
} // namespace integrate
} // namespace oracon

#endif // ORACON_INTEGRATE_DESKTOP_ENCODER_H
