# Oracon Ecosystem

A comprehensive programming language and development platform with integrated AI capabilities.

## Components

### OraconCore
Global core system providing shared utilities and infrastructure for all components.

### OraconLang
The Oracon programming language - a dynamic, modern language implemented in C++.
- **Hybrid execution model**: Interpreted with future JIT compilation
- **Dynamic typing** with optional static typing
- **First-class functions** and closures
- **Object-oriented** and **functional** programming support

### OraconMath
Cross-platform mathematical library for C/C++ and Oracon language.
- Linear algebra (vectors, matrices)
- Numerical analysis
- Statistical functions
- Computational geometry

### OraconIntegrate
Cross-platform remote desktop, file sharing, and system integration tool.
- **Network Communication**: Server/client architecture with auto-discovery
- **File Transfer**: Bidirectional file sharing with resume capability
- **Remote Terminal**: Cross-platform shell access (Linux, Windows, macOS)
- **Desktop Streaming**: Real-time desktop capture with GPU acceleration
- **System Service**: Run as background service/daemon with auto-start
- **Security**: TLS 1.3 encryption with key-based authentication

**Supported Platforms**: Linux, Windows 10+, macOS 10.14+

### OraconGfx
Integrated 2D and 3D graphics engine.
- 2D rendering engine
- 3D rendering engine
- Shader support
- Cross-platform windowing

### OraconEngine ✅
2D game engine with Entity-Component-System (ECS) architecture.
- **ECS System**: Entities, Components, and World management
- **Physics**: Rigidbody, colliders (box, circle), collision detection
- **Scripting**: Custom Script components for game logic
- **AI NPCs**: Integration with OraconAuto for intelligent game characters
- **Game Loop**: Fixed timestep physics with variable rendering

**AI NPC Features:**
- Personality-driven behavior
- Environmental perception
- Conversational memory
- Autonomous thinking
- Event-driven reactions

### OraconAuto ✅
AI/LLM automation and agent framework for building intelligent applications.
- **LLM Client Abstraction**: Support for multiple providers (Anthropic Claude, OpenAI GPT, local models)
- **Agent Framework**: Conversational agents with memory and tool support
- **Tool System**: Extensible tools for file I/O, code execution, calculations, and more
- **Memory Strategies**: Conversation history, sliding window, and summarization
- **Streaming Support**: Real-time streaming responses via SSE
- **OraconEngine Integration**: AI-powered NPCs with realistic behavior

**Demos Available:**
- `basic_agent` - Core agent functionality
- `claude_demo` - Anthropic Claude API integration
- `coding_assistant` - AI assistant with tools
- `ai_npc_demo` - AI-powered game NPCs

## Build Instructions

### Quick Build (All Components)

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### Building OraconIntegrate with Platform Scripts

**Linux:**
```bash
cd OraconIntegrate/scripts
./build-linux.sh Release
```

**Windows:**
```cmd
cd OraconIntegrate\scripts
build-windows.bat Release
```

**macOS:**
```bash
cd OraconIntegrate/scripts
./build-macos.sh Release
```

## Requirements

- C++17 or later
- CMake 3.15+
- A modern C++ compiler (GCC 7+, Clang 5+, MSVC 2017+)

## License

TBD
