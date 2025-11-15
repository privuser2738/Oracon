#ifndef ORACON_GFX_CANVAS_H
#define ORACON_GFX_CANVAS_H

#include "oracon/gfx/color.h"
#include "oracon/math/vector.h"
#include <vector>
#include <memory>
#include <fstream>
#include <string>

namespace oracon {
namespace gfx {

using math::Vec2i;
using core::u32;

// Canvas represents a 2D pixel buffer that can be drawn to
class Canvas {
public:
    Canvas(u32 width, u32 height)
        : m_width(width)
        , m_height(height)
        , m_pixels(width * height, Color::transparent())
    {}

    // Dimensions
    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }
    Vec2i getSize() const { return Vec2i(m_width, m_height); }

    // Pixel access
    void setPixel(u32 x, u32 y, const Color& color) {
        if (x < m_width && y < m_height) {
            m_pixels[y * m_width + x] = color;
        }
    }

    Color getPixel(u32 x, u32 y) const {
        if (x < m_width && y < m_height) {
            return m_pixels[y * m_width + x];
        }
        return Color::transparent();
    }

    // Raw data access
    const Color* data() const { return m_pixels.data(); }
    Color* data() { return m_pixels.data(); }

    // Get as packed RGBA data
    std::vector<u32> toRGBA32() const {
        std::vector<u32> result(m_pixels.size());
        for (size_t i = 0; i < m_pixels.size(); i++) {
            result[i] = m_pixels[i].toRGBA32();
        }
        return result;
    }

    // Get as packed ARGB data
    std::vector<u32> toARGB32() const {
        std::vector<u32> result(m_pixels.size());
        for (size_t i = 0; i < m_pixels.size(); i++) {
            result[i] = m_pixels[i].toARGB32();
        }
        return result;
    }

    // Canvas operations
    void clear(const Color& color = Color::transparent()) {
        std::fill(m_pixels.begin(), m_pixels.end(), color);
    }

    void fill(const Color& color) {
        std::fill(m_pixels.begin(), m_pixels.end(), color);
    }

    // Copy from another canvas
    void blit(const Canvas& source, u32 srcX, u32 srcY, u32 srcW, u32 srcH,
              u32 dstX, u32 dstY) {
        for (u32 y = 0; y < srcH; y++) {
            for (u32 x = 0; x < srcW; x++) {
                u32 sx = srcX + x;
                u32 sy = srcY + y;
                u32 dx = dstX + x;
                u32 dy = dstY + y;

                if (sx < source.m_width && sy < source.m_height &&
                    dx < m_width && dy < m_height) {
                    setPixel(dx, dy, source.getPixel(sx, sy));
                }
            }
        }
    }

    // Blit entire canvas
    void blit(const Canvas& source, u32 dstX, u32 dstY) {
        blit(source, 0, 0, source.m_width, source.m_height, dstX, dstY);
    }

    // Alpha blending
    void blendPixel(u32 x, u32 y, const Color& color) {
        if (x >= m_width || y >= m_height) return;

        Color dst = getPixel(x, y);
        f32 srcAlpha = color.af();
        f32 dstAlpha = dst.af();

        if (srcAlpha == 0.0f) return;
        if (srcAlpha == 1.0f) {
            setPixel(x, y, color);
            return;
        }

        // Alpha compositing
        f32 outAlpha = srcAlpha + dstAlpha * (1.0f - srcAlpha);
        if (outAlpha == 0.0f) {
            setPixel(x, y, Color::transparent());
            return;
        }

        u8 outR = static_cast<u8>((color.r * srcAlpha + dst.r * dstAlpha * (1.0f - srcAlpha)) / outAlpha);
        u8 outG = static_cast<u8>((color.g * srcAlpha + dst.g * dstAlpha * (1.0f - srcAlpha)) / outAlpha);
        u8 outB = static_cast<u8>((color.b * srcAlpha + dst.b * dstAlpha * (1.0f - srcAlpha)) / outAlpha);
        u8 outA = static_cast<u8>(outAlpha * 255.0f);

        setPixel(x, y, Color(outR, outG, outB, outA));
    }

    // Resize canvas (creates new buffer)
    void resize(u32 newWidth, u32 newHeight) {
        std::vector<Color> newPixels(newWidth * newHeight, Color::transparent());

        // Copy old pixels that fit
        u32 copyWidth = std::min(m_width, newWidth);
        u32 copyHeight = std::min(m_height, newHeight);

        for (u32 y = 0; y < copyHeight; y++) {
            for (u32 x = 0; x < copyWidth; x++) {
                newPixels[y * newWidth + x] = m_pixels[y * m_width + x];
            }
        }

        m_width = newWidth;
        m_height = newHeight;
        m_pixels = std::move(newPixels);
    }

    // Create a sub-canvas view (copy)
    Canvas subCanvas(u32 x, u32 y, u32 w, u32 h) const {
        Canvas result(w, h);
        for (u32 dy = 0; dy < h; dy++) {
            for (u32 dx = 0; dx < w; dx++) {
                u32 sx = x + dx;
                u32 sy = y + dy;
                if (sx < m_width && sy < m_height) {
                    result.setPixel(dx, dy, getPixel(sx, sy));
                }
            }
        }
        return result;
    }

    // Flip operations
    void flipHorizontal() {
        for (u32 y = 0; y < m_height; y++) {
            for (u32 x = 0; x < m_width / 2; x++) {
                u32 oppositeX = m_width - 1 - x;
                Color temp = getPixel(x, y);
                setPixel(x, y, getPixel(oppositeX, y));
                setPixel(oppositeX, y, temp);
            }
        }
    }

    void flipVertical() {
        for (u32 y = 0; y < m_height / 2; y++) {
            for (u32 x = 0; x < m_width; x++) {
                u32 oppositeY = m_height - 1 - y;
                Color temp = getPixel(x, y);
                setPixel(x, y, getPixel(x, oppositeY));
                setPixel(x, oppositeY, temp);
            }
        }
    }

    // Save as PPM file
    void savePPM(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        if (!file) return;

        file << "P6\n" << m_width << " " << m_height << "\n255\n";

        for (u32 y = 0; y < m_height; y++) {
            for (u32 x = 0; x < m_width; x++) {
                Color c = getPixel(x, y);
                file.put(static_cast<char>(c.r));
                file.put(static_cast<char>(c.g));
                file.put(static_cast<char>(c.b));
            }
        }
    }

private:
    u32 m_width;
    u32 m_height;
    std::vector<Color> m_pixels;
};

} // namespace gfx
} // namespace oracon

#endif // ORACON_GFX_CANVAS_H
