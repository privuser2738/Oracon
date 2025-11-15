#include "oracon/lang/interpreter/environment.h"
#include <stdexcept>

namespace oracon {
namespace lang {

void Environment::define(const String& name, const Value& value) {
    m_values[name] = value;
}

Value Environment::get(const String& name) const {
    auto it = m_values.find(name);
    if (it != m_values.end()) {
        return it->second;
    }

    // Look in parent scope
    if (m_parent != nullptr) {
        return m_parent->get(name);
    }

    throw std::runtime_error("Undefined variable: " + name);
}

void Environment::set(const String& name, const Value& value) {
    auto it = m_values.find(name);
    if (it != m_values.end()) {
        it->second = value;
        return;
    }

    // Try to set in parent scope
    if (m_parent != nullptr && m_parent->has(name)) {
        m_parent->set(name, value);
        return;
    }

    throw std::runtime_error("Undefined variable: " + name);
}

bool Environment::has(const String& name) const {
    if (m_values.find(name) != m_values.end()) {
        return true;
    }

    if (m_parent != nullptr) {
        return m_parent->has(name);
    }

    return false;
}

bool Environment::hasLocal(const String& name) const {
    return m_values.find(name) != m_values.end();
}

} // namespace lang
} // namespace oracon
