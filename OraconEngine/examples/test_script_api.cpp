#include "oracon/engine/engine.h"
#include <iostream>

using namespace oracon;
using namespace oracon::engine;

int main() {
    std::cout << "=== OraconEngine Script API Test ===\n";

    // Create minimal world
    World world;
    Entity* ball = world.createEntity("TestBall");

    // Add transform
    auto* transform = ball->addComponent<Transform>();
    transform->position = Vec2f(100.0f, 200.0f);

    // Add rigidbody
    auto* rb = ball->addComponent<Rigidbody>();
    rb->velocity = Vec2f(50.0f, 75.0f);

    // Simple script to test API
    String scriptCode = R"(
        log("=== Script Start ===");

        let pos = getPosition();
        log("Initial position: " + type(pos[0]) + ", " + type(pos[1]));

        let vel = getVelocity();
        log("Initial velocity: " + type(vel[0]) + ", " + type(vel[1]));

        // Modify position
        setPosition(150.0, 250.0);
        let newPos = getPosition();
        log("New position: " + type(newPos[0]) + ", " + type(newPos[1]));

        // Modify velocity
        setVelocity(100.0, 150.0);
        let newVel = getVelocity();
        log("New velocity: " + type(newVel[0]) + ", " + type(newVel[1]));

        log("=== Script End ===");
    )";

    auto* script = ball->addComponent<ScriptComponent>(scriptCode);
    script->onStart(ball, &world);

    if (script->hasErrors()) {
        std::cout << "Script errors:\n" << script->getErrors();
        return 1;
    }

    // Verify changes took effect
    std::cout << "Final position: " << transform->position.x << ", "
              << transform->position.y << "\n";
    std::cout << "Final velocity: " << rb->velocity.x << ", "
              << rb->velocity.y << "\n";

    std::cout << "Test completed successfully!\n";
    return 0;
}
