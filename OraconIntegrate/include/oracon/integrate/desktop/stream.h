#ifndef ORACON_INTEGRATE_DESKTOP_STREAM_H
#define ORACON_INTEGRATE_DESKTOP_STREAM_H

#include "oracon/integrate/desktop/capture.h"
#include "oracon/integrate/desktop/encoder.h"
#include "oracon/core/types.h"
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>

namespace oracon {
namespace integrate {
namespace desktop {

using core::u32;
using core::u64;
using core::f32;
using core::String;

// Stream configuration
struct StreamConfig {
    // Capture settings
    u32 display_id = 0;

    // Encoder settings
    EncoderConfig encoder_config;

    // Streaming settings
    u32 target_fps = 30;
    bool adaptive_bitrate = true;
    u32 min_bitrate = 1000000;      // 1 Mbps
    u32 max_bitrate = 10000000;     // 10 Mbps

    // Buffer settings
    u32 buffer_size = 5;            // Number of frames to buffer
    bool drop_frames_on_overflow = true;
};

// Stream statistics
struct StreamStats {
    // Capture stats
    u64 frames_captured = 0;
    u64 frames_dropped = 0;
    f32 capture_fps = 0.0f;

    // Encoder stats
    u64 frames_encoded = 0;
    u64 bytes_encoded = 0;
    f32 encode_fps = 0.0f;
    f32 avg_encode_time_ms = 0.0f;

    // Streaming stats
    u64 packets_sent = 0;
    f32 current_bitrate = 0.0f;     // bits per second
    f32 avg_bitrate = 0.0f;

    // Performance
    f32 cpu_usage = 0.0f;
    f32 gpu_usage = 0.0f;
};

// Stream state
enum class StreamState {
    Idle,
    Starting,
    Streaming,
    Paused,
    Stopping,
    Error
};

// Packet callback for network transmission
using StreamPacketCallback = std::function<void(const EncodedPacket& packet)>;

// Desktop streaming class - integrates capture and encoding
class DesktopStream {
public:
    DesktopStream();
    ~DesktopStream();

    // Disable copy, allow move
    DesktopStream(const DesktopStream&) = delete;
    DesktopStream& operator=(const DesktopStream&) = delete;
    DesktopStream(DesktopStream&&) noexcept = default;
    DesktopStream& operator=(DesktopStream&&) noexcept = default;

    // Initialize stream with configuration
    bool initialize(const StreamConfig& config);

    // Start streaming
    bool start();

    // Stop streaming
    void stop();

    // Pause streaming (keeps capture running)
    void pause();

    // Resume from pause
    void resume();

    // Set packet callback for network transmission
    void setPacketCallback(StreamPacketCallback callback);

    // Status
    StreamState getState() const { return m_state; }
    bool isStreaming() const { return m_state == StreamState::Streaming; }
    StreamStats getStats() const;

    // Dynamic configuration
    void setTargetBitrate(u32 bitrate);
    void setTargetFPS(u32 fps);
    void requestKeyframe();

private:
    // Worker thread functions
    void captureLoop();
    void encodeLoop();

    // Frame queue management
    void enqueueFrame(std::unique_ptr<Frame> frame);
    std::unique_ptr<Frame> dequeueFrame();

    // Adaptive bitrate control
    void updateBitrate();

    // Statistics tracking
    void updateStats();

    std::unique_ptr<DesktopCapture> m_capture;
    std::unique_ptr<VideoEncoder> m_encoder;

    StreamConfig m_config;
    std::atomic<StreamState> m_state;

    // Threading
    std::thread m_capture_thread;
    std::thread m_encode_thread;
    std::atomic<bool> m_running;

    // Frame queue
    std::queue<std::unique_ptr<Frame>> m_frame_queue;
    std::mutex m_queue_mutex;

    // Packet callback
    StreamPacketCallback m_packet_callback;
    std::mutex m_callback_mutex;

    // Statistics
    mutable std::mutex m_stats_mutex;
    StreamStats m_stats;

    // Timing
    std::chrono::steady_clock::time_point m_start_time;
    std::chrono::steady_clock::time_point m_last_stats_update;
};

} // namespace desktop
} // namespace integrate
} // namespace oracon

#endif // ORACON_INTEGRATE_DESKTOP_STREAM_H
