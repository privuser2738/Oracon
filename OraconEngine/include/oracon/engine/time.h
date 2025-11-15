#ifndef ORACON_ENGINE_TIME_H
#define ORACON_ENGINE_TIME_H

#include "oracon/core/types.h"
#include <chrono>

namespace oracon {
namespace engine {

using core::f32;
using core::f64;
using core::u32;
using core::u64;

// High-resolution time management for game loop
class Time {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<f64>;

    Time();

    // Update time (call once per frame)
    void update();

    // Delta time since last frame (in seconds)
    f32 deltaTime() const { return m_deltaTime; }
    f64 deltaTimeDouble() const { return m_deltaTimeDouble; }

    // Total elapsed time since start (in seconds)
    f32 totalTime() const { return m_totalTime; }
    f64 totalTimeDouble() const { return m_totalTimeDouble; }

    // Frame count
    u64 frameCount() const { return m_frameCount; }

    // FPS calculation
    f32 fps() const { return m_fps; }

    // Time scale (for slow-motion/fast-forward effects)
    void setTimeScale(f32 scale) { m_timeScale = scale; }
    f32 getTimeScale() const { return m_timeScale; }

    // Scaled delta time (affected by time scale)
    f32 scaledDeltaTime() const { return m_deltaTime * m_timeScale; }

    // Reset timer
    void reset();

private:
    TimePoint m_startTime;
    TimePoint m_lastFrameTime;
    TimePoint m_currentFrameTime;

    f32 m_deltaTime;
    f64 m_deltaTimeDouble;
    f32 m_totalTime;
    f64 m_totalTimeDouble;
    f32 m_timeScale;
    u64 m_frameCount;

    // FPS tracking
    f32 m_fps;
    f32 m_fpsAccumulator;
    u32 m_fpsFrameCount;
    TimePoint m_lastFpsUpdate;
};

} // namespace engine
} // namespace oracon

#endif // ORACON_ENGINE_TIME_H
