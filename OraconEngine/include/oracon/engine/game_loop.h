#ifndef ORACON_ENGINE_GAME_LOOP_H
#define ORACON_ENGINE_GAME_LOOP_H

#include "oracon/engine/time.h"
#include "oracon/engine/scene.h"
#include "oracon/engine/input.h"
#include "oracon/gfx/canvas.h"
#include "oracon/gfx/renderer.h"

namespace oracon {
namespace engine {

using core::u32;
using core::f32;
using math::Vec2f;
using gfx::Canvas;
using gfx::Renderer;

class GameLoop {
public:
    GameLoop(u32 width, u32 height);
    virtual ~GameLoop() = default;
    
    virtual void onStart() {}
    virtual void onUpdate(f32 deltaTime) { (void)deltaTime; }
    virtual void onFixedUpdate(f32 fixedDeltaTime) { (void)fixedDeltaTime; }
    virtual void onRender(Renderer& renderer) { (void)renderer; }
    virtual void onShutdown() {}
    
    void run();
    void stop() { m_running = false; }
    
    Canvas* getCanvas() { return &m_canvas; }
    Scene* getScene() { return &m_scene; }
    Input* getInput() { return &m_input; }
    Time* getTime() { return &m_time; }

protected:
    bool m_running;
    Canvas m_canvas;
    Scene m_scene;
    Input m_input;
    Time m_time;
    Vec2f m_gravity{0.0f, 9.8f};
};

} // namespace engine
} // namespace oracon

#endif
