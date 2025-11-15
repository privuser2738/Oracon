#include "oracon/engine/input.h"

namespace oracon {
namespace engine {

void Input::update() {
    m_lastKeys = m_currentKeys;
}

void Input::keyDown(KeyCode key) {
    m_currentKeys.insert(static_cast<int>(key));
}

void Input::keyUp(KeyCode key) {
    m_currentKeys.erase(static_cast<int>(key));
}

bool Input::isKeyPressed(KeyCode key) const {
    return m_currentKeys.find(static_cast<int>(key)) != m_currentKeys.end();
}

bool Input::isKeyJustPressed(KeyCode key) const {
    int keyInt = static_cast<int>(key);
    return m_currentKeys.find(keyInt) != m_currentKeys.end() &&
           m_lastKeys.find(keyInt) == m_lastKeys.end();
}

} // namespace engine
} // namespace oracon
