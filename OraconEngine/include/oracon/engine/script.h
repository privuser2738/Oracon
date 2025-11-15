#ifndef ORACON_ENGINE_SCRIPT_H
#define ORACON_ENGINE_SCRIPT_H

#include "oracon/core/types.h"
#include "oracon/engine/component.h"
#include "oracon/lang/interpreter/interpreter.h"
#include "oracon/lang/parser/parser.h"
#include "oracon/lang/lexer/lexer.h"
#include <memory>

namespace oracon {
namespace engine {

using core::String;
using core::f32;

// Forward declarations
class Entity;
class World;
class Input;

// Script component that executes OraconLang code
class ScriptComponent : public Component {
public:
    ScriptComponent() = default;
    explicit ScriptComponent(const String& code);

    // Load script from file
    static ScriptComponent* fromFile(const String& filepath);

    // Set script code
    void setCode(const String& code);
    const String& getCode() const { return m_code; }

    // Script lifecycle callbacks
    void onStart(Entity* entity, World* world);
    void onUpdate(Entity* entity, World* world, f32 deltaTime);
    void onFixedUpdate(Entity* entity, World* world, f32 fixedDeltaTime);

    // Check if script has errors
    bool hasErrors() const;
    String getErrors() const;

private:
    String m_code;
    std::unique_ptr<lang::Lexer> m_lexer;
    std::unique_ptr<lang::Parser> m_parser;
    std::unique_ptr<lang::Interpreter> m_interpreter;
    std::unique_ptr<lang::Program> m_program;
    bool m_initialized = false;

    void compile();
    void setupAPI(Entity* entity, World* world);
};

// Scripting API - exposes engine functionality to scripts
class ScriptingAPI {
public:
    static void registerBuiltins(lang::Interpreter* interpreter, Entity* entity, World* world);

private:
    static Entity* s_currentEntity;
    static World* s_currentWorld;
};

} // namespace engine
} // namespace oracon

#endif // ORACON_ENGINE_SCRIPT_H
