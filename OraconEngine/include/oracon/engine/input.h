#ifndef ORACON_ENGINE_INPUT_H
#define ORACON_ENGINE_INPUT_H

#include "oracon/core/types.h"
#include "oracon/math/vector.h"
#include <unordered_set>

namespace oracon {
namespace engine {

using math::Vec2f;

enum class KeyCode {
    W, A, S, D, Space, Escape, Left, Right, Up, Down
};

class Input {
public:
    void update();
    void keyDown(KeyCode key);
    void keyUp(KeyCode key);
    
    bool isKeyPressed(KeyCode key) const;
    bool isKeyJustPressed(KeyCode key) const;
    
    Vec2f mousePosition() const { return m_mousePos; }
    void setMousePosition(const Vec2f& pos) { m_mousePos = pos; }

private:
    std::unordered_set<int> m_currentKeys;
    std::unordered_set<int> m_lastKeys;
    Vec2f m_mousePos;
};

} // namespace engine
} // namespace oracon

#endif
