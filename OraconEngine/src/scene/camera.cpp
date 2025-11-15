#include "oracon/engine/camera.h"

namespace oracon {
namespace engine {

Camera::Camera() {}

Vec2f Camera::worldToScreen(const Vec2f& worldPos, const Vec2f& screenSize) const {
    Vec2f offset = (worldPos - position) * zoom;
    return Vec2f(screenSize.x / 2.0f + offset.x, screenSize.y / 2.0f + offset.y);
}

Vec2f Camera::screenToWorld(const Vec2f& screenPos, const Vec2f& screenSize) const {
    Vec2f centered = Vec2f(screenPos.x - screenSize.x / 2.0f, screenPos.y - screenSize.y / 2.0f);
    return position + centered / zoom;
}

} // namespace engine
} // namespace oracon
