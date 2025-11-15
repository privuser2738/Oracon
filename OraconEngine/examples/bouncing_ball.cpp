#include "oracon/engine/engine.h"
#include <iostream>
#include <unordered_map>

using namespace oracon;
using namespace oracon::engine;

class BouncingBallGame : public GameLoop {
public:
    BouncingBallGame() : GameLoop(800, 600) {}

private:
    std::unordered_map<String, gfx::Color> m_ballColors;

public:
    void onStart() override {
        std::cout << "Starting Bouncing Ball Demo...\n";

        // Create ground
        Entity* ground = getScene()->getWorld()->createEntity("Ground");
        auto* groundTransform = ground->addComponent<Transform>();
        groundTransform->position = Vec2f(400.0f, 550.0f);

        auto* groundCollider = ground->addComponent<BoxCollider>();
        groundCollider->size = Vec2f(800.0f, 50.0f);

        auto* groundTag = ground->addComponent<Tag>("ground");
        (void)groundTag;

        // Create bouncing balls
        createBall("Ball1", Vec2f(100.0f, 100.0f), Vec2f(50.0f, 20.0f), gfx::Color::red());
        createBall("Ball2", Vec2f(300.0f, 50.0f), Vec2f(30.0f, 10.0f), gfx::Color::green());
        createBall("Ball3", Vec2f(500.0f, 150.0f), Vec2f(-40.0f, 30.0f), gfx::Color::blue());
        createBall("Ball4", Vec2f(700.0f, 200.0f), Vec2f(-60.0f, -10.0f), gfx::Color::yellow());

        std::cout << "Created " << getScene()->getWorld()->getEntities().size() << " entities\n";
    }

    void createBall(const String& name, const Vec2f& pos, const Vec2f& vel, const gfx::Color& color) {
        Entity* ball = getScene()->getWorld()->createEntity(name);

        auto* transform = ball->addComponent<Transform>();
        transform->position = pos;

        auto* rb = ball->addComponent<Rigidbody>();
        rb->velocity = vel;
        rb->mass = 1.0f;
        rb->useGravity = true;
        rb->drag = 0.01f;
        rb->bounciness = 0.8f;

        auto* collider = ball->addComponent<CircleCollider>();
        collider->radius = 20.0f;

        auto* tag = ball->addComponent<Tag>(name);
        (void)tag;

        m_ballColors[name] = color;
    }

    void onUpdate(f32 deltaTime) override {
        (void)deltaTime;

        // Check collisions and apply bounce
        auto& entities = getScene()->getWorld()->getEntities();
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                if (PhysicsSystem::checkCollision(entities[i].get(), entities[j].get())) {
                    resolveCollision(entities[i].get(), entities[j].get());
                }
            }
        }

        // Keep balls in bounds with bounce
        for (auto& entity : entities) {
            auto* transform = entity->getComponent<Transform>();
            auto* rb = entity->getComponent<Rigidbody>();
            auto* collider = entity->getComponent<CircleCollider>();

            if (transform && rb && collider) {
                // Bottom bounce
                if (transform->position.y + collider->radius > 575.0f) {
                    transform->position.y = 575.0f - collider->radius;
                    rb->velocity.y = -rb->velocity.y * rb->bounciness;
                }
                // Top bounce
                if (transform->position.y - collider->radius < 0.0f) {
                    transform->position.y = collider->radius;
                    rb->velocity.y = -rb->velocity.y * rb->bounciness;
                }
                // Left bounce
                if (transform->position.x - collider->radius < 0.0f) {
                    transform->position.x = collider->radius;
                    rb->velocity.x = -rb->velocity.x * rb->bounciness;
                }
                // Right bounce
                if (transform->position.x + collider->radius > 800.0f) {
                    transform->position.x = 800.0f - collider->radius;
                    rb->velocity.x = -rb->velocity.x * rb->bounciness;
                }
            }
        }
    }

    void resolveCollision(Entity* a, Entity* b) {
        auto* rbA = a->getComponent<Rigidbody>();
        auto* rbB = b->getComponent<Rigidbody>();

        if (!rbA || !rbB) return;

        // Simple elastic collision - swap velocities
        Vec2f tempVel = rbA->velocity;
        rbA->velocity = rbB->velocity * 0.8f;
        rbB->velocity = tempVel * 0.8f;
    }

    void onRender(Renderer& renderer) override {
        // Render all entities
        for (const auto& entity : getScene()->getWorld()->getEntities()) {
            if (!entity->isActive()) continue;

            auto* transform = entity->getComponent<Transform>();
            if (!transform) continue;

            Vec2f pos = transform->position;

            // Check for circle collider (balls)
            auto* circleCollider = entity->getComponent<CircleCollider>();
            if (circleCollider) {
                auto it = m_ballColors.find(entity->getName());
                gfx::Color color = (it != m_ballColors.end()) ? it->second : gfx::Color::white();

                renderer.drawCircle(pos, circleCollider->radius, color, true);
            }

            // Check for box collider (ground)
            auto* boxCollider = entity->getComponent<BoxCollider>();
            if (boxCollider) {
                renderer.drawRect(
                    static_cast<i32>(pos.x - boxCollider->size.x / 2),
                    static_cast<i32>(pos.y - boxCollider->size.y / 2),
                    static_cast<u32>(boxCollider->size.x),
                    static_cast<u32>(boxCollider->size.y),
                    gfx::Color(100, 100, 100, 255)
                );
            }
        }

        // Save frame periodically
        u32 frame = getTime()->frameCount();
        if (frame == 1 || frame == 30 || frame == 60 || frame == 90) {
            String filename = "frame_" + std::to_string(frame) + ".ppm";
            getCanvas()->savePPM(filename);
            std::cout << "Saved " << filename << "\n";
        }
    }

    void onShutdown() override {
        std::cout << "Shutting down. Total frames: " << getTime()->frameCount() << "\n";
    }
};

int main() {
    std::cout << "=== OraconEngine Bouncing Ball Demo ===\n";

    BouncingBallGame game;
    game.run();

    std::cout << "Demo completed successfully!\n";
    return 0;
}
