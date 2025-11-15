#ifndef ORACON_ENGINE_PHYSICS_H
#define ORACON_ENGINE_PHYSICS_H

#include "oracon/engine/entity.h"

namespace oracon {
namespace engine {

class PhysicsSystem {
public:
    static bool checkCollision(const Entity* a, const Entity* b);
    static void applyPhysics(Entity* entity, f32 deltaTime, const Vec2f& gravity);
};

} // namespace engine
} // namespace oracon

#endif
