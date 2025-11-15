#include "oracon/engine/game_loop.h"
#include "oracon/engine/physics.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace oracon {
namespace engine {

GameLoop::GameLoop(u32 width, u32 height)
    : m_running(false)
    , m_canvas(width, height)
    , m_scene("Main")
{}

void GameLoop::run() {
    m_running = true;
    
    const f32 targetFPS = 60.0f;
    const f32 targetFrameTime = 1.0f / targetFPS;
    const f32 fixedTimeStep = 1.0f / 60.0f;
    f32 accumulator = 0.0f;
    
    onStart();
    
    while (m_running) {
        m_time.update();
        m_input.update();
        
        f32 deltaTime = m_time.deltaTime();
        accumulator += deltaTime;
        
        // Fixed update for physics
        while (accumulator >= fixedTimeStep) {
            onFixedUpdate(fixedTimeStep);
            
            // Apply physics to all entities with Rigidbody
            for (auto& entity : m_scene.getWorld()->getEntities()) {
                if (entity->isActive()) {
                    PhysicsSystem::applyPhysics(entity.get(), fixedTimeStep, m_gravity);
                }
            }
            
            accumulator -= fixedTimeStep;
        }
        
        // Variable update
        onUpdate(deltaTime);
        
        // Render
        Renderer renderer(&m_canvas);
        renderer.clear(gfx::Color::black());
        onRender(renderer);
        
        // Frame limiting
        f32 frameTime = m_time.deltaTime();
        if (frameTime < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::duration<f32>(targetFrameTime - frameTime)
            );
        }
        
        // Simple exit condition for headless testing
        if (m_time.frameCount() > 300) break;
    }
    
    onShutdown();
}

} // namespace engine
} // namespace oracon
