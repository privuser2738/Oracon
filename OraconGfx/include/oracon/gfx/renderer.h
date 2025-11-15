#ifndef ORACON_GFX_RENDERER_H
#define ORACON_GFX_RENDERER_H

#include "oracon/gfx/canvas.h"
#include "oracon/gfx/primitives.h"
#include <memory>

namespace oracon {
namespace gfx {

using core::i32;

// 2D software renderer
class Renderer {
public:
    explicit Renderer(Canvas* canvas) : m_canvas(canvas), m_blendMode(BlendMode::Alpha) {}

    enum class BlendMode {
        Replace,  // No blending
        Alpha     // Alpha blending
    };

    // Get/set canvas
    Canvas* getCanvas() const { return m_canvas; }
    void setCanvas(Canvas* canvas) { m_canvas = canvas; }

    // Blend mode
    void setBlendMode(BlendMode mode) { m_blendMode = mode; }
    BlendMode getBlendMode() const { return m_blendMode; }

    // Clear canvas
    void clear(const Color& color = Color::transparent()) {
        if (m_canvas) {
            m_canvas->clear(color);
        }
    }

    // Draw primitives
    void drawPoint(const Point& point);
    void drawLine(const Line& line);
    void drawRect(const Rect& rect, bool filled = false);
    void drawCircle(const Circle& circle, bool filled = false);
    void drawEllipse(const Ellipse& ellipse, bool filled = false);
    void drawTriangle(const Triangle& triangle, bool filled = false);
    void drawPolygon(const Polygon& polygon);
    void drawPath(const Path& path);

    // Convenience drawing methods
    void drawLine(const Vec2f& start, const Vec2f& end, const Color& color, f32 thickness = 1.0f) {
        drawLine(Line(start, end, color, thickness));
    }

    void drawRect(f32 x, f32 y, f32 w, f32 h, const Color& color, bool filled = false) {
        drawRect(Rect(x, y, w, h, color), filled);
    }

    void drawCircle(const Vec2f& center, f32 radius, const Color& color, bool filled = false) {
        drawCircle(Circle(center, radius, color), filled);
    }

    void fillRect(const Rect& rect) {
        drawRect(rect, true);
    }

    void fillCircle(const Circle& circle) {
        drawCircle(circle, true);
    }

    void fillTriangle(const Triangle& triangle) {
        drawTriangle(triangle, true);
    }

private:
    Canvas* m_canvas;
    BlendMode m_blendMode;

    void setPixel(u32 x, u32 y, const Color& color) {
        if (!m_canvas) return;

        if (m_blendMode == BlendMode::Alpha) {
            m_canvas->blendPixel(x, y, color);
        } else {
            m_canvas->setPixel(x, y, color);
        }
    }

    void drawLineBresenham(i32 x0, i32 y0, i32 x1, i32 y1, const Color& color);
    void drawCircleBresenham(i32 xc, i32 yc, i32 radius, const Color& color);
    void fillCircleScanline(i32 xc, i32 yc, i32 radius, const Color& color);
    void fillTriangleScanline(const Triangle& triangle);
};

} // namespace gfx
} // namespace oracon

#endif // ORACON_GFX_RENDERER_H
