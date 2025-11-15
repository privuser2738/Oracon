# AI-Powered NPCs: OraconAuto + OraconEngine Integration

**Date:** 2025-11-14
**Status:** ✅ Complete and Working

## Overview

This document describes the successful integration of **OraconAuto** (AI agent framework) with **OraconEngine** (2D game engine) to create AI-powered non-player characters (NPCs) with realistic, context-aware behavior.

## Architecture

### AIBehavior Component

The `AIBehavior` class is a Script component that wraps an `Agent` from OraconAuto:

```cpp
// OraconEngine/include/oracon/engine/ai_behavior.h
class AIBehavior : public Script {
    // Core AI agent
    std::unique_ptr<Agent> m_agent;

    // Callbacks for game integration
    PerceptionCallback m_perceptionCallback;
    ActionCallback m_actionCallback;

    // Configuration
    String m_personality;
    f32 m_thinkInterval;
    bool m_enabled;
};
```

### Key Features

1. **Personality System**
   - Each NPC has a customizable system prompt defining their personality
   - Personalities can be changed at runtime
   - Examples: friendly merchant, serious guard, eccentric wizard

2. **Environmental Perception**
   - Perception callback allows NPCs to "see" game state
   - NPCs receive context about:
     - Current location
     - Weather and time of day
     - Nearby entities (player health, gold, etc.)
     - World events

3. **Conversational Memory**
   - NPCs remember previous conversations
   - Multi-turn dialogue with context retention
   - Memory can be cleared or managed

4. **Autonomous Thinking**
   - NPCs think periodically based on `thinkInterval`
   - Autonomous reactions to their environment
   - Event-driven responses (collisions, triggers)

5. **Action System**
   - Action callback translates AI decisions into game actions
   - NPCs can describe what they're doing
   - Integration with game mechanics

## Usage Example

### Creating an AI NPC

```cpp
// Create a world and game state
World world;
GameState gameState;

// Create LLM client (mock or real)
auto llmClient = LLMClientFactory::create(
    LLMClientFactory::Provider::Mock,
    "", "mock-npc"
);

// Create NPC entity
Entity* merchant = world.createEntity("Merchant Tom");
merchant->addComponent<Transform>(Vec2f(10.0f, 5.0f));

// Add AI behavior with personality
auto* ai = merchant->addComponent<AIBehavior>(
    llmClient,
    "You are a friendly merchant who sells potions. "
    "You're always looking to make a profit, but you're honest."
);

// Set perception callback
ai->setPerceptionCallback([&gameState]() -> String {
    return "Location: " + gameState.playerLocation +
           ", Weather: " + gameState.currentWeather;
});

// Set action callback
ai->setActionCallback([](const String& decision) {
    std::cout << "[Merchant thinks: " << decision << "]\n";
});
```

### Interacting with NPCs

```cpp
// Player talks to NPC
String response = ai->talk("Hello! What are you selling?");
std::cout << "Merchant: " << response << "\n";

// NPC autonomous thinking
String thought = ai->think();

// Change personality at runtime
ai->setPersonality("You are now a greedy merchant...");
ai->clearMemory();  // Reset conversation
```

### Event Handling

```cpp
// NPC reacts to collision
void onCollision(Entity* other) override {
    String stimulus = "You collided with: " + other->getName();
    think(stimulus);
}

// Periodic autonomous thinking
void onUpdate(f32 deltaTime) override {
    m_lastThinkTime += deltaTime;

    if (m_lastThinkTime >= m_thinkInterval) {
        m_lastThinkTime = 0.0f;
        think();  // NPC thinks about current situation
    }
}
```

## Demo Application

The **ai_npc_demo** showcases all integration features:

### Demo Scenarios

1. **Autonomous NPC Thinking**
   - Three NPCs with different personalities
   - Each perceives the environment and reacts differently
   - Merchant focuses on business, Guard on security, Wizard on mysteries

2. **Player Interaction**
   - Natural conversations with NPCs
   - Context-aware responses based on personality
   - Dynamic dialogue (not scripted!)

3. **Environmental Reactivity**
   - NPCs react to changing conditions (day/night, weather)
   - Guard becomes more alert at night
   - Wizard comments on storms

4. **Multi-turn Conversations**
   - NPCs remember previous messages
   - Coherent dialogue across multiple exchanges
   - Example: negotiating potion prices with merchant

5. **Collision Events**
   - NPCs react to physical interactions
   - Guard responds when bumped
   - Context-aware reactions

6. **Memory Statistics**
   - Track conversation history
   - Monitor message counts
   - Understand AI state

7. **Runtime Modification**
   - Change NPC personality on the fly
   - Transform friendly merchant into greedy one
   - Clear memory and start fresh

### Running the Demo

```bash
cd /home/aseio/source/Oracon/build
./bin/ai_npc_demo

# For real AI responses with Claude:
ANTHROPIC_API_KEY=your_key ./bin/ai_npc_demo
```

## Technical Implementation

### Files Created/Modified

**New Files:**
- `OraconEngine/include/oracon/engine/ai_behavior.h` - AIBehavior component
- `OraconEngine/examples/ai_npc_demo.cpp` - Comprehensive demo

**Modified Files:**
- `OraconEngine/include/oracon/engine/engine.h` - Added ai_behavior.h include
- `OraconEngine/CMakeLists.txt` - Linked OraconAuto library
- `OraconEngine/examples/CMakeLists.txt` - Added ai_npc_demo target
- `STATUS.md` - Updated OraconAuto status to "CORE COMPLETE"

### Dependencies

```cmake
target_link_libraries(OraconEngine PUBLIC
    OraconCore      # Core types and utilities
    OraconMath      # Vector math
    OraconGfx       # Graphics (for future rendering)
    OraconLang      # Scripting (for future integration)
    OraconAuto      # AI agent framework (NEW!)
)
```

### Build Requirements

- CMake 3.15+
- C++17 compiler
- libcurl (for OraconAuto HTTP client)
- Optional: ANTHROPIC_API_KEY environment variable for real Claude responses

## Capabilities Demonstrated

✅ **NPCs with unique personalities**
✅ **Environmental perception and reaction**
✅ **Natural language conversations**
✅ **Conversational memory**
✅ **Event-driven behavior (collisions)**
✅ **Autonomous thinking**
✅ **Runtime behavior modification**
✅ **Multi-turn dialogue**
✅ **Context-aware responses**

## Use Cases

This integration enables:

1. **Immersive RPGs**
   - NPCs that remember player interactions
   - Dynamic quest givers
   - Realistic town inhabitants

2. **Emergent Storytelling**
   - NPCs that react to player choices
   - Unpredictable but coherent behavior
   - Procedural dialogue generation

3. **Educational Games**
   - NPCs that teach and adapt
   - Tutors that respond to questions
   - Interactive learning experiences

4. **Simulation Games**
   - Autonomous agents with goals
   - Social interactions between NPCs
   - Complex emergent behavior

5. **Adventure Games**
   - Puzzle-solving companions
   - Guide characters
   - Interactive story elements

## Performance Considerations

- **Think Interval**: Control AI computation frequency (default 2 seconds)
- **Memory Management**: Conversation history grows over time
  - Use sliding window memory for long sessions
  - Periodically clear memory for stateless NPCs
- **LLM Costs**: Real API calls have costs
  - Use mock client for testing
  - Cache common responses
  - Rate limit autonomous thinking

## Future Enhancements

Potential improvements:

1. **Multi-NPC Interactions**
   - NPCs talking to each other
   - Group dynamics
   - Social networks

2. **Goal-Oriented Behavior**
   - NPCs with objectives
   - Planning and execution
   - Task prioritization

3. **Emotional States**
   - Mood tracking
   - Emotional reactions
   - Relationship systems

4. **Tool Integration**
   - NPCs can use game tools
   - File reading for lore
   - Code execution for puzzles

5. **Visual Indicators**
   - Speech bubbles
   - Thought indicators
   - Emotional expressions

6. **Voice Synthesis**
   - Text-to-speech integration
   - Voiced dialogue
   - Lip sync

## Conclusion

The integration of OraconAuto with OraconEngine successfully demonstrates AI-powered NPCs with realistic, context-aware behavior. This opens up new possibilities for game development, enabling:

- **Dynamic, unpredictable NPC interactions**
- **Emergent storytelling and gameplay**
- **Reduced need for scripted dialogue trees**
- **More immersive player experiences**

The system is production-ready for:
- ✅ Testing with mock AI
- ✅ Development with Claude API
- ✅ Extension with custom tools and behaviors
- ✅ Integration with full game engines

**Next Steps:**
- Add OpenAI GPT-4 support for alternative LLM provider
- Implement local LLM support (llama.cpp) for offline games
- Create visual demo with OraconGfx rendering
- Build full game prototype using AI NPCs
