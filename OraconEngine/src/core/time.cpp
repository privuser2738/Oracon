#include "oracon/engine/time.h"

namespace oracon {
namespace engine {

Time::Time()
    : m_deltaTime(0.0f)
    , m_deltaTimeDouble(0.0)
    , m_totalTime(0.0f)
    , m_totalTimeDouble(0.0)
    , m_timeScale(1.0f)
    , m_frameCount(0)
    , m_fps(0.0f)
    , m_fpsAccumulator(0.0f)
    , m_fpsFrameCount(0)
{
    m_startTime = Clock::now();
    m_lastFrameTime = m_startTime;
    m_currentFrameTime = m_startTime;
    m_lastFpsUpdate = m_startTime;
}

void Time::update() {
    m_currentFrameTime = Clock::now();

    // Calculate delta time
    Duration delta = m_currentFrameTime - m_lastFrameTime;
    m_deltaTimeDouble = delta.count();
    m_deltaTime = static_cast<f32>(m_deltaTimeDouble);

    // Update total time
    Duration total = m_currentFrameTime - m_startTime;
    m_totalTimeDouble = total.count();
    m_totalTime = static_cast<f32>(m_totalTimeDouble);

    // Update frame count
    m_frameCount++;

    // FPS calculation (update every second)
    m_fpsAccumulator += m_deltaTime;
    m_fpsFrameCount++;

    Duration fpsDelta = m_currentFrameTime - m_lastFpsUpdate;
    if (fpsDelta.count() >= 1.0) {
        m_fps = static_cast<f32>(m_fpsFrameCount) / m_fpsAccumulator;
        m_fpsAccumulator = 0.0f;
        m_fpsFrameCount = 0;
        m_lastFpsUpdate = m_currentFrameTime;
    }

    // Update last frame time for next iteration
    m_lastFrameTime = m_currentFrameTime;
}

void Time::reset() {
    m_startTime = Clock::now();
    m_lastFrameTime = m_startTime;
    m_currentFrameTime = m_startTime;
    m_lastFpsUpdate = m_startTime;

    m_deltaTime = 0.0f;
    m_deltaTimeDouble = 0.0;
    m_totalTime = 0.0f;
    m_totalTimeDouble = 0.0;
    m_frameCount = 0;
    m_fps = 0.0f;
    m_fpsAccumulator = 0.0f;
    m_fpsFrameCount = 0;
}

} // namespace engine
} // namespace oracon
