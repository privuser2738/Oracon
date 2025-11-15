#include "oracon/engine/script.h"
#include "oracon/engine/entity.h"
#include "oracon/engine/world.h"
#include "oracon/engine/input.h"
#include "oracon/core/logger.h"
#include <fstream>
#include <sstream>

namespace oracon {
namespace engine {

// Static members for ScriptingAPI
Entity* ScriptingAPI::s_currentEntity = nullptr;
World* ScriptingAPI::s_currentWorld = nullptr;

ScriptComponent::ScriptComponent(const String& code)
    : m_code(code)
    , m_initialized(false)
{}

ScriptComponent* ScriptComponent::fromFile(const String& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        ORACON_LOG_ERROR("Failed to open script file: " + filepath);
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return new ScriptComponent(buffer.str());
}

void ScriptComponent::setCode(const String& code) {
    m_code = code;
    m_initialized = false;
    m_program.reset();
}

void ScriptComponent::compile() {
    if (m_code.empty()) {
        return;
    }

    // Tokenize
    m_lexer = std::make_unique<lang::Lexer>(m_code);
    auto tokens = m_lexer->tokenize();

    // Parse
    m_parser = std::make_unique<lang::Parser>(tokens);
    m_program = m_parser->parse();

    // Check for parse errors
    if (m_parser->hasError()) {
        for (const auto& error : m_parser->getErrors()) {
            ORACON_LOG_ERROR("Script parse error: " + error);
        }
        return;
    }

    // Create interpreter
    m_interpreter = std::make_unique<lang::Interpreter>();
    m_initialized = true;
}

void ScriptComponent::setupAPI(Entity* entity, World* world) {
    if (!m_interpreter) return;

    ScriptingAPI::registerBuiltins(m_interpreter.get(), entity, world);
}

void ScriptComponent::onStart(Entity* entity, World* world) {
    if (!m_initialized) {
        compile();
    }

    if (!m_initialized || !m_program) {
        return;
    }

    setupAPI(entity, world);

    // Execute the script
    m_interpreter->execute(m_program.get());

    // Check for runtime errors
    if (m_interpreter->hasError()) {
        for (const auto& error : m_interpreter->getErrors()) {
            ORACON_LOG_ERROR("Script runtime error: " + error);
        }
    }
}

void ScriptComponent::onUpdate(Entity* entity, World* world, f32 deltaTime) {
    if (!m_initialized || !m_interpreter) return;

    setupAPI(entity, world);

    // Call update function if it exists
    if (!m_interpreter->getGlobalEnv().has("update")) {
        return; // No update function defined, silently skip
    }

    // Call update function through interpreter
    std::vector<lang::Value> args;
    args.push_back(lang::Value(static_cast<lang::f64>(deltaTime)));

    m_interpreter->callFunction("update", args);

    // Check for errors after calling
    if (m_interpreter->hasError()) {
        for (const auto& error : m_interpreter->getErrors()) {
            ORACON_LOG_ERROR("Script update error: " + error);
        }
    }
}

void ScriptComponent::onFixedUpdate(Entity* entity, World* world, f32 fixedDeltaTime) {
    if (!m_initialized || !m_interpreter) return;

    setupAPI(entity, world);

    // Call fixedUpdate function if it exists
    if (!m_interpreter->getGlobalEnv().has("fixedUpdate")) {
        return; // No fixedUpdate function defined, silently skip
    }

    // Call fixedUpdate function through interpreter
    std::vector<lang::Value> args;
    args.push_back(lang::Value(static_cast<lang::f64>(fixedDeltaTime)));

    m_interpreter->callFunction("fixedUpdate", args);

    // Check for errors after calling
    if (m_interpreter->hasError()) {
        for (const auto& error : m_interpreter->getErrors()) {
            ORACON_LOG_ERROR("Script fixedUpdate error: " + error);
        }
    }
}

bool ScriptComponent::hasErrors() const {
    if (m_parser && m_parser->hasError()) return true;
    if (m_interpreter && m_interpreter->hasError()) return true;
    return false;
}

String ScriptComponent::getErrors() const {
    String errors;
    if (m_parser && m_parser->hasError()) {
        for (const auto& err : m_parser->getErrors()) {
            errors += "Parse: " + err + "\n";
        }
    }
    if (m_interpreter && m_interpreter->hasError()) {
        for (const auto& err : m_interpreter->getErrors()) {
            errors += "Runtime: " + err + "\n";
        }
    }
    return errors;
}

// ===== Scripting API Implementation =====

void ScriptingAPI::registerBuiltins(lang::Interpreter* interpreter, Entity* entity, World* world) {
    s_currentEntity = entity;
    s_currentWorld = world;

    auto& env = interpreter->getGlobalEnv();

    // getPosition() - returns entity position as array [x, y]
    auto getPosFn = std::make_shared<lang::Function>("getPosition", 0,
        [](const std::vector<lang::Value>& args) -> lang::Value {
            (void)args;
            if (!s_currentEntity) return lang::Value();

            auto* transform = s_currentEntity->getComponent<Transform>();
            if (!transform) return lang::Value();

            std::vector<lang::Value> pos;
            pos.push_back(lang::Value(static_cast<lang::f64>(transform->position.x)));
            pos.push_back(lang::Value(static_cast<lang::f64>(transform->position.y)));
            return lang::Value::createArray(pos);
        });
    env.define("getPosition", lang::Value(getPosFn));

    // setPosition(x, y) - sets entity position
    auto setPosFn = std::make_shared<lang::Function>("setPosition", 2,
        [](const std::vector<lang::Value>& args) -> lang::Value {
            if (args.size() != 2 || !s_currentEntity) return lang::Value();

            auto* transform = s_currentEntity->getComponent<Transform>();
            if (!transform) return lang::Value();

            transform->position.x = static_cast<f32>(args[0].asFloat());
            transform->position.y = static_cast<f32>(args[1].asFloat());
            return lang::Value();
        });
    env.define("setPosition", lang::Value(setPosFn));

    // getVelocity() - returns velocity as array [vx, vy]
    auto getVelFn = std::make_shared<lang::Function>("getVelocity", 0,
        [](const std::vector<lang::Value>& args) -> lang::Value {
            (void)args;
            if (!s_currentEntity) return lang::Value();

            auto* rb = s_currentEntity->getComponent<Rigidbody>();
            if (!rb) return lang::Value();

            std::vector<lang::Value> vel;
            vel.push_back(lang::Value(static_cast<lang::f64>(rb->velocity.x)));
            vel.push_back(lang::Value(static_cast<lang::f64>(rb->velocity.y)));
            return lang::Value::createArray(vel);
        });
    env.define("getVelocity", lang::Value(getVelFn));

    // setVelocity(vx, vy) - sets velocity
    auto setVelFn = std::make_shared<lang::Function>("setVelocity", 2,
        [](const std::vector<lang::Value>& args) -> lang::Value {
            if (args.size() != 2 || !s_currentEntity) return lang::Value();

            auto* rb = s_currentEntity->getComponent<Rigidbody>();
            if (!rb) return lang::Value();

            rb->velocity.x = static_cast<f32>(args[0].asFloat());
            rb->velocity.y = static_cast<f32>(args[1].asFloat());
            return lang::Value();
        });
    env.define("setVelocity", lang::Value(setVelFn));

    // log(message) - logs to console
    auto logFn = std::make_shared<lang::Function>("log", 1,
        [](const std::vector<lang::Value>& args) -> lang::Value {
            if (!args.empty()) {
                ORACON_LOG_INFO("Script: " + args[0].toString());
            }
            return lang::Value();
        });
    env.define("log", lang::Value(logFn));
}

} // namespace engine
} // namespace oracon
