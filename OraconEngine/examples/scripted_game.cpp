#include "oracon/engine/engine.h"
#include <iostream>

using namespace oracon;
using namespace oracon::engine;

class ScriptedGame : public GameLoop {
public:
    ScriptedGame() : GameLoop(800, 600) {}

    void onStart() override {
        std::cout << "Starting Scripted Game Demo...\n";

        // Create a scripted ball
        Entity* ball = getScene()->getWorld()->createEntity("ScriptedBall");

        // Add transform
        auto* transform = ball->addComponent<Transform>();
        transform->position = Vec2f(400.0f, 300.0f);

        // Add rigidbody
        auto* rb = ball->addComponent<Rigidbody>();
        rb->velocity = Vec2f(0.0f, 0.0f);
        rb->useGravity = false;

        // Add collider
        auto* collider = ball->addComponent<CircleCollider>();
        collider->radius = 20.0f;

        // Add sprite renderer
        auto* sprite = ball->addComponent<SpriteRenderer>();
        sprite->tint = gfx::Color::green();

        // Add script
        String scriptCode = R"(
            // Ball movement script
            let speed = 200.0;
            let bounceCount = 0;

            func update(dt) {
                // Get current position
                let pos = getPosition();
                let x = pos[0];
                let y = pos[1];

                // Get current velocity
                let vel = getVelocity();
                let vx = vel[0];
                let vy = vel[1];

                // Bounce off edges
                if (x < 20.0) {
                    vx = speed;
                    bounceCount = bounceCount + 1;
                    log("Left bounce! Count: " + type(bounceCount));
                }
                if (x > 780.0) {
                    vx = 0.0 - speed;
                    bounceCount = bounceCount + 1;
                    log("Right bounce! Count: " + type(bounceCount));
                }
                if (y < 20.0) {
                    vy = speed;
                    bounceCount = bounceCount + 1;
                    log("Top bounce! Count: " + type(bounceCount));
                }
                if (y > 580.0) {
                    vy = 0.0 - speed;
                    bounceCount = bounceCount + 1;
                    log("Bottom bounce! Count: " + type(bounceCount));
                }

                // Update velocity
                setVelocity(vx, vy);

                // Update position
                let newX = x + vx * dt;
                let newY = y + vy * dt;
                setPosition(newX, newY);
            }

            // Initialize with some velocity
            setVelocity(speed, speed * 0.7);
            log("Script initialized!");
        )";

        auto* script = ball->addComponent<ScriptComponent>(scriptCode);
        script->onStart(ball, getScene()->getWorld());

        std::cout << "Created scripted ball\n";

        // Check for errors
        if (script->hasErrors()) {
            std::cout << "Script errors:\n" << script->getErrors();
        }
    }

    void onUpdate(f32 deltaTime) override {
        // Update all script components
        for (auto& entity : getScene()->getWorld()->getEntities()) {
            if (!entity->isActive()) continue;

            auto* script = entity->getComponent<ScriptComponent>();
            if (script) {
                script->onUpdate(entity.get(), getScene()->getWorld(), deltaTime);
            }
        }
    }

    void onRender(Renderer& renderer) override {
        // Render all entities
        for (const auto& entity : getScene()->getWorld()->getEntities()) {
            if (!entity->isActive()) continue;

            auto* transform = entity->getComponent<Transform>();
            if (!transform) continue;

            Vec2f pos = transform->position;

            auto* circleCollider = entity->getComponent<CircleCollider>();
            if (circleCollider) {
                auto* sprite = entity->getComponent<SpriteRenderer>();
                gfx::Color color = sprite ? sprite->tint : gfx::Color::white();

                renderer.drawCircle(pos, circleCollider->radius, color, true);
            }
        }

        // Save frame periodically
        u32 frame = getTime()->frameCount();
        if (frame == 1 || frame == 60 || frame == 120) {
            String filename = "scripted_frame_" + std::to_string(frame) + ".ppm";
            getCanvas()->savePPM(filename);
            std::cout << "Saved " << filename << "\n";
        }
    }

    void onShutdown() override {
        std::cout << "Shutting down. Total frames: " << getTime()->frameCount() << "\n";
    }
};

int main() {
    std::cout << "=== OraconEngine Scripted Game Demo ===\n";

    ScriptedGame game;
    game.run();

    std::cout << "Demo completed successfully!\n";
    return 0;
}
