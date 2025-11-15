#ifndef ORACON_MATH_FUNCTIONS_H
#define ORACON_MATH_FUNCTIONS_H

#include "oracon/math/constants.h"
#include <cmath>
#include <algorithm>
#include <type_traits>

namespace oracon {
namespace math {

using core::i32;
using core::u32;

// Angle conversion
template<typename T>
inline T radians(T degrees) {
    return degrees * Constants<T>::DEG_TO_RAD;
}

template<typename T>
inline T degrees(T radians) {
    return radians * Constants<T>::RAD_TO_DEG;
}

// Clamping
template<typename T>
inline T clamp(T value, T min, T max) {
    return std::max(min, std::min(value, max));
}

// Linear interpolation
template<typename T>
inline T lerp(T a, T b, T t) {
    return a + (b - a) * t;
}

// Smooth step (cubic Hermite interpolation)
template<typename T>
inline T smoothstep(T edge0, T edge1, T x) {
    T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
    return t * t * (T(3) - T(2) * t);
}

// Smooth step with derivatives equal to zero at edges
template<typename T>
inline T smootherstep(T edge0, T edge1, T x) {
    T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
    return t * t * t * (t * (t * T(6) - T(15)) + T(10));
}

// Float comparison with epsilon
template<typename T>
inline bool floatEqual(T a, T b, T epsilon = Constants<T>::EPSILON) {
    return std::abs(a - b) <= epsilon;
}

// Sign function
template<typename T>
inline T sign(T value) {
    return (T(0) < value) - (value < T(0));
}

// Modulo that works with negative numbers
template<typename T>
inline T mod(T a, T b) {
    T result = std::fmod(a, b);
    return result < T(0) ? result + b : result;
}

// Map value from one range to another
template<typename T>
inline T map(T value, T inMin, T inMax, T outMin, T outMax) {
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

// Power of 2 checks
inline bool isPowerOfTwo(u32 value) {
    return value > 0 && (value & (value - 1)) == 0;
}

inline u32 nextPowerOfTwo(u32 value) {
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}

// Fast inverse square root (Quake III algorithm)
inline f32 fastInvSqrt(f32 x) {
    f32 xhalf = 0.5f * x;
    i32 i = *reinterpret_cast<i32*>(&x);
    i = 0x5f3759df - (i >> 1);
    x = *reinterpret_cast<f32*>(&i);
    x = x * (1.5f - xhalf * x * x);
    return x;
}

// Minimum/Maximum of 3 values
template<typename T>
inline T min3(T a, T b, T c) {
    return std::min(std::min(a, b), c);
}

template<typename T>
inline T max3(T a, T b, T c) {
    return std::max(std::max(a, b), c);
}

} // namespace math
} // namespace oracon

#endif // ORACON_MATH_FUNCTIONS_H
