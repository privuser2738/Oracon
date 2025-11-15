#include "oracon/lang/interpreter/value.h"
#include "oracon/lang/interpreter/environment.h"
#include "oracon/lang/ast/ast.h"
#include <sstream>

namespace oracon {
namespace lang {

// ===== Function Implementation =====

Function::Function(const FunctionStmt* declaration, Environment* closure)
    : m_name(declaration->getName().getLexeme())
    , m_arity(declaration->getParameters().size())
    , m_isNative(false)
    , m_declaration(declaration)
    , m_closure(closure)
    , m_nativeFunction(nullptr)
{}

Function::Function(const String& name, usize arity, NativeFunction fn)
    : m_name(name)
    , m_arity(arity)
    , m_isNative(true)
    , m_declaration(nullptr)
    , m_closure(nullptr)
    , m_nativeFunction(fn)
{}

Value Function::call(const std::vector<Value>& arguments, Environment* globals) {
    // Native function
    if (m_isNative) {
        return m_nativeFunction(arguments);
    }

    // User-defined function
    // Create new environment for function scope
    Environment functionEnv(m_closure);

    // Bind parameters to arguments
    const auto& params = m_declaration->getParameters();
    for (usize i = 0; i < params.size(); ++i) {
        functionEnv.define(params[i].getLexeme(), arguments[i]);
    }

    // Execute function body
    // This is a placeholder - we'll need to integrate with the interpreter
    // For now, return nil
    (void)globals;
    return Value();
}

// ===== Value Implementation =====

ValueType Value::getType() const {
    return static_cast<ValueType>(m_data.index());
}

String Value::toString() const {
    std::ostringstream oss;

    switch (getType()) {
        case ValueType::Nil:
            return "nil";
        case ValueType::Boolean:
            return std::get<bool>(m_data) ? "true" : "false";
        case ValueType::Integer:
            oss << std::get<i64>(m_data);
            return oss.str();
        case ValueType::Float:
            oss << std::get<f64>(m_data);
            return oss.str();
        case ValueType::String:
            return std::get<String>(m_data);
        case ValueType::Array: {
            oss << "[";
            auto arr = std::get<ArrayType>(m_data);
            for (usize i = 0; i < arr->size(); ++i) {
                if (i > 0) oss << ", ";
                oss << (*arr)[i].toString();
            }
            oss << "]";
            return oss.str();
        }
        case ValueType::Map: {
            oss << "{";
            auto map = std::get<MapType>(m_data);
            bool first = true;
            for (const auto& pair : *map) {
                if (!first) oss << ", ";
                first = false;
                oss << pair.first << ": " << pair.second.toString();
            }
            oss << "}";
            return oss.str();
        }
        case ValueType::Function: {
            auto fn = std::get<FunctionType>(m_data);
            oss << "<function " << fn->name() << ">";
            return oss.str();
        }
    }

    return "unknown";
}

bool Value::asBool() const {
    switch (getType()) {
        case ValueType::Nil:
            return false;
        case ValueType::Boolean:
            return std::get<bool>(m_data);
        case ValueType::Integer:
            return std::get<i64>(m_data) != 0;
        case ValueType::Float:
            return std::get<f64>(m_data) != 0.0;
        case ValueType::String:
            return !std::get<String>(m_data).empty();
        case ValueType::Array:
            return !std::get<ArrayType>(m_data)->empty();
        case ValueType::Map:
            return !std::get<MapType>(m_data)->empty();
        case ValueType::Function:
            return true;  // Functions are always truthy
    }
    return false;
}

i64 Value::asInteger() const {
    if (getType() == ValueType::Integer) {
        return std::get<i64>(m_data);
    } else if (getType() == ValueType::Float) {
        return static_cast<i64>(std::get<f64>(m_data));
    } else if (getType() == ValueType::Boolean) {
        return std::get<bool>(m_data) ? 1 : 0;
    }
    return 0;
}

f64 Value::asFloat() const {
    if (getType() == ValueType::Float) {
        return std::get<f64>(m_data);
    } else if (getType() == ValueType::Integer) {
        return static_cast<f64>(std::get<i64>(m_data));
    } else if (getType() == ValueType::Boolean) {
        return std::get<bool>(m_data) ? 1.0 : 0.0;
    }
    return 0.0;
}

String Value::asString() const {
    if (getType() == ValueType::String) {
        return std::get<String>(m_data);
    }
    return toString();
}

// Array operations
usize Value::arraySize() const {
    if (!isArray()) return 0;
    return std::get<ArrayType>(m_data)->size();
}

Value Value::arrayGet(usize index) const {
    if (!isArray()) return Value();
    auto arr = std::get<ArrayType>(m_data);
    if (index >= arr->size()) return Value();
    return (*arr)[index];
}

void Value::arraySet(usize index, const Value& value) {
    if (!isArray()) return;
    auto arr = std::get<ArrayType>(m_data);
    if (index >= arr->size()) return;
    (*arr)[index] = value;
}

void Value::arrayPush(const Value& value) {
    if (!isArray()) return;
    std::get<ArrayType>(m_data)->push_back(value);
}

Value Value::arrayPop() {
    if (!isArray() || arraySize() == 0) return Value();
    auto arr = std::get<ArrayType>(m_data);
    Value result = arr->back();
    arr->pop_back();
    return result;
}

// Map operations
usize Value::mapSize() const {
    if (!isMap()) return 0;
    return std::get<MapType>(m_data)->size();
}

Value Value::mapGet(const String& key) const {
    if (!isMap()) return Value();
    auto map = std::get<MapType>(m_data);
    auto it = map->find(key);
    if (it == map->end()) return Value();
    return it->second;
}

void Value::mapSet(const String& key, const Value& value) {
    if (!isMap()) return;
    (*std::get<MapType>(m_data))[key] = value;
}

bool Value::mapHas(const String& key) const {
    if (!isMap()) return false;
    return std::get<MapType>(m_data)->count(key) > 0;
}

void Value::mapDelete(const String& key) {
    if (!isMap()) return;
    std::get<MapType>(m_data)->erase(key);
}

// Function operations
FunctionType Value::asFunction() const {
    if (!isFunction()) return nullptr;
    return std::get<FunctionType>(m_data);
}

} // namespace lang
} // namespace oracon
