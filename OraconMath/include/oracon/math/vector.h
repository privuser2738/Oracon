#ifndef ORACON_MATH_VECTOR_H
#define ORACON_MATH_VECTOR_H

#include "oracon/math/functions.h"
#include <cmath>
#include <ostream>

namespace oracon {
namespace math {

using core::i32;

// ===== Vector2 =====

template<typename T>
struct Vector2 {
    T x, y;

    // Constructors
    Vector2() : x(T(0)), y(T(0)) {}
    Vector2(T x, T y) : x(x), y(y) {}
    explicit Vector2(T scalar) : x(scalar), y(scalar) {}

    // Arithmetic operators
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 operator*(T scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    Vector2 operator/(T scalar) const {
        return Vector2(x / scalar, y / scalar);
    }

    Vector2 operator-() const {
        return Vector2(-x, -y);
    }

    // Compound assignment operators
    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2& operator/=(T scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Comparison operators
    bool operator==(const Vector2& other) const {
        return floatEqual(x, other.x) && floatEqual(y, other.y);
    }

    bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }

    // Vector operations
    T dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    T cross(const Vector2& other) const {
        return x * other.y - y * other.x;
    }

    T length() const {
        return std::sqrt(x * x + y * y);
    }

    T lengthSquared() const {
        return x * x + y * y;
    }

    Vector2 normalized() const {
        T len = length();
        return len > T(0) ? *this / len : Vector2();
    }

    void normalize() {
        T len = length();
        if (len > T(0)) {
            x /= len;
            y /= len;
        }
    }

    T distance(const Vector2& other) const {
        return (*this - other).length();
    }

    T distanceSquared(const Vector2& other) const {
        return (*this - other).lengthSquared();
    }

    Vector2 lerp(const Vector2& other, T t) const {
        return Vector2(
            math::lerp(x, other.x, t),
            math::lerp(y, other.y, t)
        );
    }

    // Static factory methods
    static Vector2 zero() { return Vector2(T(0), T(0)); }
    static Vector2 one() { return Vector2(T(1), T(1)); }
    static Vector2 unitX() { return Vector2(T(1), T(0)); }
    static Vector2 unitY() { return Vector2(T(0), T(1)); }
};

// Scalar * Vector
template<typename T>
inline Vector2<T> operator*(T scalar, const Vector2<T>& vec) {
    return vec * scalar;
}

// Stream output
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Vector2<T>& vec) {
    return os << "(" << vec.x << ", " << vec.y << ")";
}

// Common type aliases
using Vec2f = Vector2<f32>;
using Vec2d = Vector2<f64>;
using Vec2i = Vector2<i32>;

// ===== Vector3 =====

template<typename T>
struct Vector3 {
    T x, y, z;

    // Constructors
    Vector3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
    explicit Vector3(T scalar) : x(scalar), y(scalar), z(scalar) {}
    Vector3(const Vector2<T>& xy, T z) : x(xy.x), y(xy.y), z(z) {}

    // Arithmetic operators
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(T scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3 operator/(T scalar) const {
        return Vector3(x / scalar, y / scalar, z / scalar);
    }

    Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }

    // Compound assignment operators
    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vector3& operator/=(T scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // Comparison operators
    bool operator==(const Vector3& other) const {
        return floatEqual(x, other.x) && floatEqual(y, other.y) && floatEqual(z, other.z);
    }

    bool operator!=(const Vector3& other) const {
        return !(*this == other);
    }

    // Vector operations
    T dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    T length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    T lengthSquared() const {
        return x * x + y * y + z * z;
    }

    Vector3 normalized() const {
        T len = length();
        return len > T(0) ? *this / len : Vector3();
    }

    void normalize() {
        T len = length();
        if (len > T(0)) {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    T distance(const Vector3& other) const {
        return (*this - other).length();
    }

    T distanceSquared(const Vector3& other) const {
        return (*this - other).lengthSquared();
    }

    Vector3 lerp(const Vector3& other, T t) const {
        return Vector3(
            math::lerp(x, other.x, t),
            math::lerp(y, other.y, t),
            math::lerp(z, other.z, t)
        );
    }

    // Swizzle accessors
    Vector2<T> xy() const { return Vector2<T>(x, y); }
    Vector2<T> xz() const { return Vector2<T>(x, z); }
    Vector2<T> yz() const { return Vector2<T>(y, z); }

    // Static factory methods
    static Vector3 zero() { return Vector3(T(0), T(0), T(0)); }
    static Vector3 one() { return Vector3(T(1), T(1), T(1)); }
    static Vector3 unitX() { return Vector3(T(1), T(0), T(0)); }
    static Vector3 unitY() { return Vector3(T(0), T(1), T(0)); }
    static Vector3 unitZ() { return Vector3(T(0), T(0), T(1)); }
    static Vector3 up() { return Vector3(T(0), T(1), T(0)); }
    static Vector3 down() { return Vector3(T(0), T(-1), T(0)); }
    static Vector3 left() { return Vector3(T(-1), T(0), T(0)); }
    static Vector3 right() { return Vector3(T(1), T(0), T(0)); }
    static Vector3 forward() { return Vector3(T(0), T(0), T(-1)); }
    static Vector3 back() { return Vector3(T(0), T(0), T(1)); }
};

// Scalar * Vector
template<typename T>
inline Vector3<T> operator*(T scalar, const Vector3<T>& vec) {
    return vec * scalar;
}

// Stream output
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Vector3<T>& vec) {
    return os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
}

// Common type aliases
using Vec3f = Vector3<f32>;
using Vec3d = Vector3<f64>;
using Vec3i = Vector3<i32>;

// ===== Vector4 =====

template<typename T>
struct Vector4 {
    T x, y, z, w;

    // Constructors
    Vector4() : x(T(0)), y(T(0)), z(T(0)), w(T(0)) {}
    Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    explicit Vector4(T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
    Vector4(const Vector3<T>& xyz, T w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
    Vector4(const Vector2<T>& xy, const Vector2<T>& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}

    // Arithmetic operators
    Vector4 operator+(const Vector4& other) const {
        return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    Vector4 operator-(const Vector4& other) const {
        return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    Vector4 operator*(T scalar) const {
        return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    Vector4 operator/(T scalar) const {
        return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
    }

    Vector4 operator-() const {
        return Vector4(-x, -y, -z, -w);
    }

    // Compound assignment operators
    Vector4& operator+=(const Vector4& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vector4& operator-=(const Vector4& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    Vector4& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    Vector4& operator/=(T scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    // Comparison operators
    bool operator==(const Vector4& other) const {
        return floatEqual(x, other.x) && floatEqual(y, other.y) &&
               floatEqual(z, other.z) && floatEqual(w, other.w);
    }

    bool operator!=(const Vector4& other) const {
        return !(*this == other);
    }

    // Vector operations
    T dot(const Vector4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    T length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    T lengthSquared() const {
        return x * x + y * y + z * z + w * w;
    }

    Vector4 normalized() const {
        T len = length();
        return len > T(0) ? *this / len : Vector4();
    }

    void normalize() {
        T len = length();
        if (len > T(0)) {
            x /= len;
            y /= len;
            z /= len;
            w /= len;
        }
    }

    Vector4 lerp(const Vector4& other, T t) const {
        return Vector4(
            math::lerp(x, other.x, t),
            math::lerp(y, other.y, t),
            math::lerp(z, other.z, t),
            math::lerp(w, other.w, t)
        );
    }

    // Swizzle accessors
    Vector2<T> xy() const { return Vector2<T>(x, y); }
    Vector2<T> zw() const { return Vector2<T>(z, w); }
    Vector3<T> xyz() const { return Vector3<T>(x, y, z); }

    // Static factory methods
    static Vector4 zero() { return Vector4(T(0), T(0), T(0), T(0)); }
    static Vector4 one() { return Vector4(T(1), T(1), T(1), T(1)); }
};

// Scalar * Vector
template<typename T>
inline Vector4<T> operator*(T scalar, const Vector4<T>& vec) {
    return vec * scalar;
}

// Stream output
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Vector4<T>& vec) {
    return os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
}

// Common type aliases
using Vec4f = Vector4<f32>;
using Vec4d = Vector4<f64>;
using Vec4i = Vector4<i32>;

} // namespace math
} // namespace oracon

#endif // ORACON_MATH_VECTOR_H
