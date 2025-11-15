#include "oracon/integrate/desktop/stream.h"
#include "oracon/core/logger.h"
#include <chrono>
#include <thread>

namespace oracon {
namespace integrate {
namespace desktop {

DesktopStream::DesktopStream()
    : m_state(StreamState::Idle)
    , m_running(false)
{}

DesktopStream::~DesktopStream() {
    stop();
}

bool DesktopStream::initialize(const StreamConfig& config) {
    if (m_state != StreamState::Idle) {
        ORACON_LOG_ERROR("Stream already initialized");
        return false;
    }

    m_config = config;

    // Create capture instance
    m_capture = DesktopCapture::create();
    if (!m_capture) {
        ORACON_LOG_ERROR("Failed to create desktop capture");
        return false;
    }

    if (!m_capture->initialize(config.display_id)) {
        ORACON_LOG_ERROR("Failed to initialize desktop capture");
        return false;
    }

    // Create encoder with best available acceleration
    m_encoder = VideoEncoder::create(config.encoder_config.accel);
    if (!m_encoder) {
        ORACON_LOG_ERROR("Failed to create video encoder");
        return false;
    }

    // Get display resolution
    u32 width, height;
    m_capture->getDisplayResolution(config.display_id, width, height);

    // Update encoder config with actual resolution
    EncoderConfig encoder_config = config.encoder_config;
    encoder_config.width = width;
    encoder_config.height = height;
    encoder_config.fps = config.target_fps;

    if (!m_encoder->initialize(encoder_config)) {
        ORACON_LOG_ERROR("Failed to initialize video encoder");
        return false;
    }

    ORACON_LOG_INFO("Desktop stream initialized: ", width, "x", height,
                    " @ ", config.target_fps, " fps");

    return true;
}

bool DesktopStream::start() {
    if (m_state != StreamState::Idle) {
        ORACON_LOG_ERROR("Stream not in idle state");
        return false;
    }

    m_state = StreamState::Starting;

    // Start capture
    if (!m_capture->start()) {
        ORACON_LOG_ERROR("Failed to start desktop capture");
        m_state = StreamState::Error;
        return false;
    }

    // Start encoder
    if (!m_encoder->start()) {
        ORACON_LOG_ERROR("Failed to start video encoder");
        m_capture->stop();
        m_state = StreamState::Error;
        return false;
    }

    // Reset statistics
    m_stats = StreamStats{};
    m_start_time = std::chrono::steady_clock::now();
    m_last_stats_update = m_start_time;

    // Start worker threads
    m_running = true;
    m_capture_thread = std::thread(&DesktopStream::captureLoop, this);
    m_encode_thread = std::thread(&DesktopStream::encodeLoop, this);

    m_state = StreamState::Streaming;
    ORACON_LOG_INFO("Desktop streaming started");

    return true;
}

void DesktopStream::stop() {
    if (m_state == StreamState::Idle || m_state == StreamState::Stopping) {
        return;
    }

    m_state = StreamState::Stopping;
    m_running = false;

    // Wait for threads to finish
    if (m_capture_thread.joinable()) {
        m_capture_thread.join();
    }

    if (m_encode_thread.joinable()) {
        m_encode_thread.join();
    }

    // Stop capture and encoder
    if (m_capture) {
        m_capture->stop();
    }

    if (m_encoder) {
        m_encoder->stop();
    }

    // Clear frame queue
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        while (!m_frame_queue.empty()) {
            m_frame_queue.pop();
        }
    }

    m_state = StreamState::Idle;
    ORACON_LOG_INFO("Desktop streaming stopped");
}

void DesktopStream::pause() {
    if (m_state == StreamState::Streaming) {
        m_state = StreamState::Paused;
        ORACON_LOG_INFO("Desktop streaming paused");
    }
}

void DesktopStream::resume() {
    if (m_state == StreamState::Paused) {
        m_state = StreamState::Streaming;
        ORACON_LOG_INFO("Desktop streaming resumed");
    }
}

void DesktopStream::setPacketCallback(StreamPacketCallback callback) {
    std::lock_guard<std::mutex> lock(m_callback_mutex);
    m_packet_callback = callback;
}

StreamStats DesktopStream::getStats() const {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    return m_stats;
}

void DesktopStream::setTargetBitrate(u32 bitrate) {
    m_config.encoder_config.bitrate = bitrate;
    // TODO: Update encoder bitrate dynamically
}

void DesktopStream::setTargetFPS(u32 fps) {
    m_config.target_fps = fps;
}

void DesktopStream::requestKeyframe() {
    // TODO: Request keyframe from encoder
}

void DesktopStream::captureLoop() {
    ORACON_LOG_DEBUG("Capture loop started");

    using namespace std::chrono;
    auto frame_duration = microseconds(1000000 / m_config.target_fps);
    auto next_frame_time = steady_clock::now();

    while (m_running) {
        // Wait until next frame time
        std::this_thread::sleep_until(next_frame_time);
        next_frame_time += frame_duration;

        // Skip if paused
        if (m_state == StreamState::Paused) {
            continue;
        }

        // Capture frame
        auto frame = m_capture->captureFrame();
        if (!frame) {
            std::lock_guard<std::mutex> lock(m_stats_mutex);
            m_stats.frames_dropped++;
            continue;
        }

        // Update capture stats
        {
            std::lock_guard<std::mutex> lock(m_stats_mutex);
            m_stats.frames_captured++;
        }

        // Enqueue frame for encoding
        enqueueFrame(std::move(frame));
    }

    ORACON_LOG_DEBUG("Capture loop ended");
}

void DesktopStream::encodeLoop() {
    ORACON_LOG_DEBUG("Encode loop started");

    while (m_running) {
        // Dequeue frame
        auto frame = dequeueFrame();
        if (!frame) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        // Skip if paused
        if (m_state == StreamState::Paused) {
            continue;
        }

        // Encode frame
        auto packet = m_encoder->encodeFrame(*frame);
        if (!packet) {
            std::lock_guard<std::mutex> lock(m_stats_mutex);
            m_stats.frames_dropped++;
            continue;
        }

        // Update encoder stats
        {
            std::lock_guard<std::mutex> lock(m_stats_mutex);
            m_stats.frames_encoded++;
            m_stats.bytes_encoded += packet->getSize();
        }

        // Call packet callback
        {
            std::lock_guard<std::mutex> lock(m_callback_mutex);
            if (m_packet_callback) {
                m_packet_callback(*packet);
                std::lock_guard<std::mutex> stats_lock(m_stats_mutex);
                m_stats.packets_sent++;
            }
        }

        // Update statistics periodically
        updateStats();

        // Adaptive bitrate control
        if (m_config.adaptive_bitrate) {
            updateBitrate();
        }
    }

    ORACON_LOG_DEBUG("Encode loop ended");
}

void DesktopStream::enqueueFrame(std::unique_ptr<Frame> frame) {
    std::lock_guard<std::mutex> lock(m_queue_mutex);

    // Check buffer overflow
    if (m_frame_queue.size() >= m_config.buffer_size) {
        if (m_config.drop_frames_on_overflow) {
            // Drop oldest frame
            m_frame_queue.pop();
            std::lock_guard<std::mutex> stats_lock(m_stats_mutex);
            m_stats.frames_dropped++;
        } else {
            // Drop new frame
            std::lock_guard<std::mutex> stats_lock(m_stats_mutex);
            m_stats.frames_dropped++;
            return;
        }
    }

    m_frame_queue.push(std::move(frame));
}

std::unique_ptr<Frame> DesktopStream::dequeueFrame() {
    std::lock_guard<std::mutex> lock(m_queue_mutex);

    if (m_frame_queue.empty()) {
        return nullptr;
    }

    auto frame = std::move(m_frame_queue.front());
    m_frame_queue.pop();
    return frame;
}

void DesktopStream::updateBitrate() {
    // Simple adaptive bitrate based on frame queue size
    std::lock_guard<std::mutex> lock(m_queue_mutex);

    float queue_ratio = static_cast<float>(m_frame_queue.size()) / m_config.buffer_size;

    if (queue_ratio > 0.8f) {
        // Queue almost full - decrease bitrate
        u32 new_bitrate = m_config.encoder_config.bitrate * 0.9f;
        if (new_bitrate >= m_config.min_bitrate) {
            m_config.encoder_config.bitrate = new_bitrate;
        }
    } else if (queue_ratio < 0.2f) {
        // Queue mostly empty - increase bitrate
        u32 new_bitrate = m_config.encoder_config.bitrate * 1.1f;
        if (new_bitrate <= m_config.max_bitrate) {
            m_config.encoder_config.bitrate = new_bitrate;
        }
    }
}

void DesktopStream::updateStats() {
    using namespace std::chrono;

    auto now = steady_clock::now();
    auto elapsed = duration_cast<seconds>(now - m_last_stats_update);

    // Update stats every second
    if (elapsed.count() >= 1) {
        std::lock_guard<std::mutex> lock(m_stats_mutex);

        auto total_time = duration_cast<seconds>(now - m_start_time).count();
        if (total_time > 0) {
            m_stats.capture_fps = static_cast<float>(m_stats.frames_captured) / total_time;
            m_stats.encode_fps = static_cast<float>(m_stats.frames_encoded) / total_time;
            m_stats.avg_bitrate = (m_stats.bytes_encoded * 8.0f) / total_time;
        }

        m_stats.current_bitrate = (m_stats.bytes_encoded * 8.0f) / elapsed.count();

        // Get encoder stats
        if (m_encoder) {
            auto encoder_stats = m_encoder->getStats();
            m_stats.avg_encode_time_ms = encoder_stats.avg_encode_time_ms;
        }

        m_last_stats_update = now;
    }
}

} // namespace desktop
} // namespace integrate
} // namespace oracon
