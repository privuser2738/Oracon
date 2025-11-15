#ifndef ORACON_ENGINE_COMPONENT_H
#define ORACON_ENGINE_COMPONENT_H

#include "oracon/core/types.h"
#include "oracon/math/vector.h"
#include "oracon/gfx/primitives.h"
#include "oracon/gfx/sprite.h"
#include <memory>

namespace oracon {
namespace engine {

using core::String;
using core::f32;
using core::i32;
using math::Vec2f;
using gfx::Color;
using gfx::Sprite;

// Base component class
struct Component {
    virtual ~Component() = default;
    bool enabled = true;
};

// Transform component - position, rotation, scale
struct Transform : public Component {
    Vec2f position{0.0f, 0.0f};
    f32 rotation = 0.0f;  // In radians
    Vec2f scale{1.0f, 1.0f};

    Transform() = default;
    Transform(const Vec2f& pos) : position(pos) {}
    Transform(f32 x, f32 y) : position(x, y) {}
};

// Sprite renderer component
struct SpriteRenderer : public Component {
    std::shared_ptr<Sprite> sprite;
    Color tint = Color::white();
    i32 sortingLayer = 0;  // For render ordering

    SpriteRenderer() = default;
    explicit SpriteRenderer(std::shared_ptr<Sprite> spr) : sprite(spr) {}
};

// Rigidbody component - physics properties
struct Rigidbody : public Component {
    Vec2f velocity{0.0f, 0.0f};
    Vec2f acceleration{0.0f, 0.0f};
    f32 mass = 1.0f;
    f32 drag = 0.0f;  // Linear drag
    f32 bounciness = 0.0f;  // Coefficient of restitution (0 = no bounce, 1 = perfect bounce)
    f32 angularVelocity = 0.0f;
    f32 angularDrag = 0.0f;
    bool useGravity = true;
    bool isKinematic = false;  // If true, not affected by physics forces

    Rigidbody() = default;
};

// Collider component base
struct Collider : public Component {
    enum class Type {
        Box,
        Circle
    };

    Type type;
    bool isTrigger = false;  // Trigger colliders don't apply physics forces
    Vec2f offset{0.0f, 0.0f};  // Offset from entity position

    virtual ~Collider() = default;

protected:
    explicit Collider(Type t) : type(t) {}
};

// Box collider
struct BoxCollider : public Collider {
    Vec2f size{1.0f, 1.0f};

    BoxCollider() : Collider(Type::Box) {}
    BoxCollider(f32 width, f32 height) : Collider(Type::Box), size(width, height) {}
    BoxCollider(const Vec2f& sz) : Collider(Type::Box), size(sz) {}
};

// Circle collider
struct CircleCollider : public Collider {
    f32 radius = 0.5f;

    CircleCollider() : Collider(Type::Circle) {}
    explicit CircleCollider(f32 r) : Collider(Type::Circle), radius(r) {}
};

// Script component - for custom game logic
struct Script : public Component {
    String scriptName;

    virtual void onStart() {}
    virtual void onUpdate(f32 deltaTime) { (void)deltaTime; }
    virtual void onFixedUpdate(f32 fixedDeltaTime) { (void)fixedDeltaTime; }
    virtual void onCollision(class Entity* other) { (void)other; }

    virtual ~Script() = default;

protected:
    Script() = default;
};

// Tag component - for identifying entities
struct Tag : public Component {
    String tag;

    Tag() = default;
    explicit Tag(const String& t) : tag(t) {}
};

} // namespace engine
} // namespace oracon

#endif // ORACON_ENGINE_COMPONENT_H
