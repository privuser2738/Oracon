#include "oracon/engine/engine.h"
#include "oracon/auto/auto.h"
#include "oracon/gfx/gfx.h"
#include "oracon/gfx/window.h"
#include "oracon/gfx/text.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <fstream>

using namespace oracon;
using namespace oracon::engine;
using namespace oracon::gfx;
using namespace oracon::auto_ns;
using namespace oracon::math;

// Forward declarations
struct Quest;
struct Item;
struct PlayerData;

// Item structure
struct Item {
    String name;
    String description;
    int value;
    String type;  // "potion", "weapon", "misc"

    Item() : value(0) {}
    Item(const String& n, const String& desc, int v, const String& t)
        : name(n), description(desc), value(v), type(t) {}
};

// Quest structure
struct Quest {
    String id;
    String title;
    String description;
    String giver;  // NPC who gave the quest
    bool active;
    bool completed;
    String completionHint;

    Quest() : active(false), completed(false) {}
    Quest(const String& qid, const String& t, const String& desc, const String& g, const String& hint)
        : id(qid), title(t), description(desc), giver(g), active(true), completed(false), completionHint(hint) {}
};

// Player data
struct PlayerData {
    Vec2f position;
    int health;
    int maxHealth;
    int gold;
    int experience;
    std::vector<Item> inventory;
    std::vector<Quest> quests;

    PlayerData()
        : position(512, 384), health(100), maxHealth(100), gold(50), experience(0) {}

    void addItem(const Item& item) {
        inventory.push_back(item);
    }

    bool hasItem(const String& itemName) const {
        for (const auto& item : inventory) {
            if (item.name == itemName) return true;
        }
        return false;
    }

    void removeItem(const String& itemName) {
        for (auto it = inventory.begin(); it != inventory.end(); ++it) {
            if (it->name == itemName) {
                inventory.erase(it);
                return;
            }
        }
    }

    void addQuest(const Quest& quest) {
        quests.push_back(quest);
    }

    Quest* getQuest(const String& questId) {
        for (auto& quest : quests) {
            if (quest.id == questId) return &quest;
        }
        return nullptr;
    }

    int getActiveQuestCount() const {
        int count = 0;
        for (const auto& quest : quests) {
            if (quest.active && !quest.completed) count++;
        }
        return count;
    }
};

// Game state
struct GameState {
    String playerLocation;
    String currentWeather;
    String timeOfDay;
    int dayNumber;
    PlayerData player;

    GameState()
        : playerLocation("village square")
        , currentWeather("sunny")
        , timeOfDay("morning")
        , dayNumber(1) {}

    String describe() const {
        std::stringstream ss;
        ss << "Day " << dayNumber << ", " << timeOfDay << ", " << currentWeather;
        return ss.str();
    }

    String getPlayerContext() const {
        std::stringstream ss;
        ss << "Player stats: Health " << player.health << "/" << player.maxHealth;
        ss << ", Gold " << player.gold;
        ss << ", Active quests: " << player.getActiveQuestCount();
        return ss.str();
    }
};

// UI Panels
enum class UIPanel {
    None,
    Inventory,
    QuestLog,
    Help
};

// Draw player character
void drawPlayer(Canvas& canvas, const Vec2f& pos) {
    Renderer renderer(&canvas);

    // Shadow
    renderer.fillCircle(Circle(Vec2f(pos.x + 2, pos.y + 2), 23, Color(0, 0, 0, 64)));

    // Body (green for player)
    renderer.fillCircle(Circle(pos, 25, Color(100, 255, 100)));

    // Outline
    renderer.drawCircle(pos, 25, Color::black(), false);

    // Eyes
    renderer.fillCircle(Circle(Vec2f(pos.x - 8, pos.y - 5), 4, Color::white()));
    renderer.fillCircle(Circle(Vec2f(pos.x + 8, pos.y - 5), 4, Color::white()));
    renderer.fillCircle(Circle(Vec2f(pos.x - 8, pos.y - 5), 2, Color::black()));
    renderer.fillCircle(Circle(Vec2f(pos.x + 8, pos.y - 5), 2, Color::black()));

    // Smile
    for (i32 i = -8; i <= 8; i++) {
        i32 dy = static_cast<i32>(std::sqrt(64 - i * i) * 0.3f);
        canvas.setPixel(static_cast<u32>(pos.x + i), static_cast<u32>(pos.y + 10 + dy), Color::black());
    }
}

// Draw NPC (simpler version from visual demo)
void drawNPC(Canvas& canvas, const Vec2f& pos, const Color& color) {
    Renderer renderer(&canvas);
    renderer.fillCircle(Circle(Vec2f(pos.x + 2, pos.y + 2), 18, Color(0, 0, 0, 64)));
    renderer.fillCircle(Circle(pos, 20, color));
    renderer.drawCircle(pos, 20, Color::black(), false);
    renderer.fillCircle(Circle(Vec2f(pos.x - 7, pos.y - 5), 4, Color::white()));
    renderer.fillCircle(Circle(Vec2f(pos.x + 7, pos.y - 5), 4, Color::white()));
    renderer.fillCircle(Circle(Vec2f(pos.x - 7, pos.y - 5), 2, Color::black()));
    renderer.fillCircle(Circle(Vec2f(pos.x + 7, pos.y - 5), 2, Color::black()));
    for (i32 i = -6; i <= 6; i++) {
        i32 dy = static_cast<i32>(std::sqrt(36 - i * i) * 0.3f);
        canvas.setPixel(static_cast<u32>(pos.x + i), static_cast<u32>(pos.y + 7 + dy), Color::black());
    }
}

// Draw UI panel
void drawPanel(Canvas& canvas, const BitmapFont& font, i32 x, i32 y, i32 width, i32 height,
               const String& title, const std::vector<String>& lines) {
    Renderer renderer(&canvas);

    // Background
    for (i32 py = 0; py < height; py++) {
        for (i32 px = 0; px < width; px++) {
            canvas.setPixel(x + px, y + py, Color(40, 40, 60, 230));
        }
    }

    // Border
    for (i32 px = 0; px < width; px++) {
        canvas.setPixel(x + px, y, Color::white());
        canvas.setPixel(x + px, y + height - 1, Color::white());
    }
    for (i32 py = 0; py < height; py++) {
        canvas.setPixel(x, y + py, Color::white());
        canvas.setPixel(x + width - 1, y + py, Color::white());
    }

    // Title
    font.drawText(canvas, title, x + 10, y + 5, Color(255, 255, 100));

    // Lines
    i32 lineY = y + 20;
    for (const auto& line : lines) {
        if (lineY + 10 < y + height) {
            font.drawText(canvas, line, x + 10, lineY, Color::white());
            lineY += 10;
        }
    }
}

// Wrap text helper
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
    if (!currentLine.empty()) result += currentLine;
    return result;
}

// NPC with enhanced AI context
struct GameNPC {
    Entity* entity;
    AIBehavior* ai;
    Color color;
    String currentSpeech;
    f32 speechTimer;
    String role;  // "merchant", "guard", "wizard"

    GameNPC() : entity(nullptr), ai(nullptr), speechTimer(0.0f) {}
};

int main() {
    std::cout << "=== Village of Oracon ===\n\n";
    std::cout << "A playable AI-powered RPG!\n\n";

    // Create window
    Window window("Village of Oracon - AI RPG", 1024, 768);
    if (!window.isValid()) {
        std::cerr << "Failed to create window: " << window.getError() << "\n";
        return 1;
    }

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
        std::cout << "Using Mock AI\n";
        auto client = LLMClientFactory::create(
            LLMClientFactory::Provider::Mock, "", "mock-npc"
        );
        llmClient = std::shared_ptr<LLMClient>(std::move(client));
    }

    // Create game world and state
    World world;
    GameState gameState;

    // Initialize starting items
    gameState.player.addItem(Item("Rusty Sword", "An old but serviceable sword", 10, "weapon"));
    gameState.player.addItem(Item("Bread", "A loaf of fresh bread", 2, "food"));

    // Create NPCs with enhanced AI
    std::vector<GameNPC> npcs;

    // Merchant Tom
    {
        GameNPC npc;
        npc.entity = world.createEntity("Merchant Tom");
        npc.entity->addComponent<Transform>(Vec2f(200, 300));
        npc.color = Color(255, 200, 100);
        npc.role = "merchant";

        npc.ai = npc.entity->addComponent<AIBehavior>(llmClient,
            "You are Merchant Tom, a friendly merchant in Oracon Village. "
            "You sell potions and items. You can give quests to find rare items. "
            "Keep responses under 30 words. Be cheerful about trading."
        );

        npc.ai->setPerceptionCallback([&gameState, &npc]() -> String {
            std::stringstream ss;
            ss << "You see the player. " << gameState.getPlayerContext() << ". ";
            ss << "Time: " << gameState.timeOfDay << ". ";
            ss << "You sell: Health Potion (20g), Stamina Potion (15g).";
            return ss.str();
        });

        npcs.push_back(npc);
    }

    // Guard Sarah
    {
        GameNPC npc;
        npc.entity = world.createEntity("Guard Sarah");
        npc.entity->addComponent<Transform>(Vec2f(500, 400));
        npc.color = Color(100, 150, 255);
        npc.role = "guard";

        npc.ai = npc.entity->addComponent<AIBehavior>(llmClient,
            "You are Guard Sarah, the town guard. You're professional and protect the village. "
            "You can give quests to deal with threats. "
            "Keep responses under 30 words. Be dutiful and serious."
        );

        npc.ai->setPerceptionCallback([&gameState]() -> String {
            std::stringstream ss;
            ss << "You're on patrol. " << gameState.getPlayerContext() << ". ";
            ss << "Time: " << gameState.timeOfDay << ". All seems quiet in the village.";
            return ss.str();
        });

        npcs.push_back(npc);
    }

    // Wizard Aldric
    {
        GameNPC npc;
        npc.entity = world.createEntity("Wizard Aldric");
        npc.entity->addComponent<Transform>(Vec2f(800, 250));
        npc.color = Color(180, 100, 255);
        npc.role = "wizard";

        npc.ai = npc.entity->addComponent<AIBehavior>(llmClient,
            "You are Wizard Aldric, an eccentric wizard. You speak mysteriously and can teach magic. "
            "You can give quests to find magical items or solve riddles. "
            "Keep responses under 30 words. Be cryptic and wise."
        );

        npc.ai->setPerceptionCallback([&gameState]() -> String {
            std::stringstream ss;
            ss << "You sense the player's presence. " << gameState.getPlayerContext() << ". ";
            ss << "The magical energies flow... Time: " << gameState.timeOfDay << ".";
            return ss.str();
        });

        npcs.push_back(npc);
    }

    std::cout << "\nGame started! Controls:\n";
    std::cout << "  WASD/Arrows - Move\n";
    std::cout << "  E - Talk to nearby NPC\n";
    std::cout << "  I - Inventory\n";
    std::cout << "  Q - Quest Log\n";
    std::cout << "  H - Help\n";
    std::cout << "  ESC - Quit\n\n";

    // Game loop
    SDL_Event event;
    u32 lastTime = SDL_GetTicks();
    f32 deltaTime = 0.016f;
    UIPanel currentPanel = UIPanel::None;
    GameNPC* nearbyNPC = nullptr;

    while (window.isRunning()) {
        u32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Event handling
        while (window.pollEvent(event)) {
            if (event.type == SDL_QUIT) {
                window.close();
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (currentPanel != UIPanel::None) {
                        currentPanel = UIPanel::None;
                    } else {
                        window.close();
                    }
                } else if (event.key.keysym.sym == SDLK_i) {
                    currentPanel = (currentPanel == UIPanel::Inventory) ? UIPanel::None : UIPanel::Inventory;
                } else if (event.key.keysym.sym == SDLK_q) {
                    currentPanel = (currentPanel == UIPanel::QuestLog) ? UIPanel::None : UIPanel::QuestLog;
                } else if (event.key.keysym.sym == SDLK_h) {
                    currentPanel = (currentPanel == UIPanel::Help) ? UIPanel::None : UIPanel::Help;
                } else if (event.key.keysym.sym == SDLK_e && nearbyNPC) {
                    // Talk to nearby NPC
                    std::cout << "Talking to " << nearbyNPC->entity->getName() << "...\n";
                    String response = nearbyNPC->ai->talk("Hello! I'm an adventurer.");
                    nearbyNPC->currentSpeech = response;
                    nearbyNPC->speechTimer = 6.0f;
                    std::cout << nearbyNPC->entity->getName() << ": " << response << "\n";
                }
            }
        }

        // Player movement
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        f32 moveSpeed = 150.0f * deltaTime;

        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) gameState.player.position.y -= moveSpeed;
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) gameState.player.position.y += moveSpeed;
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) gameState.player.position.x -= moveSpeed;
        if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) gameState.player.position.x += moveSpeed;

        // Keep player in bounds
        gameState.player.position.x = std::max(30.0f, std::min(994.0f, gameState.player.position.x));
        gameState.player.position.y = std::max(30.0f, std::min(738.0f, gameState.player.position.y));

        // Check for nearby NPCs
        nearbyNPC = nullptr;
        for (auto& npc : npcs) {
            auto* transform = npc.entity->getComponent<Transform>();
            if (transform) {
                f32 dx = gameState.player.position.x - transform->position.x;
                f32 dy = gameState.player.position.y - transform->position.y;
                f32 dist = std::sqrt(dx * dx + dy * dy);
                if (dist < 60.0f) {
                    nearbyNPC = &npc;
                    break;
                }
            }
        }

        // Update NPC speech timers
        for (auto& npc : npcs) {
            if (npc.speechTimer > 0.0f) {
                npc.speechTimer -= deltaTime;
                if (npc.speechTimer <= 0.0f) {
                    npc.currentSpeech = "";
                }
            }
        }

        // Render
        renderer.clear(Color(50, 150, 80));  // Grass

        // Draw title
        font.drawText(canvas, "Village of Oracon", 10, 10, Color::white());

        // Draw stats
        std::stringstream stats;
        stats << "Health: " << gameState.player.health << "/" << gameState.player.maxHealth
              << " | Gold: " << gameState.player.gold << " | " << gameState.describe();
        font.drawText(canvas, stats.str(), 10, 25, Color(255, 255, 200));

        // Draw NPCs
        for (const auto& npc : npcs) {
            auto* transform = npc.entity->getComponent<Transform>();
            if (!transform) continue;

            drawNPC(canvas, transform->position, npc.color);

            // Name
            String name = npc.entity->getName();
            u32 nameWidth = font.measureText(name);
            font.drawText(canvas, name,
                         static_cast<i32>(transform->position.x - nameWidth / 2),
                         static_cast<i32>(transform->position.y + 30),
                         Color::white());

            // Speech bubble
            if (!npc.currentSpeech.empty()) {
                String wrapped = wrapText(npc.currentSpeech, 300, font);
                i32 bubbleX = static_cast<i32>(transform->position.x + 30);
                i32 bubbleY = static_cast<i32>(transform->position.y - 40);
                if (bubbleX + 320 > 1024) bubbleX = 1024 - 320;
                if (bubbleY < 0) bubbleY = 0;
                drawSpeechBubble(canvas, font, wrapped, bubbleX, bubbleY,
                               Color(255, 255, 220, 240), Color::black(), 6);
            }
        }

        // Draw player
        drawPlayer(canvas, gameState.player.position);
        font.drawText(canvas, "You",
                     static_cast<i32>(gameState.player.position.x - 10),
                     static_cast<i32>(gameState.player.position.y + 35),
                     Color(200, 255, 200));

        // Draw E prompt if near NPC
        if (nearbyNPC) {
            String prompt = "Press E to talk";
            font.drawText(canvas, prompt,
                         static_cast<i32>(gameState.player.position.x - 50),
                         static_cast<i32>(gameState.player.position.y - 40),
                         Color(255, 255, 100));
        }

        // Draw UI panels
        if (currentPanel == UIPanel::Inventory) {
            std::vector<String> lines;
            lines.push_back("INVENTORY:");
            if (gameState.player.inventory.empty()) {
                lines.push_back("  (empty)");
            } else {
                for (const auto& item : gameState.player.inventory) {
                    lines.push_back("  " + item.name + " (" + std::to_string(item.value) + "g)");
                }
            }
            lines.push_back("");
            lines.push_back("Press I to close");
            drawPanel(canvas, font, 300, 200, 400, 300, "Inventory", lines);
        } else if (currentPanel == UIPanel::QuestLog) {
            std::vector<String> lines;
            lines.push_back("ACTIVE QUESTS:");
            bool hasQuests = false;
            for (const auto& quest : gameState.player.quests) {
                if (quest.active && !quest.completed) {
                    lines.push_back("  " + quest.title);
                    hasQuests = true;
                }
            }
            if (!hasQuests) lines.push_back("  (no active quests)");
            lines.push_back("");
            lines.push_back("Talk to NPCs to get quests!");
            lines.push_back("Press Q to close");
            drawPanel(canvas, font, 300, 200, 400, 250, "Quest Log", lines);
        } else if (currentPanel == UIPanel::Help) {
            std::vector<String> lines;
            lines.push_back("CONTROLS:");
            lines.push_back("  WASD/Arrows - Move");
            lines.push_back("  E - Talk to nearby NPC");
            lines.push_back("  I - Inventory");
            lines.push_back("  Q - Quest Log");
            lines.push_back("  H - Help");
            lines.push_back("  ESC - Close menu/Quit");
            lines.push_back("");
            lines.push_back("GAMEPLAY:");
            lines.push_back("  Talk to NPCs to get quests");
            lines.push_back("  Trade with Merchant Tom");
            lines.push_back("  Explore the village!");
            lines.push_back("");
            lines.push_back("Press H to close");
            drawPanel(canvas, font, 250, 150, 500, 400, "Help", lines);
        }

        // Controls hint
        font.drawText(canvas, "I: Inventory | Q: Quests | H: Help | ESC: Quit",
                     10, 750, Color(200, 200, 200));

        window.present(canvas);
        SDL_Delay(16);  // 60 FPS
    }

    std::cout << "\nThanks for playing Village of Oracon!\n";
    return 0;
}
