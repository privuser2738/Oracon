#ifndef ORACON_LANG_INTERPRETER_VALUE_H
#define ORACON_LANG_INTERPRETER_VALUE_H

#include "oracon/core/types.h"
#include <variant>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace oracon {
namespace lang {

using core::i64;
using core::f64;
using core::String;
using core::usize;

// Forward declarations
class Value;
class Environment;
class FunctionStmt;

// Function types
using NativeFunction = std::function<Value(const std::vector<Value>&)>;

// Callable function object
class Function {
public:
    // User-defined function
    Function(const FunctionStmt* declaration, Environment* closure);

    // Native/built-in function
    Function(const String& name, usize arity, NativeFunction fn);

    Value call(const std::vector<Value>& arguments, Environment* globals);

    usize arity() const { return m_arity; }
    const String& name() const { return m_name; }
    bool isNative() const { return m_isNative; }
    const FunctionStmt* getDeclaration() const { return m_declaration; }
    Environment* getClosure() const { return m_closure; }

private:
    String m_name;
    usize m_arity;
    bool m_isNative;

    // For user-defined functions
    const FunctionStmt* m_declaration;
    Environment* m_closure;

    // For native functions
    NativeFunction m_nativeFunction;
};

using FunctionType = std::shared_ptr<Function>;

// Array and Map types
using ArrayType = std::shared_ptr<std::vector<Value>>;
using MapType = std::shared_ptr<std::unordered_map<String, Value>>;

enum class ValueType {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Array,
    Map,
    Function
};

class Value {
public:
    Value() : m_data(nullptr) {}
    explicit Value(bool b) : m_data(b) {}
    explicit Value(i64 i) : m_data(i) {}
    explicit Value(f64 f) : m_data(f) {}
    explicit Value(const String& s) : m_data(s) {}
    explicit Value(const ArrayType& arr) : m_data(arr) {}
    explicit Value(const MapType& map) : m_data(map) {}
    explicit Value(const FunctionType& fn) : m_data(fn) {}

    // Create empty array
    static Value createArray() {
        return Value(std::make_shared<std::vector<Value>>());
    }

    // Create array from initializer list
    static Value createArray(const std::vector<Value>& values) {
        return Value(std::make_shared<std::vector<Value>>(values));
    }

    // Create empty map
    static Value createMap() {
        return Value(std::make_shared<std::unordered_map<String, Value>>());
    }

    ValueType getType() const;
    String toString() const;

    // Type conversion
    bool asBool() const;
    i64 asInteger() const;
    f64 asFloat() const;
    String asString() const;

    // Type checking
    bool isNil() const { return getType() == ValueType::Nil; }
    bool isBool() const { return getType() == ValueType::Boolean; }
    bool isInteger() const { return getType() == ValueType::Integer; }
    bool isFloat() const { return getType() == ValueType::Float; }
    bool isString() const { return getType() == ValueType::String; }
    bool isArray() const { return getType() == ValueType::Array; }
    bool isMap() const { return getType() == ValueType::Map; }
    bool isFunction() const { return getType() == ValueType::Function; }
    bool isNumber() const { return isInteger() || isFloat(); }

    // Array operations
    usize arraySize() const;
    Value arrayGet(usize index) const;
    void arraySet(usize index, const Value& value);
    void arrayPush(const Value& value);
    Value arrayPop();

    // Map operations
    usize mapSize() const;
    Value mapGet(const String& key) const;
    void mapSet(const String& key, const Value& value);
    bool mapHas(const String& key) const;
    void mapDelete(const String& key);

    // Function operations
    FunctionType asFunction() const;

    // Get raw data (for operations)
    template<typename T>
    T get() const { return std::get<T>(m_data); }

private:
    using DataType = std::variant<std::nullptr_t, bool, i64, f64, String, ArrayType, MapType, FunctionType>;
    DataType m_data;
};

} // namespace lang
} // namespace oracon

#endif // ORACON_LANG_INTERPRETER_VALUE_H
