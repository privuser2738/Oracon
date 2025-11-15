#include "oracon/gfx/renderer.h"
#include <algorithm>
#include <cmath>

namespace oracon {
namespace gfx {

using core::i32;

// ===== Point =====

void Renderer::drawPoint(const Point& point) {
    if (!m_canvas) return;

    i32 x = static_cast<i32>(point.position.x);
    i32 y = static_cast<i32>(point.position.y);

    if (x >= 0 && x < static_cast<i32>(m_canvas->getWidth()) &&
        y >= 0 && y < static_cast<i32>(m_canvas->getHeight())) {
        setPixel(x, y, point.color);
    }
}

// ===== Line (Bresenham's algorithm) =====

void Renderer::drawLineBresenham(i32 x0, i32 y0, i32 x1, i32 y1, const Color& color) {
    i32 dx = std::abs(x1 - x0);
    i32 dy = std::abs(y1 - y0);
    i32 sx = x0 < x1 ? 1 : -1;
    i32 sy = y0 < y1 ? 1 : -1;
    i32 err = dx - dy;

    while (true) {
        if (x0 >= 0 && x0 < static_cast<i32>(m_canvas->getWidth()) &&
            y0 >= 0 && y0 < static_cast<i32>(m_canvas->getHeight())) {
            setPixel(x0, y0, color);
        }

        if (x0 == x1 && y0 == y1) break;

        i32 e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Renderer::drawLine(const Line& line) {
    if (!m_canvas) return;

    i32 x0 = static_cast<i32>(line.start.x);
    i32 y0 = static_cast<i32>(line.start.y);
    i32 x1 = static_cast<i32>(line.end.x);
    i32 y1 = static_cast<i32>(line.end.y);

    // TODO: Support thickness > 1
    drawLineBresenham(x0, y0, x1, y1, line.color);
}

// ===== Rectangle =====

void Renderer::drawRect(const Rect& rect, bool filled) {
    if (!m_canvas) return;

    i32 x = static_cast<i32>(rect.x());
    i32 y = static_cast<i32>(rect.y());
    i32 w = static_cast<i32>(rect.width());
    i32 h = static_cast<i32>(rect.height());

    if (filled) {
        // Fill rectangle
        i32 x1 = std::max(0, x);
        i32 y1 = std::max(0, y);
        i32 x2 = std::min(static_cast<i32>(m_canvas->getWidth()), x + w);
        i32 y2 = std::min(static_cast<i32>(m_canvas->getHeight()), y + h);

        for (i32 py = y1; py < y2; py++) {
            for (i32 px = x1; px < x2; px++) {
                setPixel(px, py, rect.color);
            }
        }
    } else {
        // Draw outline
        drawLineBresenham(x, y, x + w - 1, y, rect.color);           // Top
        drawLineBresenham(x + w - 1, y, x + w - 1, y + h - 1, rect.color); // Right
        drawLineBresenham(x + w - 1, y + h - 1, x, y + h - 1, rect.color); // Bottom
        drawLineBresenham(x, y + h - 1, x, y, rect.color);           // Left
    }
}

// ===== Circle (Bresenham's algorithm) =====

void Renderer::drawCircleBresenham(i32 xc, i32 yc, i32 radius, const Color& color) {
    i32 x = 0;
    i32 y = radius;
    i32 d = 3 - 2 * radius;

    auto drawCirclePoints = [&](i32 cx, i32 cy, i32 px, i32 py) {
        auto plot = [&](i32 x, i32 y) {
            if (x >= 0 && x < static_cast<i32>(m_canvas->getWidth()) &&
                y >= 0 && y < static_cast<i32>(m_canvas->getHeight())) {
                setPixel(x, y, color);
            }
        };

        plot(cx + px, cy + py);
        plot(cx - px, cy + py);
        plot(cx + px, cy - py);
        plot(cx - px, cy - py);
        plot(cx + py, cy + px);
        plot(cx - py, cy + px);
        plot(cx + py, cy - px);
        plot(cx - py, cy - px);
    };

    drawCirclePoints(xc, yc, x, y);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        drawCirclePoints(xc, yc, x, y);
    }
}

void Renderer::fillCircleScanline(i32 xc, i32 yc, i32 radius, const Color& color) {
    for (i32 y = -radius; y <= radius; y++) {
        i32 py = yc + y;
        if (py < 0 || py >= static_cast<i32>(m_canvas->getHeight())) continue;

        i32 dx = static_cast<i32>(std::sqrt(radius * radius - y * y));
        i32 x1 = std::max(0, xc - dx);
        i32 x2 = std::min(static_cast<i32>(m_canvas->getWidth()) - 1, xc + dx);

        for (i32 x = x1; x <= x2; x++) {
            setPixel(x, py, color);
        }
    }
}

void Renderer::drawCircle(const Circle& circle, bool filled) {
    if (!m_canvas) return;

    i32 xc = static_cast<i32>(circle.center.x);
    i32 yc = static_cast<i32>(circle.center.y);
    i32 radius = static_cast<i32>(circle.radius);

    if (filled) {
        fillCircleScanline(xc, yc, radius, circle.color);
    } else {
        drawCircleBresenham(xc, yc, radius, circle.color);
    }
}

// ===== Ellipse =====

void Renderer::drawEllipse(const Ellipse& ellipse, bool filled) {
    if (!m_canvas) return;

    i32 xc = static_cast<i32>(ellipse.center.x);
    i32 yc = static_cast<i32>(ellipse.center.y);
    i32 rx = static_cast<i32>(ellipse.radiusX);
    i32 ry = static_cast<i32>(ellipse.radiusY);

    if (filled) {
        // Fill ellipse using scanline
        for (i32 y = -ry; y <= ry; y++) {
            i32 py = yc + y;
            if (py < 0 || py >= static_cast<i32>(m_canvas->getHeight())) continue;

            f32 fyNorm = static_cast<f32>(y) / ry;
            i32 dx = static_cast<i32>(rx * std::sqrt(1.0f - fyNorm * fyNorm));

            i32 x1 = std::max(0, xc - dx);
            i32 x2 = std::min(static_cast<i32>(m_canvas->getWidth()) - 1, xc + dx);

            for (i32 x = x1; x <= x2; x++) {
                setPixel(x, py, ellipse.color);
            }
        }
    } else {
        // Draw ellipse outline (Bresenham-like algorithm)
        i32 x = 0;
        i32 y = ry;

        i32 rx2 = rx * rx;
        i32 ry2 = ry * ry;

        i32 dx = 0;
        i32 dy = 2 * rx2 * y;

        i32 p1 = ry2 - rx2 * ry + (rx2 / 4);

        auto plotEllipsePoints = [&](i32 px, i32 py) {
            auto plot = [&](i32 x, i32 y) {
                if (x >= 0 && x < static_cast<i32>(m_canvas->getWidth()) &&
                    y >= 0 && y < static_cast<i32>(m_canvas->getHeight())) {
                    setPixel(x, y, ellipse.color);
                }
            };
            plot(xc + px, yc + py);
            plot(xc - px, yc + py);
            plot(xc + px, yc - py);
            plot(xc - px, yc - py);
        };

        while (dx < dy) {
            plotEllipsePoints(x, y);
            x++;
            dx += 2 * ry2;
            if (p1 < 0) {
                p1 += dx + ry2;
            } else {
                y--;
                dy -= 2 * rx2;
                p1 += dx - dy + ry2;
            }
        }

        i32 p2 = ry2 * (x * x) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
        while (y >= 0) {
            plotEllipsePoints(x, y);
            y--;
            dy -= 2 * rx2;
            if (p2 > 0) {
                p2 += -dy + rx2;
            } else {
                x++;
                dx += 2 * ry2;
                p2 += dx - dy + rx2;
            }
        }
    }
}

// ===== Triangle =====

void Renderer::fillTriangleScanline(const Triangle& triangle) {
    // Sort vertices by y-coordinate
    Vec2f v0 = triangle.p1;
    Vec2f v1 = triangle.p2;
    Vec2f v2 = triangle.p3;

    if (v0.y > v1.y) std::swap(v0, v1);
    if (v0.y > v2.y) std::swap(v0, v2);
    if (v1.y > v2.y) std::swap(v1, v2);

    auto drawScanline = [&](f32 y, f32 x1, f32 x2) {
        if (y < 0 || y >= m_canvas->getHeight()) return;

        i32 ix1 = static_cast<i32>(x1);
        i32 ix2 = static_cast<i32>(x2);
        if (ix1 > ix2) std::swap(ix1, ix2);

        ix1 = std::max(0, ix1);
        ix2 = std::min(static_cast<i32>(m_canvas->getWidth()) - 1, ix2);

        for (i32 x = ix1; x <= ix2; x++) {
            setPixel(x, static_cast<i32>(y), triangle.color);
        }
    };

    // Flat-bottom triangle
    if (v1.y != v0.y) {
        f32 invSlope1 = (v1.x - v0.x) / (v1.y - v0.y);
        f32 invSlope2 = (v2.x - v0.x) / (v2.y - v0.y);

        f32 curx1 = v0.x;
        f32 curx2 = v0.x;

        for (i32 scanY = static_cast<i32>(v0.y); scanY <= static_cast<i32>(v1.y); scanY++) {
            drawScanline(scanY, curx1, curx2);
            curx1 += invSlope1;
            curx2 += invSlope2;
        }
    }

    // Flat-top triangle
    if (v2.y != v1.y) {
        f32 invSlope1 = (v2.x - v1.x) / (v2.y - v1.y);
        f32 invSlope2 = (v2.x - v0.x) / (v2.y - v0.y);

        f32 curx1 = v2.x;
        f32 curx2 = v2.x;

        for (i32 scanY = static_cast<i32>(v2.y); scanY > static_cast<i32>(v1.y); scanY--) {
            drawScanline(scanY, curx1, curx2);
            curx1 -= invSlope1;
            curx2 -= invSlope2;
        }
    }
}

void Renderer::drawTriangle(const Triangle& triangle, bool filled) {
    if (!m_canvas) return;

    if (filled) {
        fillTriangleScanline(triangle);
    } else {
        drawLineBresenham(
            static_cast<i32>(triangle.p1.x), static_cast<i32>(triangle.p1.y),
            static_cast<i32>(triangle.p2.x), static_cast<i32>(triangle.p2.y),
            triangle.color
        );
        drawLineBresenham(
            static_cast<i32>(triangle.p2.x), static_cast<i32>(triangle.p2.y),
            static_cast<i32>(triangle.p3.x), static_cast<i32>(triangle.p3.y),
            triangle.color
        );
        drawLineBresenham(
            static_cast<i32>(triangle.p3.x), static_cast<i32>(triangle.p3.y),
            static_cast<i32>(triangle.p1.x), static_cast<i32>(triangle.p1.y),
            triangle.color
        );
    }
}

// ===== Polygon =====

void Renderer::drawPolygon(const Polygon& polygon) {
    if (!m_canvas || polygon.vertices.size() < 2) return;

    if (polygon.filled) {
        // Triangulate and fill (simple ear clipping for convex polygons)
        for (size_t i = 1; i < polygon.vertices.size() - 1; i++) {
            Triangle tri(polygon.vertices[0], polygon.vertices[i], polygon.vertices[i + 1], polygon.color);
            fillTriangleScanline(tri);
        }
    } else {
        // Draw outline
        for (size_t i = 0; i < polygon.vertices.size() - 1; i++) {
            drawLineBresenham(
                static_cast<i32>(polygon.vertices[i].x),
                static_cast<i32>(polygon.vertices[i].y),
                static_cast<i32>(polygon.vertices[i + 1].x),
                static_cast<i32>(polygon.vertices[i + 1].y),
                polygon.color
            );
        }

        // Close the polygon
        if (polygon.vertices.size() > 2) {
            drawLineBresenham(
                static_cast<i32>(polygon.vertices.back().x),
                static_cast<i32>(polygon.vertices.back().y),
                static_cast<i32>(polygon.vertices[0].x),
                static_cast<i32>(polygon.vertices[0].y),
                polygon.color
            );
        }
    }
}

// ===== Path =====

void Renderer::drawPath(const Path& path) {
    if (!m_canvas || path.points.size() < 2) return;

    for (size_t i = 1; i < path.points.size(); i++) {
        drawLineBresenham(
            static_cast<i32>(path.points[i-1].x),
            static_cast<i32>(path.points[i-1].y),
            static_cast<i32>(path.points[i].x),
            static_cast<i32>(path.points[i].y),
            path.color
        );
    }

    if (path.closed && path.points.size() > 2) {
        drawLineBresenham(
            static_cast<i32>(path.points.back().x),
            static_cast<i32>(path.points.back().y),
            static_cast<i32>(path.points[0].x),
            static_cast<i32>(path.points[0].y),
            path.color
        );
    }
}

} // namespace gfx
} // namespace oracon
