#include "oracon/engine/engine.h"
#include "oracon/auto/auto.h"
#include "oracon/gfx/gfx.h"
#include "oracon/gfx/window.h"
#include "oracon/gfx/text.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace oracon;
using namespace oracon::engine;
using namespace oracon::gfx;
using namespace oracon::auto_ns;
using namespace oracon::math;

// Game state
struct GameState {
    int playerHealth = 100;
    int playerGold = 50;
    String playerLocation = "village square";
    String currentWeather = "sunny";
    String timeOfDay = "afternoon";

    String describe() const {
        std::stringstream ss;
        ss << "Location: " << playerLocation << "\n";
        ss << "Weather: " << currentWeather << ", Time: " << timeOfDay << "\n";
        ss << "Player health: " << playerHealth << "/100, Gold: " << playerGold;
        return ss.str();
    }
};

// Visual NPC representation
struct VisualNPC {
    Entity* entity;
    AIBehavior* ai;
    Color color;
    String currentSpeech;
    f32 speechTimer = 0.0f;
    f32 wanderTimer = 0.0f;
    Vec2f targetPos;
    bool isSpeaking = false;
    bool isThinking = false;
};

// Draw a simple NPC sprite (circle with face)
void drawNPC(Canvas& canvas, const Vec2f& pos, const Color& color, bool thinking) {
    Renderer renderer(&canvas);

    // Shadow
    renderer.fillCircle(Circle(Vec2f(pos.x + 2, pos.y + 2), 18, Color(0, 0, 0, 64)));

    // Body
    renderer.fillCircle(Circle(pos, 20, color));

    // Outline
    renderer.drawCircle(pos, 20, Color::black(), false);

    // Eyes
    Color eyeColor = thinking ? Color(100, 100, 255) : Color::white();
    renderer.fillCircle(Circle(Vec2f(pos.x - 7, pos.y - 5), 4, eyeColor));
    renderer.fillCircle(Circle(Vec2f(pos.x + 7, pos.y - 5), 4, eyeColor));

    // Pupils
    renderer.fillCircle(Circle(Vec2f(pos.x - 7, pos.y - 5), 2, Color::black()));
    renderer.fillCircle(Circle(Vec2f(pos.x + 7, pos.y - 5), 2, Color::black()));

    // Mouth (smile or thinking)
    if (thinking) {
        // Wavy mouth (thinking)
        for (i32 i = -6; i <= 6; i++) {
            i32 dy = static_cast<i32>(std::sin(i * 0.5f) * 2);
            canvas.setPixel(static_cast<u32>(pos.x + i), static_cast<u32>(pos.y + 7 + dy), Color::black());
        }
    } else {
        // Smile
        for (i32 i = -6; i <= 6; i++) {
            i32 dy = static_cast<i32>(std::sqrt(36 - i * i) * 0.3f);
            canvas.setPixel(static_cast<u32>(pos.x + i), static_cast<u32>(pos.y + 7 + dy), Color::black());
        }
    }

    // Thinking indicator (animated dots)
    if (thinking) {
        f32 time = SDL_GetTicks() / 200.0f;
        for (int i = 0; i < 3; i++) {
            f32 offset = std::sin(time + i * 1.0f) * 3.0f;
            renderer.fillCircle(Circle(Vec2f(pos.x - 10 + i * 10, pos.y - 30 + offset), 2,
                                      Color(200, 200, 255)));
        }
    }
}

// Create NPC entity with AI
VisualNPC createVisualNPC(
    World& world,
    const String& name,
    const String& personality,
    std::shared_ptr<LLMClient> llmClient,
    GameState* gameState,
    const Vec2f& position,
    const Color& color
) {
    Entity* npc = world.createEntity(name);
    npc->addComponent<Transform>(position);

    auto* ai = npc->addComponent<AIBehavior>(llmClient, personality);
    ai->setThinkInterval(5.0f);  // Think every 5 seconds

    // Set perception callback
    ai->setPerceptionCallback([gameState, name]() -> String {
        std::stringstream ss;
        ss << "You are " << name << " in the " << gameState->playerLocation << ". ";
        ss << "Weather: " << gameState->currentWeather << ", Time: " << gameState->timeOfDay << ". ";
        ss << "You see a player nearby (health: " << gameState->playerHealth;
        ss << ", gold: " << gameState->playerGold << ").";
        return ss.str();
    });

    VisualNPC vnpc;
    vnpc.entity = npc;
    vnpc.ai = ai;
    vnpc.color = color;
    vnpc.targetPos = position;
    vnpc.currentSpeech = "";
    vnpc.speechTimer = 0.0f;
    vnpc.wanderTimer = 0.0f;
    vnpc.isSpeaking = false;
    vnpc.isThinking = false;

    return vnpc;
}

// Wrap text to fit in bubble
String wrapText(const String& text, u32 maxWidth, const BitmapFont& font) {
    String result;
    String currentLine;
    std::istringstream stream(text);
    String word;

    while (stream >> word) {
        String testLine = currentLine.empty() ? word : currentLine + " " + word;
        if (font.measureText(testLine) > maxWidth) {
            if (!currentLine.empty()) {
                result += currentLine + "\n";
                currentLine = word;
            } else {
                result += word + "\n";
            }
        } else {
            currentLine = testLine;
        }
    }

    if (!currentLine.empty()) {
        result += currentLine;
    }

    return result;
}

int main() {
    std::cout << "=== Visual AI NPC Demo ===\n\n";
    std::cout << "Initializing...\n";

    // Create window
    Window window("Oracon - AI NPCs Demo", 1024, 768);
    if (!window.isValid()) {
        std::cerr << "Failed to create window: " << window.getError() << "\n";
        return 1;
    }

    // Create canvas and renderer
    Canvas canvas(1024, 768);
    Renderer renderer(&canvas);
    BitmapFont font;

    // Create LLM client
    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    std::shared_ptr<LLMClient> llmClient;

    if (apiKey) {
        std::cout << "Using Anthropic Claude API\n";
        auto client = LLMClientFactory::create(
            LLMClientFactory::Provider::Anthropic,
            String(apiKey),
            "claude-3-5-sonnet-20241022"
        );
        llmClient = std::shared_ptr<LLMClient>(std::move(client));
    } else {
        std::cout << "Using Mock AI (set ANTHROPIC_API_KEY for real responses)\n";
        auto client = LLMClientFactory::create(
            LLMClientFactory::Provider::Mock,
            "", "mock-npc"
        );
        llmClient = std::shared_ptr<LLMClient>(std::move(client));
    }

    // Create game world and state
    World world;
    GameState gameState;

    // Create NPCs
    std::vector<VisualNPC> npcs;

    npcs.push_back(createVisualNPC(
        world, "Merchant Tom",
        "You are a friendly merchant. Keep responses under 30 words. Be cheerful and talk about your wares.",
        llmClient, &gameState,
        Vec2f(200, 300), Color(255, 200, 100)
    ));

    npcs.push_back(createVisualNPC(
        world, "Guard Sarah",
        "You are a serious town guard. Keep responses under 30 words. Be professional and watchful.",
        llmClient, &gameState,
        Vec2f(500, 400), Color(100, 150, 255)
    ));

    npcs.push_back(createVisualNPC(
        world, "Wizard Aldric",
        "You are an eccentric wizard. Keep responses under 30 words. Be mysterious and speak in riddles.",
        llmClient, &gameState,
        Vec2f(800, 250), Color(180, 100, 255)
    ));

    std::cout << "Created " << npcs.size() << " AI NPCs\n";
    std::cout << "\nControls:\n";
    std::cout << "  - Click on NPC to talk\n";
    std::cout << "  - Press 1, 2, 3 to make NPCs think\n";
    std::cout << "  - Press SPACE to change time of day\n";
    std::cout << "  - Press ESC to quit\n\n";

    // Game loop
    SDL_Event event;
    u32 lastTime = SDL_GetTicks();
    f32 deltaTime = 0.016f;
    VisualNPC* selectedNPC = nullptr;

    while (window.isRunning()) {
        // Calculate delta time
        u32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Handle events
        while (window.pollEvent(event)) {
            if (event.type == SDL_QUIT) {
                window.close();
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    window.close();
                } else if (event.key.keysym.sym == SDLK_SPACE) {
                    // Cycle time of day
                    if (gameState.timeOfDay == "morning") gameState.timeOfDay = "afternoon";
                    else if (gameState.timeOfDay == "afternoon") gameState.timeOfDay = "evening";
                    else if (gameState.timeOfDay == "evening") gameState.timeOfDay = "night";
                    else gameState.timeOfDay = "morning";
                    std::cout << "Time changed to: " << gameState.timeOfDay << "\n";
                } else if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_3) {
                    // Make NPC think
                    int idx = event.key.keysym.sym - SDLK_1;
                    if (idx < static_cast<int>(npcs.size())) {
                        npcs[idx].isThinking = true;
                        std::cout << npcs[idx].entity->getName() << " is thinking...\n";

                        // Trigger AI thinking in background (simplified)
                        npcs[idx].currentSpeech = "...";
                        npcs[idx].isSpeaking = true;
                        npcs[idx].speechTimer = 3.0f;
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                i32 mouseX, mouseY;
                window.getMousePosition(mouseX, mouseY);

                // Check if clicked on an NPC
                for (auto& npc : npcs) {
                    auto* transform = npc.entity->getComponent<Transform>();
                    if (transform) {
                        Vec2f npcPos = transform->position;
                        f32 dx = static_cast<f32>(mouseX) - npcPos.x;
                        f32 dy = static_cast<f32>(mouseY) - npcPos.y;
                        f32 dist = std::sqrt(dx * dx + dy * dy);

                        if (dist < 25.0f) {  // Click radius
                            std::cout << "Clicked on " << npc.entity->getName() << "\n";
                            std::cout << "Player: Hello!\n";

                            // Start conversation
                            npc.isThinking = true;
                            String response = npc.ai->talk("Hello!");
                            npc.currentSpeech = response;
                            npc.isSpeaking = true;
                            npc.speechTimer = 5.0f;  // Display for 5 seconds
                            npc.isThinking = false;

                            std::cout << npc.entity->getName() << ": " << response << "\n\n";
                            break;
                        }
                    }
                }
            }
        }

        // Update NPCs
        for (auto& npc : npcs) {
            auto* transform = npc.entity->getComponent<Transform>();
            if (!transform) continue;

            // Update speech timer
            if (npc.speechTimer > 0.0f) {
                npc.speechTimer -= deltaTime;
                if (npc.speechTimer <= 0.0f) {
                    npc.isSpeaking = false;
                    npc.currentSpeech = "";
                }
            }

            // Simple wandering behavior
            npc.wanderTimer += deltaTime;
            if (npc.wanderTimer > 4.0f) {
                npc.wanderTimer = 0.0f;
                // Set new random target
                npc.targetPos = Vec2f(
                    100.0f + (std::rand() % 824),
                    100.0f + (std::rand() % 568)
                );
            }

            // Move towards target
            Vec2f dir = npc.targetPos - transform->position;
            f32 dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (dist > 2.0f) {
                dir.x /= dist;
                dir.y /= dist;
                transform->position.x += dir.x * 30.0f * deltaTime;
                transform->position.y += dir.y * 30.0f * deltaTime;
            }
        }

        // Render
        renderer.clear(Color(50, 150, 80));  // Grass green background

        // Draw title
        font.drawText(canvas, "AI NPCs in Oracon Village", 10, 10, Color::white());

        // Draw game state
        String statusText = "Time: " + gameState.timeOfDay + " | Weather: " + gameState.currentWeather;
        font.drawText(canvas, statusText, 10, 30, Color::white());

        // Draw instructions
        font.drawText(canvas, "Click NPCs to talk | 1-3: Make think | SPACE: Change time | ESC: Quit",
                     10, 740, Color(200, 200, 200));

        // Draw NPCs
        for (const auto& npc : npcs) {
            auto* transform = npc.entity->getComponent<Transform>();
            if (!transform) continue;

            // Draw NPC
            drawNPC(canvas, transform->position, npc.color, npc.isThinking);

            // Draw name label
            String name = npc.entity->getName();
            u32 nameWidth = font.measureText(name);
            i32 nameX = static_cast<i32>(transform->position.x - nameWidth / 2);
            i32 nameY = static_cast<i32>(transform->position.y + 30);
            font.drawText(canvas, name, nameX, nameY, Color::white());

            // Draw speech bubble if speaking
            if (npc.isSpeaking && !npc.currentSpeech.empty()) {
                String wrappedText = wrapText(npc.currentSpeech, 300, font);

                i32 bubbleX = static_cast<i32>(transform->position.x + 30);
                i32 bubbleY = static_cast<i32>(transform->position.y - 40);

                // Keep bubble on screen
                if (bubbleX + 320 > 1024) bubbleX = 1024 - 320;
                if (bubbleY < 0) bubbleY = 0;

                drawSpeechBubble(canvas, font, wrappedText,
                               bubbleX, bubbleY,
                               Color(255, 255, 220, 240),
                               Color::black(),
                               6);
            }
        }

        // Present to window
        window.present(canvas);

        // Small delay to limit FPS
        SDL_Delay(16);  // ~60 FPS
    }

    std::cout << "\nDemo ended. Thanks for playing!\n";
    return 0;
}
