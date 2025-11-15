#ifndef ORACON_GFX_SPRITE_H
#define ORACON_GFX_SPRITE_H

#include "oracon/gfx/canvas.h"
#include "oracon/math/vector.h"
#include <memory>

namespace oracon {
namespace gfx {

using math::Vec2f;

// Sprite represents a textured quad with transformation
class Sprite {
public:
    Sprite()
        : m_texture(nullptr)
        , m_position(0.0f, 0.0f)
        , m_scale(1.0f, 1.0f)
        , m_rotation(0.0f)
        , m_origin(0.0f, 0.0f)
        , m_tint(Color::white())
        , m_flipX(false)
        , m_flipY(false)
    {}

    explicit Sprite(std::shared_ptr<Canvas> texture)
        : m_texture(texture)
        , m_position(0.0f, 0.0f)
        , m_scale(1.0f, 1.0f)
        , m_rotation(0.0f)
        , m_origin(0.0f, 0.0f)
        , m_tint(Color::white())
        , m_flipX(false)
        , m_flipY(false)
    {}

    // Texture
    void setTexture(std::shared_ptr<Canvas> texture) { m_texture = texture; }
    std::shared_ptr<Canvas> getTexture() const { return m_texture; }

    // Transform
    void setPosition(const Vec2f& pos) { m_position = pos; }
    void setPosition(f32 x, f32 y) { m_position = Vec2f(x, y); }
    const Vec2f& getPosition() const { return m_position; }

    void setScale(const Vec2f& scale) { m_scale = scale; }
    void setScale(f32 x, f32 y) { m_scale = Vec2f(x, y); }
    void setScale(f32 uniform) { m_scale = Vec2f(uniform, uniform); }
    const Vec2f& getScale() const { return m_scale; }

    void setRotation(f32 angle) { m_rotation = angle; }
    f32 getRotation() const { return m_rotation; }

    void setOrigin(const Vec2f& origin) { m_origin = origin; }
    void setOrigin(f32 x, f32 y) { m_origin = Vec2f(x, y); }
    const Vec2f& getOrigin() const { return m_origin; }

    // Appearance
    void setTint(const Color& color) { m_tint = color; }
    const Color& getTint() const { return m_tint; }

    void setFlipX(bool flip) { m_flipX = flip; }
    bool getFlipX() const { return m_flipX; }

    void setFlipY(bool flip) { m_flipY = flip; }
    bool getFlipY() const { return m_flipY; }

    // Size
    Vec2f getSize() const {
        if (!m_texture) return Vec2f(0.0f, 0.0f);
        return Vec2f(
            static_cast<f32>(m_texture->getWidth()),
            static_cast<f32>(m_texture->getHeight())
        );
    }

    // Bounds (after transformation)
    Rect getBounds() const {
        Vec2f size = getSize();
        Vec2f scaledSize = Vec2f(size.x * m_scale.x, size.y * m_scale.y);
        Vec2f topLeft = m_position - m_origin;
        return Rect(topLeft, scaledSize);
    }

private:
    std::shared_ptr<Canvas> m_texture;
    Vec2f m_position;
    Vec2f m_scale;
    f32 m_rotation;
    Vec2f m_origin;
    Color m_tint;
    bool m_flipX;
    bool m_flipY;
};

// Animated sprite with frame support
class AnimatedSprite : public Sprite {
public:
    AnimatedSprite() : Sprite(), m_currentFrame(0), m_frameTime(0.0f), m_frameDuration(0.1f) {}

    void addFrame(std::shared_ptr<Canvas> frame) {
        m_frames.push_back(frame);
        if (m_frames.size() == 1) {
            setTexture(frame);
        }
    }

    void setCurrentFrame(size_t index) {
        if (index < m_frames.size()) {
            m_currentFrame = index;
            setTexture(m_frames[index]);
        }
    }

    size_t getCurrentFrame() const { return m_currentFrame; }
    size_t getFrameCount() const { return m_frames.size(); }

    void setFrameDuration(f32 duration) { m_frameDuration = duration; }
    f32 getFrameDuration() const { return m_frameDuration; }

    void update(f32 deltaTime) {
        if (m_frames.size() <= 1) return;

        m_frameTime += deltaTime;
        if (m_frameTime >= m_frameDuration) {
            m_frameTime = 0.0f;
            m_currentFrame = (m_currentFrame + 1) % m_frames.size();
            setTexture(m_frames[m_currentFrame]);
        }
    }

private:
    std::vector<std::shared_ptr<Canvas>> m_frames;
    size_t m_currentFrame;
    f32 m_frameTime;
    f32 m_frameDuration;
};

} // namespace gfx
} // namespace oracon

#endif // ORACON_GFX_SPRITE_H
