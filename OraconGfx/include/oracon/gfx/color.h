#ifndef ORACON_GFX_COLOR_H
#define ORACON_GFX_COLOR_H

#include "oracon/core/types.h"
#include "oracon/math/functions.h"
#include <ostream>

namespace oracon {
namespace gfx {

using core::u8;
using core::u32;
using core::f32;

// RGBA color with 8-bit channels
struct Color {
    u8 r, g, b, a;

    // Constructors
    Color() : r(0), g(0), b(0), a(255) {}
    Color(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a) {}

    // Create from float values (0.0 - 1.0)
    static Color fromFloat(f32 r, f32 g, f32 b, f32 a = 1.0f) {
        return Color(
            static_cast<u8>(math::clamp(r, 0.0f, 1.0f) * 255.0f),
            static_cast<u8>(math::clamp(g, 0.0f, 1.0f) * 255.0f),
            static_cast<u8>(math::clamp(b, 0.0f, 1.0f) * 255.0f),
            static_cast<u8>(math::clamp(a, 0.0f, 1.0f) * 255.0f)
        );
    }

    // Create from packed 32-bit RGBA
    static Color fromRGBA32(u32 rgba) {
        return Color(
            static_cast<u8>((rgba >> 24) & 0xFF),
            static_cast<u8>((rgba >> 16) & 0xFF),
            static_cast<u8>((rgba >> 8) & 0xFF),
            static_cast<u8>(rgba & 0xFF)
        );
    }

    // Create from packed 32-bit ARGB
    static Color fromARGB32(u32 argb) {
        return Color(
            static_cast<u8>((argb >> 16) & 0xFF),
            static_cast<u8>((argb >> 8) & 0xFF),
            static_cast<u8>(argb & 0xFF),
            static_cast<u8>((argb >> 24) & 0xFF)
        );
    }

    // Create from HSV (hue: 0-360, saturation: 0-1, value: 0-1)
    static Color fromHSV(f32 h, f32 s, f32 v, f32 a = 1.0f) {
        f32 c = v * s;
        f32 x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
        f32 m = v - c;

        f32 rp, gp, bp;
        if (h >= 0.0f && h < 60.0f) {
            rp = c; gp = x; bp = 0.0f;
        } else if (h >= 60.0f && h < 120.0f) {
            rp = x; gp = c; bp = 0.0f;
        } else if (h >= 120.0f && h < 180.0f) {
            rp = 0.0f; gp = c; bp = x;
        } else if (h >= 180.0f && h < 240.0f) {
            rp = 0.0f; gp = x; bp = c;
        } else if (h >= 240.0f && h < 300.0f) {
            rp = x; gp = 0.0f; bp = c;
        } else {
            rp = c; gp = 0.0f; bp = x;
        }

        return fromFloat(rp + m, gp + m, bp + m, a);
    }

    // Convert to packed formats
    u32 toRGBA32() const {
        return (static_cast<u32>(r) << 24) |
               (static_cast<u32>(g) << 16) |
               (static_cast<u32>(b) << 8) |
               static_cast<u32>(a);
    }

    u32 toARGB32() const {
        return (static_cast<u32>(a) << 24) |
               (static_cast<u32>(r) << 16) |
               (static_cast<u32>(g) << 8) |
               static_cast<u32>(b);
    }

    // Get float components (0.0 - 1.0)
    f32 rf() const { return r / 255.0f; }
    f32 gf() const { return g / 255.0f; }
    f32 bf() const { return b / 255.0f; }
    f32 af() const { return a / 255.0f; }

    // Color operations
    Color blend(const Color& other, f32 t) const {
        return Color(
            static_cast<u8>(math::lerp(static_cast<f32>(r), static_cast<f32>(other.r), t)),
            static_cast<u8>(math::lerp(static_cast<f32>(g), static_cast<f32>(other.g), t)),
            static_cast<u8>(math::lerp(static_cast<f32>(b), static_cast<f32>(other.b), t)),
            static_cast<u8>(math::lerp(static_cast<f32>(a), static_cast<f32>(other.a), t))
        );
    }

    Color withAlpha(u8 newAlpha) const {
        return Color(r, g, b, newAlpha);
    }

    Color multiply(f32 factor) const {
        return Color(
            static_cast<u8>(math::clamp(r * factor, 0.0f, 255.0f)),
            static_cast<u8>(math::clamp(g * factor, 0.0f, 255.0f)),
            static_cast<u8>(math::clamp(b * factor, 0.0f, 255.0f)),
            a
        );
    }

    // Comparison
    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    bool operator!=(const Color& other) const {
        return !(*this == other);
    }

    // Predefined colors
    static Color transparent() { return Color(0, 0, 0, 0); }
    static Color black() { return Color(0, 0, 0, 255); }
    static Color white() { return Color(255, 255, 255, 255); }
    static Color red() { return Color(255, 0, 0, 255); }
    static Color green() { return Color(0, 255, 0, 255); }
    static Color blue() { return Color(0, 0, 255, 255); }
    static Color yellow() { return Color(255, 255, 0, 255); }
    static Color cyan() { return Color(0, 255, 255, 255); }
    static Color magenta() { return Color(255, 0, 255, 255); }
    static Color gray() { return Color(128, 128, 128, 255); }
    static Color darkGray() { return Color(64, 64, 64, 255); }
    static Color lightGray() { return Color(192, 192, 192, 255); }
    static Color orange() { return Color(255, 165, 0, 255); }
    static Color purple() { return Color(128, 0, 128, 255); }
    static Color brown() { return Color(165, 42, 42, 255); }
    static Color pink() { return Color(255, 192, 203, 255); }
};

// Stream output
inline std::ostream& operator<<(std::ostream& os, const Color& c) {
    return os << "Color(" << static_cast<int>(c.r) << ", "
              << static_cast<int>(c.g) << ", "
              << static_cast<int>(c.b) << ", "
              << static_cast<int>(c.a) << ")";
}

} // namespace gfx
} // namespace oracon

#endif // ORACON_GFX_COLOR_H
