#ifndef ORACON_ENGINE_CAMERA_H
#define ORACON_ENGINE_CAMERA_H

#include "oracon/math/vector.h"

namespace oracon {
namespace engine {

using core::f32;
using math::Vec2f;

class Camera {
public:
    Camera();
    
    Vec2f position{0.0f, 0.0f};
    f32 zoom = 1.0f;
    
    Vec2f worldToScreen(const Vec2f& worldPos, const Vec2f& screenSize) const;
    Vec2f screenToWorld(const Vec2f& screenPos, const Vec2f& screenSize) const;
};

} // namespace engine
} // namespace oracon

#endif
