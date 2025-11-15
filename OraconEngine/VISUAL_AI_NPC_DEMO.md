# Visual AI NPC Demo

**Date:** 2025-11-14
**Status:** ✅ Complete and Working

## Overview

The Visual AI NPC Demo is a stunning showcase of AI-powered non-player characters in a graphical game environment. It brings together **OraconEngine** (game engine), **OraconGfx** (graphics), and **OraconAuto** (AI framework) into a cohesive, interactive experience.

## Features

### Visual Rendering
- **SDL2 Window**: 1024x768 resolution window
- **Custom Bitmap Font**: 8x8 pixel font for text rendering
- **Speech Bubbles**: Dynamic text backgrounds with borders
- **Sprite Rendering**: Colorful NPC characters with animated features
- **Smooth Animation**: 60 FPS game loop

### AI NPCs
Three distinct AI-powered characters:

1. **Merchant Tom** (Orange)
   - Personality: Friendly merchant
   - Behavior: Talks about wares, cheerful
   - Max response: 30 words

2. **Guard Sarah** (Blue)
   - Personality: Serious town guard
   - Behavior: Professional, watchful
   - Max response: 30 words

3. **Wizard Aldric** (Purple)
   - Personality: Eccentric wizard
   - Behavior: Mysterious, speaks in riddles
   - Max response: 30 words

### Interactions

#### Click-to-Talk
- Click on any NPC to start a conversation
- NPCs respond with AI-generated dialogue
- Speech bubbles display above NPCs
- Responses automatically disappear after 5 seconds

#### Keyboard Controls
- **1, 2, 3**: Trigger autonomous thinking for each NPC
- **SPACE**: Cycle time of day (morning → afternoon → evening → night)
- **ESC**: Exit the demo

### AI Features
- **Environmental Perception**: NPCs see game state (time, weather, player stats)
- **Conversational Memory**: NPCs remember previous interactions
- **Autonomous Wandering**: NPCs move around the village
- **Thinking Animation**: Visual indicator when AI is processing
- **Context-Aware Responses**: NPCs react to time of day and conditions

## Technical Implementation

### Architecture

```
visual_ai_npcs.cpp
├── Window (SDL2)
│   └── Canvas (Pixel buffer)
│       ├── Renderer (Drawing primitives)
│       └── BitmapFont (Text rendering)
├── World (ECS)
│   └── Entities
│       ├── Transform (Position)
│       ├── Tag (Name)
│       └── AIBehavior (Agent)
│           ├── LLMClient (Claude/Mock)
│           ├── Perception Callback
│           └── Action Callback
└── Game Loop (60 FPS)
    ├── Event Handling
    ├── NPC Updates
    └── Rendering
```

### Key Components

**VisualNPC Structure**:
```cpp
struct VisualNPC {
    Entity* entity;          // Game entity
    AIBehavior* ai;         // AI component
    Color color;            // NPC color
    String currentSpeech;   // Active dialogue
    f32 speechTimer;        // Display duration
    f32 wanderTimer;        // Movement timer
    Vec2f targetPos;        // Destination
    bool isSpeaking;        // Speech state
    bool isThinking;        // Thinking state
};
```

**NPC Rendering**:
```cpp
void drawNPC(Canvas& canvas, const Vec2f& pos,
             const Color& color, bool thinking) {
    // Draw shadow
    renderer.fillCircle(...);

    // Draw body
    renderer.fillCircle(Circle(pos, 20, color));

    // Draw eyes (change color when thinking)
    Color eyeColor = thinking ? Color(100, 100, 255) : Color::white();

    // Draw mouth (smile or wavy when thinking)
    if (thinking) {
        // Animated wavy mouth
    } else {
        // Normal smile
    }

    // Thinking indicator (animated dots)
    if (thinking) {
        f32 time = SDL_GetTicks() / 200.0f;
        for (int i = 0; i < 3; i++) {
            f32 offset = std::sin(time + i * 1.0f) * 3.0f;
            renderer.fillCircle(...);
        }
    }
}
```

### Game State
```cpp
struct GameState {
    int playerHealth = 100;
    int playerGold = 50;
    String playerLocation = "village square";
    String currentWeather = "sunny";
    String timeOfDay = "afternoon";
};
```

NPCs perceive this state through callbacks:
```cpp
ai->setPerceptionCallback([gameState, name]() -> String {
    return "You are " + name + " in the " + gameState->playerLocation + ". " +
           "Weather: " + gameState->currentWeather + ", Time: " + gameState->timeOfDay;
});
```

## Build Instructions

### Prerequisites
- CMake 3.15+
- C++17 compiler
- SDL2 development libraries
- libcurl (for OraconAuto)

### Building
```bash
cd /home/aseio/source/Oracon/build
cmake ..
make visual_ai_npcs
```

### Running

**With Mock AI** (no API key needed):
```bash
./bin/visual_ai_npcs
```

**With Real Claude AI**:
```bash
ANTHROPIC_API_KEY=your_key_here ./bin/visual_ai_npcs
```

## Controls

| Control | Action |
|---------|--------|
| **Mouse Click** | Click on NPC to talk |
| **1** | Make Merchant Tom think |
| **2** | Make Guard Sarah think |
| **3** | Make Wizard Aldric think |
| **SPACE** | Change time of day |
| **ESC** | Quit |

## Gameplay Flow

1. **Startup**: Window opens, 3 NPCs spawn at random positions
2. **Wandering**: NPCs autonomously move around the village
3. **Interaction**: Click an NPC to greet them
4. **AI Response**: NPC's AI generates contextual dialogue
5. **Speech Bubble**: Response displays above NPC for 5 seconds
6. **Memory**: NPC remembers the interaction for future conversations

## Performance

- **FPS**: 60 (capped with SDL_Delay(16))
- **Binary Size**: 493 KB
- **Memory**: ~50 MB (with SDL2 and AI framework)
- **AI Response Time**:
  - Mock: Instant
  - Claude API: 1-3 seconds (depending on network)

## Code Statistics

- **Main File**: `visual_ai_npcs.cpp` - 411 lines
- **Dependencies**: OraconEngine, OraconGfx, OraconAuto, SDL2
- **New Components**:
  - `window.h` - SDL2 window wrapper
  - `text.h` - Bitmap font renderer (C++17 compatible)

## Limitations

- **Font**: Simple 8x8 bitmap font (limited characters)
- **Graphics**: 2D software rendering (no GPU acceleration for sprites)
- **AI Latency**: Real Claude API has network delay
- **No Physics**: NPCs don't collide with each other
- **Single Scene**: No scene transitions

## Future Enhancements

Potential improvements:

1. **Better Graphics**
   - Sprite sheets for NPC animations
   - Tile-based background
   - Particle effects

2. **More Interactions**
   - Trading system with Merchant
   - Quest giving
   - NPC-to-NPC conversations

3. **Advanced AI**
   - Emotional states
   - Relationship systems
   - Long-term memory
   - Goal-oriented behavior

4. **Gameplay**
   - Player avatar
   - Inventory system
   - Quest log
   - Multiple locations

5. **Performance**
   - GPU-accelerated rendering
   - Background AI processing
   - Response caching

## Showcase Value

This demo demonstrates:

✅ **Full Stack Integration**: Engine + Graphics + AI working seamlessly
✅ **Real-time AI**: Conversational NPCs in a game
✅ **Visual Feedback**: Speech bubbles, animations, indicators
✅ **Interactive**: Click and keyboard controls
✅ **Autonomous Behavior**: NPCs think and act independently
✅ **Production Ready**: Clean code, modular architecture

Perfect for:
- Game development portfolio
- AI integration demonstration
- Educational purposes
- Proof of concept for larger projects

## Conclusion

The Visual AI NPC Demo successfully proves that AI-powered NPCs can be integrated into a game engine with minimal complexity. The combination of OraconEngine's ECS system, OraconGfx's rendering, and OraconAuto's agent framework creates a compelling, interactive experience.

This is not just a tech demo - it's a foundation for building immersive games with truly intelligent NPCs that can engage players in dynamic, unpredictable ways.

---

**Next Steps**: Try running the demo and talking to the NPCs. Each conversation will be unique and contextually relevant to the game state!
