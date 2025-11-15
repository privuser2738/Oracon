#ifndef ORACON_LANG_INTERPRETER_ENVIRONMENT_H
#define ORACON_LANG_INTERPRETER_ENVIRONMENT_H

#include "oracon/lang/interpreter/value.h"
#include <unordered_map>
#include <memory>

namespace oracon {
namespace lang {

class Environment {
public:
    Environment() : m_parent(nullptr) {}
    explicit Environment(Environment* parent) : m_parent(parent) {}

    void define(const String& name, const Value& value);
    Value get(const String& name) const;
    void set(const String& name, const Value& value);
    bool has(const String& name) const;
    bool hasLocal(const String& name) const;

private:
    std::unordered_map<String, Value> m_values;
    Environment* m_parent;
};

} // namespace lang
} // namespace oracon

#endif // ORACON_LANG_INTERPRETER_ENVIRONMENT_H
