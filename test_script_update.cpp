#include "oracon/engine/engine.h"
#include <iostream>

using namespace oracon;
using namespace oracon::engine;

class TestUpdateGame : public GameLoop {
public:
    TestUpdateGame() : GameLoop(800, 600) {}

    void onStart() override {
        std::cout << "Creating test entity with update script...\n";

        Entity* ball = getScene()->getWorld()->createEntity("TestBall");

        auto* transform = ball->addComponent<Transform>();
        transform->position = Vec2f(400.0f, 300.0f);

        auto* rb = ball->addComponent<Rigidbody>();
        rb->velocity = Vec2f(100.0f, 100.0f);
        rb->useGravity = false;

        // Simple update test script
        String scriptCode = R"(
            let frameCount = 0;

            func update(dt) {
                frameCount = frameCount + 1;
                if (frameCount < 10) {
                    log("Update called: frame " + type(frameCount) + ", dt=" + type(dt));
                }

                let pos = getPosition();
                let x = pos[0];
                let y = pos[1];

                if (frameCount == 5) {
                    log("Position at frame 5: x=" + type(x) + ", y=" + type(y));
                }

                // Simple movement
                let newX = x + 1.0;
                let newY = y + 1.0;
                setPosition(newX, newY);
            }

            log("Script initialized!");
        )";

        auto* script = ball->addComponent<ScriptComponent>(scriptCode);
        script->onStart(ball, getScene()->getWorld());

        if (script->hasErrors()) {
            std::cout << "Script errors:\n" << script->getErrors();
        }

        m_ball = ball;
        m_script = script;
    }

    void onUpdate(f32 deltaTime) override {
        // Update script
        if (m_script && m_ball) {
            m_script->onUpdate(m_ball, getScene()->getWorld(), deltaTime);
        }
    }

    void onShutdown() override {
        auto* transform = m_ball->getComponent<Transform>();
        std::cout << "Final position: " << transform->position.x
                  << ", " << transform->position.y << "\n";
        std::cout << "Total frames: " << getTime()->frameCount() << "\n";
    }

private:
    Entity* m_ball = nullptr;
    ScriptComponent* m_script = nullptr;
};

int main() {
    std::cout << "=== Script Update Test ===\n";

    TestUpdateGame game;
    game.run();

    std::cout << "Test completed!\n";
    return 0;
}
