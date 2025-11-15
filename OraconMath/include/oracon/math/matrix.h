#ifndef ORACON_MATH_MATRIX_H
#define ORACON_MATH_MATRIX_H

#include "oracon/math/vector.h"
#include <cstring>

namespace oracon {
namespace math {

// ===== Matrix4x4 =====
// Column-major order for OpenGL compatibility

template<typename T>
struct Matrix4 {
    T m[16]; // Column-major: m[column * 4 + row]

    // Constructors
    Matrix4() {
        setIdentity();
    }

    explicit Matrix4(T diagonal) {
        std::memset(m, 0, sizeof(m));
        m[0] = m[5] = m[10] = m[15] = diagonal;
    }

    Matrix4(T m00, T m01, T m02, T m03,
            T m10, T m11, T m12, T m13,
            T m20, T m21, T m22, T m23,
            T m30, T m31, T m32, T m33) {
        // Row-major input, stored as column-major
        m[0]  = m00; m[4]  = m01; m[8]  = m02; m[12] = m03;
        m[1]  = m10; m[5]  = m11; m[9]  = m12; m[13] = m13;
        m[2]  = m20; m[6]  = m21; m[10] = m22; m[14] = m23;
        m[3]  = m30; m[7]  = m31; m[11] = m32; m[15] = m33;
    }

    // Element access
    T& operator()(int row, int col) {
        return m[col * 4 + row];
    }

    const T& operator()(int row, int col) const {
        return m[col * 4 + row];
    }

    T& operator[](int index) {
        return m[index];
    }

    const T& operator[](int index) const {
        return m[index];
    }

    // Matrix operations
    Matrix4 operator+(const Matrix4& other) const {
        Matrix4 result;
        for (int i = 0; i < 16; i++) {
            result.m[i] = m[i] + other.m[i];
        }
        return result;
    }

    Matrix4 operator-(const Matrix4& other) const {
        Matrix4 result;
        for (int i = 0; i < 16; i++) {
            result.m[i] = m[i] - other.m[i];
        }
        return result;
    }

    Matrix4 operator*(const Matrix4& other) const {
        Matrix4 result(T(0));
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                T sum = T(0);
                for (int k = 0; k < 4; k++) {
                    sum += (*this)(row, k) * other(k, col);
                }
                result(row, col) = sum;
            }
        }
        return result;
    }

    Matrix4 operator*(T scalar) const {
        Matrix4 result;
        for (int i = 0; i < 16; i++) {
            result.m[i] = m[i] * scalar;
        }
        return result;
    }

    Vector4<T> operator*(const Vector4<T>& vec) const {
        return Vector4<T>(
            m[0] * vec.x + m[4] * vec.y + m[8]  * vec.z + m[12] * vec.w,
            m[1] * vec.x + m[5] * vec.y + m[9]  * vec.z + m[13] * vec.w,
            m[2] * vec.x + m[6] * vec.y + m[10] * vec.z + m[14] * vec.w,
            m[3] * vec.x + m[7] * vec.y + m[11] * vec.z + m[15] * vec.w
        );
    }

    Vector3<T> transformPoint(const Vector3<T>& point) const {
        Vector4<T> v(point, T(1));
        Vector4<T> result = (*this) * v;
        return Vector3<T>(result.x, result.y, result.z);
    }

    Vector3<T> transformVector(const Vector3<T>& vec) const {
        Vector4<T> v(vec, T(0));
        Vector4<T> result = (*this) * v;
        return Vector3<T>(result.x, result.y, result.z);
    }

    // Matrix-specific operations
    void setIdentity() {
        std::memset(m, 0, sizeof(m));
        m[0] = m[5] = m[10] = m[15] = T(1);
    }

    Matrix4 transposed() const {
        return Matrix4(
            m[0], m[1], m[2],  m[3],
            m[4], m[5], m[6],  m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15]
        );
    }

    T determinant() const {
        T a = m[0], b = m[4], c = m[8],  d = m[12];
        T e = m[1], f = m[5], g = m[9],  h = m[13];
        T i = m[2], j = m[6], k = m[10], l = m[14];
        T mm = m[3], n = m[7], o = m[11], p = m[15];

        T kp_lo = k * p - l * o;
        T jp_ln = j * p - l * n;
        T jo_kn = j * o - k * n;
        T ip_lm = i * p - l * mm;
        T io_km = i * o - k * mm;
        T in_jm = i * n - j * mm;

        return a * (f * kp_lo - g * jp_ln + h * jo_kn) -
               b * (e * kp_lo - g * ip_lm + h * io_km) +
               c * (e * jp_ln - f * ip_lm + h * in_jm) -
               d * (e * jo_kn - f * io_km + g * in_jm);
    }

    Matrix4 inverted() const {
        T a = m[0], b = m[4], c = m[8],  d = m[12];
        T e = m[1], f = m[5], g = m[9],  h = m[13];
        T i = m[2], j = m[6], k = m[10], l = m[14];
        T mm = m[3], n = m[7], o = m[11], p = m[15];

        T kp_lo = k * p - l * o;
        T jp_ln = j * p - l * n;
        T jo_kn = j * o - k * n;
        T ip_lm = i * p - l * mm;
        T io_km = i * o - k * mm;
        T in_jm = i * n - j * mm;

        T a11 = +(f * kp_lo - g * jp_ln + h * jo_kn);
        T a12 = -(e * kp_lo - g * ip_lm + h * io_km);
        T a13 = +(e * jp_ln - f * ip_lm + h * in_jm);
        T a14 = -(e * jo_kn - f * io_km + g * in_jm);

        T det = a * a11 + b * a12 + c * a13 + d * a14;

        if (std::abs(det) < Constants<T>::EPSILON) {
            return Matrix4(); // Return identity if not invertible
        }

        T invDet = T(1) / det;

        T gp_ho = g * p - h * o;
        T fp_hn = f * p - h * n;
        T fo_gn = f * o - g * n;
        T ep_hm = e * p - h * mm;
        T eo_gm = e * o - g * mm;
        T en_fm = e * n - f * mm;
        T gl_hk = g * l - h * k;
        T fl_hj = f * l - h * j;
        T fk_gj = f * k - g * j;
        T el_hi = e * l - h * i;
        T ek_gi = e * k - g * i;
        T ej_fi = e * j - f * i;

        Matrix4 result;
        result.m[0]  = a11 * invDet;
        result.m[1]  = a12 * invDet;
        result.m[2]  = a13 * invDet;
        result.m[3]  = a14 * invDet;
        result.m[4]  = -(b * kp_lo - c * jp_ln + d * jo_kn) * invDet;
        result.m[5]  = +(a * kp_lo - c * ip_lm + d * io_km) * invDet;
        result.m[6]  = -(a * jp_ln - b * ip_lm + d * in_jm) * invDet;
        result.m[7]  = +(a * jo_kn - b * io_km + c * in_jm) * invDet;
        result.m[8]  = +(b * gp_ho - c * fp_hn + d * fo_gn) * invDet;
        result.m[9]  = -(a * gp_ho - c * ep_hm + d * eo_gm) * invDet;
        result.m[10] = +(a * fp_hn - b * ep_hm + d * en_fm) * invDet;
        result.m[11] = -(a * fo_gn - b * eo_gm + c * en_fm) * invDet;
        result.m[12] = -(b * gl_hk - c * fl_hj + d * fk_gj) * invDet;
        result.m[13] = +(a * gl_hk - c * el_hi + d * ek_gi) * invDet;
        result.m[14] = -(a * fl_hj - b * el_hi + d * ej_fi) * invDet;
        result.m[15] = +(a * fk_gj - b * ek_gi + c * ej_fi) * invDet;

        return result;
    }

    // Static factory methods
    static Matrix4 identity() {
        return Matrix4(T(1));
    }

    static Matrix4 translation(T x, T y, T z) {
        Matrix4 result;
        result.m[12] = x;
        result.m[13] = y;
        result.m[14] = z;
        return result;
    }

    static Matrix4 translation(const Vector3<T>& v) {
        return translation(v.x, v.y, v.z);
    }

    static Matrix4 scale(T x, T y, T z) {
        Matrix4 result(T(0));
        result.m[0] = x;
        result.m[5] = y;
        result.m[10] = z;
        result.m[15] = T(1);
        return result;
    }

    static Matrix4 scale(const Vector3<T>& v) {
        return scale(v.x, v.y, v.z);
    }

    static Matrix4 scale(T s) {
        return scale(s, s, s);
    }

    static Matrix4 rotationX(T angle) {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix4(
            T(1), T(0), T(0), T(0),
            T(0), c,    -s,   T(0),
            T(0), s,    c,    T(0),
            T(0), T(0), T(0), T(1)
        );
    }

    static Matrix4 rotationY(T angle) {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix4(
            c,    T(0), s,    T(0),
            T(0), T(1), T(0), T(0),
            -s,   T(0), c,    T(0),
            T(0), T(0), T(0), T(1)
        );
    }

    static Matrix4 rotationZ(T angle) {
        T c = std::cos(angle);
        T s = std::sin(angle);
        return Matrix4(
            c,    -s,   T(0), T(0),
            s,    c,    T(0), T(0),
            T(0), T(0), T(1), T(0),
            T(0), T(0), T(0), T(1)
        );
    }

    static Matrix4 rotation(const Vector3<T>& axis, T angle) {
        T c = std::cos(angle);
        T s = std::sin(angle);
        T t = T(1) - c;

        Vector3<T> a = axis.normalized();
        T x = a.x, y = a.y, z = a.z;

        return Matrix4(
            t*x*x + c,   t*x*y - s*z, t*x*z + s*y, T(0),
            t*x*y + s*z, t*y*y + c,   t*y*z - s*x, T(0),
            t*x*z - s*y, t*y*z + s*x, t*z*z + c,   T(0),
            T(0),        T(0),        T(0),        T(1)
        );
    }

    static Matrix4 lookAt(const Vector3<T>& eye, const Vector3<T>& center, const Vector3<T>& up) {
        Vector3<T> f = (center - eye).normalized();
        Vector3<T> s = f.cross(up).normalized();
        Vector3<T> u = s.cross(f);

        Matrix4 result;
        result.m[0] = s.x;
        result.m[4] = s.y;
        result.m[8] = s.z;
        result.m[1] = u.x;
        result.m[5] = u.y;
        result.m[9] = u.z;
        result.m[2] = -f.x;
        result.m[6] = -f.y;
        result.m[10] = -f.z;
        result.m[12] = -s.dot(eye);
        result.m[13] = -u.dot(eye);
        result.m[14] = f.dot(eye);
        return result;
    }

    static Matrix4 perspective(T fov, T aspect, T near, T far) {
        T tanHalfFov = std::tan(fov / T(2));

        Matrix4 result(T(0));
        result.m[0] = T(1) / (aspect * tanHalfFov);
        result.m[5] = T(1) / tanHalfFov;
        result.m[10] = -(far + near) / (far - near);
        result.m[11] = -T(1);
        result.m[14] = -(T(2) * far * near) / (far - near);
        return result;
    }

    static Matrix4 ortho(T left, T right, T bottom, T top, T near, T far) {
        Matrix4 result;
        result.m[0] = T(2) / (right - left);
        result.m[5] = T(2) / (top - bottom);
        result.m[10] = -T(2) / (far - near);
        result.m[12] = -(right + left) / (right - left);
        result.m[13] = -(top + bottom) / (top - bottom);
        result.m[14] = -(far + near) / (far - near);
        return result;
    }
};

// Scalar * Matrix
template<typename T>
inline Matrix4<T> operator*(T scalar, const Matrix4<T>& mat) {
    return mat * scalar;
}

// Stream output
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Matrix4<T>& mat) {
    os << "[\n";
    for (int row = 0; row < 4; row++) {
        os << "  [";
        for (int col = 0; col < 4; col++) {
            os << mat(row, col);
            if (col < 3) os << ", ";
        }
        os << "]";
        if (row < 3) os << ",";
        os << "\n";
    }
    os << "]";
    return os;
}

// Common type aliases
using Mat4f = Matrix4<f32>;
using Mat4d = Matrix4<f64>;

} // namespace math
} // namespace oracon

#endif // ORACON_MATH_MATRIX_H
