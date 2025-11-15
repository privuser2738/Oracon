#include "oracon/engine/engine.h"
#include "oracon/auto/auto.h"
#include <iostream>
#include <sstream>

using namespace oracon;
using namespace oracon::engine;
using namespace oracon::auto_ns;

// Helper class to track game state
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

// Create an NPC entity with AI behavior
Entity* createNPC(
    World& world,
    const String& name,
    const String& personality,
    std::shared_ptr<LLMClient> llmClient,
    GameState* gameState,
    const Vec2f& position = Vec2f(0.0f, 0.0f)
) {
    Entity* npc = world.createEntity(name);

    // Add transform
    npc->addComponent<Transform>(position);

    // Add tag
    npc->addComponent<Tag>(name);

    // Add AI behavior
    auto* ai = npc->addComponent<AIBehavior>(llmClient, personality);

    // Set perception callback - let AI see the game state
    ai->setPerceptionCallback([gameState, name]() -> String {
        std::stringstream ss;
        ss << "You are " << name << " in the " << gameState->playerLocation << ".\n";
        ss << "Weather: " << gameState->currentWeather << ", Time: " << gameState->timeOfDay << "\n";
        ss << "You can see a player nearby (health: " << gameState->playerHealth;
        ss << ", gold: " << gameState->playerGold << ").";
        return ss.str();
    });

    // Set action callback - let AI describe what it's doing
    ai->setActionCallback([name](const String& decision) {
        std::cout << "[" << name << " thinks: " << decision << "]\n";
    });

    return npc;
}

int main() {
    std::cout << "=== OraconEngine AI NPC Demo ===\n\n";
    std::cout << "This demo shows AI-powered NPCs using the OraconAuto framework.\n";
    std::cout << "NPCs have personalities, can perceive their environment, and interact!\n\n";

    // Create game world
    World world;
    GameState gameState;

    // Create LLM client (using mock for demo, can use Claude with API key)
    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    std::shared_ptr<LLMClient> llmClient;

    if (apiKey) {
        std::cout << "Using Anthropic Claude API for realistic AI responses!\n\n";
        auto client = LLMClientFactory::create(
            LLMClientFactory::Provider::Anthropic,
            String(apiKey),
            "claude-3-5-sonnet-20241022"
        );
        llmClient = std::shared_ptr<LLMClient>(std::move(client));
    } else {
        std::cout << "Using Mock AI (set ANTHROPIC_API_KEY for real Claude responses)\n\n";
        auto client = LLMClientFactory::create(
            LLMClientFactory::Provider::Mock,
            "",
            "mock-npc"
        );
        llmClient = std::shared_ptr<LLMClient>(std::move(client));
    }

    // Create NPCs with different personalities
    std::cout << "=== Creating NPCs ===\n";

    Entity* merchant = createNPC(
        world,
        "Merchant Tom",
        "You are a friendly merchant who sells potions and equipment. "
        "You're always looking to make a profit, but you're honest and helpful. "
        "You greet customers warmly and love to chat about your wares.",
        llmClient,
        &gameState,
        Vec2f(10.0f, 5.0f)
    );
    std::cout << "Created: Merchant Tom (friendly merchant)\n";

    Entity* guard = createNPC(
        world,
        "Guard Sarah",
        "You are a serious town guard who takes your duty very seriously. "
        "You're protective of citizens, suspicious of strangers, and always alert. "
        "You speak formally and professionally.",
        llmClient,
        &gameState,
        Vec2f(20.0f, 10.0f)
    );
    std::cout << "Created: Guard Sarah (serious town guard)\n";

    Entity* wizard = createNPC(
        world,
        "Wizard Aldric",
        "You are an eccentric old wizard who speaks in riddles and metaphors. "
        "You're wise but cryptic, and you often get lost in your own thoughts. "
        "You're fascinated by magic and ancient mysteries.",
        llmClient,
        &gameState,
        Vec2f(15.0f, 15.0f)
    );
    std::cout << "Created: Wizard Aldric (eccentric wizard)\n\n";

    // Demo 1: NPCs perceive their environment
    std::cout << "=== Demo 1: Autonomous NPC Thinking ===\n";
    std::cout << "Game state:\n" << gameState.describe() << "\n\n";

    auto* merchantAI = merchant->getComponent<AIBehavior>();
    auto* guardAI = guard->getComponent<AIBehavior>();
    auto* wizardAI = wizard->getComponent<AIBehavior>();

    std::cout << "Each NPC autonomously thinks about their situation...\n\n";

    std::cout << "[Merchant Tom]: ";
    String merchantThought = merchantAI->think();
    std::cout << merchantThought << "\n\n";

    std::cout << "[Guard Sarah]: ";
    String guardThought = guardAI->think();
    std::cout << guardThought << "\n\n";

    std::cout << "[Wizard Aldric]: ";
    String wizardThought = wizardAI->think();
    std::cout << wizardThought << "\n\n";

    // Demo 2: Player talks to NPCs
    std::cout << "=== Demo 2: Player Interaction ===\n";
    std::cout << "Player approaches Merchant Tom...\n\n";

    std::cout << "Player: \"Hello! What are you selling?\"\n";
    String merchantReply = merchantAI->talk("Hello! What are you selling?");
    std::cout << "[Merchant Tom]: " << merchantReply << "\n\n";

    std::cout << "Player approaches Guard Sarah...\n\n";

    std::cout << "Player: \"Good day, officer. Is the town safe?\"\n";
    String guardReply = guardAI->talk("Good day, officer. Is the town safe?");
    std::cout << "[Guard Sarah]: " << guardReply << "\n\n";

    std::cout << "Player approaches Wizard Aldric...\n\n";

    std::cout << "Player: \"Wise wizard, can you teach me magic?\"\n";
    String wizardReply = wizardAI->talk("Wise wizard, can you teach me magic?");
    std::cout << "[Wizard Aldric]: " << wizardReply << "\n\n";

    // Demo 3: Game state changes affect NPC behavior
    std::cout << "=== Demo 3: NPCs React to Changing Conditions ===\n";
    std::cout << "Night falls and storm clouds gather...\n\n";

    gameState.timeOfDay = "night";
    gameState.currentWeather = "stormy";

    std::cout << "New game state:\n" << gameState.describe() << "\n\n";

    std::cout << "[Guard Sarah]: ";
    String guardNightThought = guardAI->think();
    std::cout << guardNightThought << "\n\n";

    std::cout << "[Wizard Aldric]: ";
    String wizardStormThought = wizardAI->think();
    std::cout << wizardStormThought << "\n\n";

    // Demo 4: Continuous conversation
    std::cout << "=== Demo 4: Multi-turn Conversation ===\n";
    std::cout << "Having a deeper conversation with Merchant Tom...\n\n";

    std::cout << "Player: \"Do you have any health potions?\"\n";
    String reply1 = merchantAI->talk("Do you have any health potions?");
    std::cout << "[Merchant Tom]: " << reply1 << "\n\n";

    std::cout << "Player: \"How much for one potion?\"\n";
    String reply2 = merchantAI->talk("How much for one potion?");
    std::cout << "[Merchant Tom]: " << reply2 << "\n\n";

    std::cout << "Player: \"I'll take two potions, please!\"\n";
    String reply3 = merchantAI->talk("I'll take two potions, please!");
    std::cout << "[Merchant Tom]: " << reply3 << "\n\n";

    // Demo 5: NPC collision/interaction
    std::cout << "=== Demo 5: NPC Collision Events ===\n";
    std::cout << "Player accidentally bumps into Guard Sarah...\n\n";

    // Simulate collision event
    Entity* player = world.createEntity("Player");
    guardAI->onCollision(player);
    std::cout << "(Guard Sarah reacts to collision: " << guardAI->getLastResponse() << ")\n\n";

    // Demo 6: NPC statistics
    std::cout << "=== Demo 6: NPC Memory & Statistics ===\n";

    std::cout << "\nMerchant Tom's conversation history:\n";
    auto merchantHistory = merchantAI->getAgent()->getHistory();
    std::cout << "  Total messages in memory: " << merchantHistory.size() << "\n";

    std::cout << "\nGuard Sarah's conversation history:\n";
    auto guardHistory = guardAI->getAgent()->getHistory();
    std::cout << "  Total messages in memory: " << guardHistory.size() << "\n";

    std::cout << "\nWizard Aldric's conversation history:\n";
    auto wizardHistory = wizardAI->getAgent()->getHistory();
    std::cout << "  Total messages in memory: " << wizardHistory.size() << "\n\n";

    // Demo 7: NPC behavior configuration
    std::cout << "=== Demo 7: Runtime Behavior Modification ===\n";
    std::cout << "Changing Merchant Tom's personality...\n\n";

    merchantAI->setPersonality(
        "You are now a greedy, suspicious merchant who tries to overcharge customers. "
        "You're constantly worried about thieves and speak in a paranoid manner."
    );
    merchantAI->clearMemory();  // Reset conversation

    std::cout << "Player: \"Hello again, how much for a potion now?\"\n";
    String greedyReply = merchantAI->talk("Hello again, how much for a potion now?");
    std::cout << "[Merchant Tom (now greedy)]: " << greedyReply << "\n\n";

    // Summary
    std::cout << "=== Demo Complete! ===\n\n";
    std::cout << "Summary of AI NPC capabilities:\n";
    std::cout << "  ✓ NPCs have unique personalities\n";
    std::cout << "  ✓ NPCs perceive and react to game state\n";
    std::cout << "  ✓ NPCs engage in natural conversations\n";
    std::cout << "  ✓ NPCs maintain conversation memory\n";
    std::cout << "  ✓ NPCs react to events (collisions)\n";
    std::cout << "  ✓ NPCs can think autonomously\n";
    std::cout << "  ✓ NPC behavior can be modified at runtime\n\n";

    std::cout << "World statistics:\n";
    std::cout << "  Total entities: " << world.getEntities().size() << "\n";
    std::cout << "  Total AI NPCs: 3\n\n";

    std::cout << "You can now create immersive games with:\n";
    std::cout << "  - Realistic NPC conversations\n";
    std::cout << "  - Dynamic, context-aware behavior\n";
    std::cout << "  - Emergent storytelling\n";
    std::cout << "  - Procedural dialogue\n\n";

    return 0;
}
