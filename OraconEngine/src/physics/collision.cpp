#include "oracon/engine/physics.h"

namespace oracon {
namespace engine {

bool PhysicsSystem::checkCollision(const Entity* a, const Entity* b) {
    auto* colliderA = a->getComponent<Collider>();
    auto* colliderB = b->getComponent<Collider>();
    auto* transformA = a->getComponent<Transform>();
    auto* transformB = b->getComponent<Transform>();
    
    if (!colliderA || !colliderB || !transformA || !transformB) return false;
    
    if (colliderA->type == Collider::Type::Box && colliderB->type == Collider::Type::Box) {
        auto* boxA = static_cast<const BoxCollider*>(colliderA);
        auto* boxB = static_cast<const BoxCollider*>(colliderB);
        
        Vec2f posA = transformA->position + colliderA->offset;
        Vec2f posB = transformB->position + colliderB->offset;
        
        return !(posA.x + boxA->size.x / 2 < posB.x - boxB->size.x / 2 ||
                 posA.x - boxA->size.x / 2 > posB.x + boxB->size.x / 2 ||
                 posA.y + boxA->size.y / 2 < posB.y - boxB->size.y / 2 ||
                 posA.y - boxA->size.y / 2 > posB.y + boxB->size.y / 2);
    }
    
    return false;
}

void PhysicsSystem::applyPhysics(Entity* entity, f32 deltaTime, const Vec2f& gravity) {
    auto* rb = entity->getComponent<Rigidbody>();
    auto* transform = entity->getComponent<Transform>();
    
    if (!rb || !transform || rb->isKinematic) return;
    
    if (rb->useGravity) {
        rb->acceleration += gravity;
    }
    
    rb->velocity += rb->acceleration * deltaTime;
    rb->velocity *= (1.0f - rb->drag * deltaTime);
    
    transform->position += rb->velocity * deltaTime;
    
    rb->acceleration = Vec2f(0.0f, 0.0f);
}

} // namespace engine
} // namespace oracon
