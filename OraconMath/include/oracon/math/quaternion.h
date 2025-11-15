#ifndef ORACON_MATH_QUATERNION_H
#define ORACON_MATH_QUATERNION_H

#include "oracon/math/vector.h"
#include "oracon/math/matrix.h"

namespace oracon {
namespace math {

template<typename T>
struct Quaternion {
    T w, x, y, z;

    // Constructors
    Quaternion() : w(T(1)), x(T(0)), y(T(0)), z(T(0)) {}
    Quaternion(T w, T x, T y, T z) : w(w), x(x), y(y), z(z) {}
    Quaternion(T w, const Vector3<T>& v) : w(w), x(v.x), y(v.y), z(v.z) {}

    // Arithmetic operators
    Quaternion operator+(const Quaternion& other) const {
        return Quaternion(w + other.w, x + other.x, y + other.y, z + other.z);
    }

    Quaternion operator-(const Quaternion& other) const {
        return Quaternion(w - other.w, x - other.x, y - other.y, z - other.z);
    }

    Quaternion operator*(const Quaternion& other) const {
        return Quaternion(
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w
        );
    }

    Quaternion operator*(T scalar) const {
        return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar);
    }

    Quaternion operator/(T scalar) const {
        return Quaternion(w / scalar, x / scalar, y / scalar, z / scalar);
    }

    Quaternion operator-() const {
        return Quaternion(-w, -x, -y, -z);
    }

    // Compound assignment operators
    Quaternion& operator+=(const Quaternion& other) {
        w += other.w;
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Quaternion& operator-=(const Quaternion& other) {
        w -= other.w;
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Quaternion& operator*=(const Quaternion& other) {
        *this = *this * other;
        return *this;
    }

    Quaternion& operator*=(T scalar) {
        w *= scalar;
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Quaternion& operator/=(T scalar) {
        w /= scalar;
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // Comparison operators
    bool operator==(const Quaternion& other) const {
        return floatEqual(w, other.w) && floatEqual(x, other.x) &&
               floatEqual(y, other.y) && floatEqual(z, other.z);
    }

    bool operator!=(const Quaternion& other) const {
        return !(*this == other);
    }

    // Quaternion operations
    T dot(const Quaternion& other) const {
        return w * other.w + x * other.x + y * other.y + z * other.z;
    }

    T length() const {
        return std::sqrt(w * w + x * x + y * y + z * z);
    }

    T lengthSquared() const {
        return w * w + x * x + y * y + z * z;
    }

    Quaternion normalized() const {
        T len = length();
        return len > T(0) ? *this / len : Quaternion();
    }

    void normalize() {
        T len = length();
        if (len > T(0)) {
            w /= len;
            x /= len;
            y /= len;
            z /= len;
        }
    }

    Quaternion conjugate() const {
        return Quaternion(w, -x, -y, -z);
    }

    Quaternion inverse() const {
        T lenSq = lengthSquared();
        if (lenSq > T(0)) {
            return conjugate() / lenSq;
        }
        return Quaternion();
    }

    // Rotate a vector by this quaternion
    Vector3<T> rotate(const Vector3<T>& v) const {
        Quaternion p(T(0), v);
        Quaternion result = (*this) * p * conjugate();
        return Vector3<T>(result.x, result.y, result.z);
    }

    // Convert to rotation matrix
    Matrix4<T> toMatrix() const {
        T xx = x * x;
        T yy = y * y;
        T zz = z * z;
        T xy = x * y;
        T xz = x * z;
        T yz = y * z;
        T wx = w * x;
        T wy = w * y;
        T wz = w * z;

        return Matrix4<T>(
            T(1) - T(2) * (yy + zz), T(2) * (xy - wz),       T(2) * (xz + wy),       T(0),
            T(2) * (xy + wz),         T(1) - T(2) * (xx + zz), T(2) * (yz - wx),       T(0),
            T(2) * (xz - wy),         T(2) * (yz + wx),       T(1) - T(2) * (xx + yy), T(0),
            T(0),                     T(0),                   T(0),                   T(1)
        );
    }

    // Get euler angles (in radians)
    Vector3<T> toEuler() const {
        Vector3<T> euler;

        // Roll (x-axis rotation)
        T sinr_cosp = T(2) * (w * x + y * z);
        T cosr_cosp = T(1) - T(2) * (x * x + y * y);
        euler.x = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        T sinp = T(2) * (w * y - z * x);
        if (std::abs(sinp) >= T(1))
            euler.y = std::copysign(Constants<T>::HALF_PI, sinp);
        else
            euler.y = std::asin(sinp);

        // Yaw (z-axis rotation)
        T siny_cosp = T(2) * (w * z + x * y);
        T cosy_cosp = T(1) - T(2) * (y * y + z * z);
        euler.z = std::atan2(siny_cosp, cosy_cosp);

        return euler;
    }

    // Spherical linear interpolation
    Quaternion slerp(const Quaternion& other, T t) const {
        T cosHalfTheta = dot(other);

        // If the quaternions are very close, use linear interpolation
        if (std::abs(cosHalfTheta) >= T(1)) {
            return *this;
        }

        // Choose the shortest path
        Quaternion end = other;
        if (cosHalfTheta < T(0)) {
            end = -other;
            cosHalfTheta = -cosHalfTheta;
        }

        T halfTheta = std::acos(cosHalfTheta);
        T sinHalfTheta = std::sqrt(T(1) - cosHalfTheta * cosHalfTheta);

        // If theta is close to 180 degrees, result is not fully defined
        if (std::abs(sinHalfTheta) < T(0.001)) {
            return Quaternion(
                w * T(0.5) + end.w * T(0.5),
                x * T(0.5) + end.x * T(0.5),
                y * T(0.5) + end.y * T(0.5),
                z * T(0.5) + end.z * T(0.5)
            );
        }

        T ratioA = std::sin((T(1) - t) * halfTheta) / sinHalfTheta;
        T ratioB = std::sin(t * halfTheta) / sinHalfTheta;

        return Quaternion(
            w * ratioA + end.w * ratioB,
            x * ratioA + end.x * ratioB,
            y * ratioA + end.y * ratioB,
            z * ratioA + end.z * ratioB
        );
    }

    // Static factory methods
    static Quaternion identity() {
        return Quaternion(T(1), T(0), T(0), T(0));
    }

    static Quaternion fromAxisAngle(const Vector3<T>& axis, T angle) {
        T halfAngle = angle * T(0.5);
        T s = std::sin(halfAngle);
        Vector3<T> a = axis.normalized();
        return Quaternion(std::cos(halfAngle), a.x * s, a.y * s, a.z * s);
    }

    static Quaternion fromEuler(T pitch, T yaw, T roll) {
        T cy = std::cos(yaw * T(0.5));
        T sy = std::sin(yaw * T(0.5));
        T cp = std::cos(pitch * T(0.5));
        T sp = std::sin(pitch * T(0.5));
        T cr = std::cos(roll * T(0.5));
        T sr = std::sin(roll * T(0.5));

        return Quaternion(
            cr * cp * cy + sr * sp * sy,
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy
        );
    }

    static Quaternion fromEuler(const Vector3<T>& euler) {
        return fromEuler(euler.x, euler.y, euler.z);
    }

    static Quaternion lookRotation(const Vector3<T>& forward, const Vector3<T>& up) {
        Vector3<T> f = forward.normalized();
        Vector3<T> u = up.normalized();
        Vector3<T> r = u.cross(f);
        u = f.cross(r);

        T m00 = r.x, m01 = r.y, m02 = r.z;
        T m10 = u.x, m11 = u.y, m12 = u.z;
        T m20 = f.x, m21 = f.y, m22 = f.z;

        T trace = m00 + m11 + m22;
        Quaternion q;

        if (trace > T(0)) {
            T s = T(0.5) / std::sqrt(trace + T(1));
            q.w = T(0.25) / s;
            q.x = (m21 - m12) * s;
            q.y = (m02 - m20) * s;
            q.z = (m10 - m01) * s;
        } else if (m00 > m11 && m00 > m22) {
            T s = T(2) * std::sqrt(T(1) + m00 - m11 - m22);
            q.w = (m21 - m12) / s;
            q.x = T(0.25) * s;
            q.y = (m01 + m10) / s;
            q.z = (m02 + m20) / s;
        } else if (m11 > m22) {
            T s = T(2) * std::sqrt(T(1) + m11 - m00 - m22);
            q.w = (m02 - m20) / s;
            q.x = (m01 + m10) / s;
            q.y = T(0.25) * s;
            q.z = (m12 + m21) / s;
        } else {
            T s = T(2) * std::sqrt(T(1) + m22 - m00 - m11);
            q.w = (m10 - m01) / s;
            q.x = (m02 + m20) / s;
            q.y = (m12 + m21) / s;
            q.z = T(0.25) * s;
        }

        return q.normalized();
    }
};

// Scalar * Quaternion
template<typename T>
inline Quaternion<T> operator*(T scalar, const Quaternion<T>& q) {
    return q * scalar;
}

// Stream output
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Quaternion<T>& q) {
    return os << "(" << q.w << " + " << q.x << "i + " << q.y << "j + " << q.z << "k)";
}

// Common type aliases
using Quatf = Quaternion<f32>;
using Quatd = Quaternion<f64>;

} // namespace math
} // namespace oracon

#endif // ORACON_MATH_QUATERNION_H
