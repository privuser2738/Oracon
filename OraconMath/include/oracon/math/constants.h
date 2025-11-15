#ifndef ORACON_MATH_CONSTANTS_H
#define ORACON_MATH_CONSTANTS_H

#include "oracon/core/types.h"
#include <cmath>

namespace oracon {
namespace math {

using core::f32;
using core::f64;

// Mathematical constants
template<typename T>
struct Constants {
    static constexpr T PI = static_cast<T>(3.14159265358979323846);
    static constexpr T TWO_PI = static_cast<T>(6.28318530717958647692);
    static constexpr T HALF_PI = static_cast<T>(1.57079632679489661923);
    static constexpr T E = static_cast<T>(2.71828182845904523536);
    static constexpr T SQRT_2 = static_cast<T>(1.41421356237309504880);
    static constexpr T SQRT_3 = static_cast<T>(1.73205080756887729352);
    static constexpr T GOLDEN_RATIO = static_cast<T>(1.61803398874989484820);
    static constexpr T DEG_TO_RAD = PI / static_cast<T>(180.0);
    static constexpr T RAD_TO_DEG = static_cast<T>(180.0) / PI;
    static constexpr T EPSILON = std::numeric_limits<T>::epsilon();
};

// Common type aliases
using ConstantsF = Constants<f32>;
using ConstantsD = Constants<f64>;

// Convenience constants
inline constexpr f32 PI_F = ConstantsF::PI;
inline constexpr f64 PI_D = ConstantsD::PI;
inline constexpr f32 TWO_PI_F = ConstantsF::TWO_PI;
inline constexpr f64 TWO_PI_D = ConstantsD::TWO_PI;
inline constexpr f32 HALF_PI_F = ConstantsF::HALF_PI;
inline constexpr f64 HALF_PI_D = ConstantsD::HALF_PI;

} // namespace math
} // namespace oracon

#endif // ORACON_MATH_CONSTANTS_H
