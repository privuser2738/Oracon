#ifndef ORACON_GFX_PRIMITIVES_H
#define ORACON_GFX_PRIMITIVES_H

#include "oracon/gfx/color.h"
#include "oracon/math/vector.h"
#include <vector>

namespace oracon {
namespace gfx {

using math::Vec2f;
using math::Vec2i;

// ===== Point =====

struct Point {
    Vec2f position;
    Color color;

    Point() : position(0.0f, 0.0f), color(Color::white()) {}
    Point(f32 x, f32 y, const Color& color = Color::white())
        : position(x, y), color(color) {}
    Point(const Vec2f& pos, const Color& color = Color::white())
        : position(pos), color(color) {}
};

// ===== Line =====

struct Line {
    Vec2f start;
    Vec2f end;
    Color color;
    f32 thickness;

    Line() : start(0.0f, 0.0f), end(0.0f, 0.0f), color(Color::white()), thickness(1.0f) {}
    Line(const Vec2f& start, const Vec2f& end, const Color& color = Color::white(), f32 thickness = 1.0f)
        : start(start), end(end), color(color), thickness(thickness) {}
    Line(f32 x1, f32 y1, f32 x2, f32 y2, const Color& color = Color::white(), f32 thickness = 1.0f)
        : start(x1, y1), end(x2, y2), color(color), thickness(thickness) {}

    f32 length() const {
        return start.distance(end);
    }

    Vec2f direction() const {
        return (end - start).normalized();
    }
};

// ===== Rectangle =====

struct Rect {
    Vec2f position;  // Top-left corner
    Vec2f size;
    Color color;
    f32 rotation;    // In radians

    Rect() : position(0.0f, 0.0f), size(0.0f, 0.0f), color(Color::white()), rotation(0.0f) {}
    Rect(const Vec2f& pos, const Vec2f& size, const Color& color = Color::white())
        : position(pos), size(size), color(color), rotation(0.0f) {}
    Rect(f32 x, f32 y, f32 w, f32 h, const Color& color = Color::white())
        : position(x, y), size(w, h), color(color), rotation(0.0f) {}

    f32 x() const { return position.x; }
    f32 y() const { return position.y; }
    f32 width() const { return size.x; }
    f32 height() const { return size.y; }

    f32 left() const { return position.x; }
    f32 right() const { return position.x + size.x; }
    f32 top() const { return position.y; }
    f32 bottom() const { return position.y + size.y; }

    Vec2f center() const {
        return position + size * 0.5f;
    }

    bool contains(const Vec2f& point) const {
        return point.x >= left() && point.x <= right() &&
               point.y >= top() && point.y <= bottom();
    }

    bool intersects(const Rect& other) const {
        return !(right() < other.left() || left() > other.right() ||
                 bottom() < other.top() || top() > other.bottom());
    }
};

// ===== Circle =====

struct Circle {
    Vec2f center;
    f32 radius;
    Color color;

    Circle() : center(0.0f, 0.0f), radius(0.0f), color(Color::white()) {}
    Circle(const Vec2f& center, f32 radius, const Color& color = Color::white())
        : center(center), radius(radius), color(color) {}
    Circle(f32 x, f32 y, f32 radius, const Color& color = Color::white())
        : center(x, y), radius(radius), color(color) {}

    bool contains(const Vec2f& point) const {
        return center.distance(point) <= radius;
    }

    bool intersects(const Circle& other) const {
        f32 distSq = center.distanceSquared(other.center);
        f32 radiusSum = radius + other.radius;
        return distSq <= radiusSum * radiusSum;
    }

    bool intersects(const Rect& rect) const {
        // Find closest point on rectangle to circle center
        f32 closestX = math::clamp(center.x, rect.left(), rect.right());
        f32 closestY = math::clamp(center.y, rect.top(), rect.bottom());
        Vec2f closest(closestX, closestY);
        return center.distanceSquared(closest) <= radius * radius;
    }
};

// ===== Ellipse =====

struct Ellipse {
    Vec2f center;
    f32 radiusX;
    f32 radiusY;
    Color color;
    f32 rotation; // In radians

    Ellipse() : center(0.0f, 0.0f), radiusX(0.0f), radiusY(0.0f), color(Color::white()), rotation(0.0f) {}
    Ellipse(const Vec2f& center, f32 rx, f32 ry, const Color& color = Color::white())
        : center(center), radiusX(rx), radiusY(ry), color(color), rotation(0.0f) {}
    Ellipse(f32 x, f32 y, f32 rx, f32 ry, const Color& color = Color::white())
        : center(x, y), radiusX(rx), radiusY(ry), color(color), rotation(0.0f) {}
};

// ===== Triangle =====

struct Triangle {
    Vec2f p1, p2, p3;
    Color color;

    Triangle() : p1(0.0f, 0.0f), p2(0.0f, 0.0f), p3(0.0f, 0.0f), color(Color::white()) {}
    Triangle(const Vec2f& p1, const Vec2f& p2, const Vec2f& p3, const Color& color = Color::white())
        : p1(p1), p2(p2), p3(p3), color(color) {}

    Vec2f center() const {
        return (p1 + p2 + p3) / 3.0f;
    }

    f32 area() const {
        return std::abs((p1.x * (p2.y - p3.y) +
                        p2.x * (p3.y - p1.y) +
                        p3.x * (p1.y - p2.y)) / 2.0f);
    }

    bool contains(const Vec2f& point) const {
        f32 totalArea = area();
        Triangle t1(p1, p2, point);
        Triangle t2(p2, p3, point);
        Triangle t3(p3, p1, point);
        f32 sumArea = t1.area() + t2.area() + t3.area();
        return std::abs(totalArea - sumArea) < 0.001f;
    }
};

// ===== Polygon =====

struct Polygon {
    std::vector<Vec2f> vertices;
    Color color;
    bool filled;

    Polygon() : color(Color::white()), filled(true) {}
    Polygon(const std::vector<Vec2f>& vertices, const Color& color = Color::white(), bool filled = true)
        : vertices(vertices), color(color), filled(filled) {}

    void addVertex(const Vec2f& vertex) {
        vertices.push_back(vertex);
    }

    void addVertex(f32 x, f32 y) {
        vertices.push_back(Vec2f(x, y));
    }

    Vec2f center() const {
        if (vertices.empty()) return Vec2f(0.0f, 0.0f);
        Vec2f sum(0.0f, 0.0f);
        for (const auto& v : vertices) {
            sum += v;
        }
        return sum / static_cast<f32>(vertices.size());
    }

    Rect boundingBox() const {
        if (vertices.empty()) return Rect();

        f32 minX = vertices[0].x, maxX = vertices[0].x;
        f32 minY = vertices[0].y, maxY = vertices[0].y;

        for (size_t i = 1; i < vertices.size(); i++) {
            minX = std::min(minX, vertices[i].x);
            maxX = std::max(maxX, vertices[i].x);
            minY = std::min(minY, vertices[i].y);
            maxY = std::max(maxY, vertices[i].y);
        }

        return Rect(minX, minY, maxX - minX, maxY - minY);
    }
};

// ===== Path =====

struct Path {
    std::vector<Vec2f> points;
    Color color;
    f32 thickness;
    bool closed;

    Path() : color(Color::white()), thickness(1.0f), closed(false) {}
    Path(const Color& color, f32 thickness = 1.0f, bool closed = false)
        : color(color), thickness(thickness), closed(closed) {}

    void moveTo(const Vec2f& point) {
        points.push_back(point);
    }

    void moveTo(f32 x, f32 y) {
        points.push_back(Vec2f(x, y));
    }

    void lineTo(const Vec2f& point) {
        points.push_back(point);
    }

    void lineTo(f32 x, f32 y) {
        points.push_back(Vec2f(x, y));
    }

    void close() {
        closed = true;
    }

    f32 length() const {
        if (points.size() < 2) return 0.0f;
        f32 len = 0.0f;
        for (size_t i = 1; i < points.size(); i++) {
            len += points[i-1].distance(points[i]);
        }
        if (closed && points.size() > 2) {
            len += points.back().distance(points.front());
        }
        return len;
    }
};

} // namespace gfx
} // namespace oracon

#endif // ORACON_GFX_PRIMITIVES_H
