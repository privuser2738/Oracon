#include "oracon/math/math.h"
#include <iostream>

using namespace oracon::math;

int main() {
    std::cout << "=== OraconMath Library Test ===" << std::endl;
    std::cout << std::endl;

    // Test constants
    std::cout << "Constants:" << std::endl;
    std::cout << "  PI: " << PI_F << std::endl;
    std::cout << "  E: " << ConstantsF::E << std::endl;
    std::cout << std::endl;

    // Test Vector2
    std::cout << "Vector2 operations:" << std::endl;
    Vec2f v1(3.0f, 4.0f);
    Vec2f v2(1.0f, 2.0f);
    std::cout << "  v1: " << v1 << std::endl;
    std::cout << "  v2: " << v2 << std::endl;
    std::cout << "  v1 + v2: " << (v1 + v2) << std::endl;
    std::cout << "  v1.length(): " << v1.length() << std::endl;
    std::cout << "  v1.dot(v2): " << v1.dot(v2) << std::endl;
    std::cout << std::endl;

    // Test Vector3
    std::cout << "Vector3 operations:" << std::endl;
    Vec3f v3(1.0f, 0.0f, 0.0f);
    Vec3f v4(0.0f, 1.0f, 0.0f);
    std::cout << "  v3: " << v3 << std::endl;
    std::cout << "  v4: " << v4 << std::endl;
    std::cout << "  v3.cross(v4): " << v3.cross(v4) << std::endl;
    std::cout << "  v3 * 5: " << (v3 * 5.0f) << std::endl;
    std::cout << std::endl;

    // Test Matrix4
    std::cout << "Matrix4 operations:" << std::endl;
    Mat4f identity = Mat4f::identity();
    Mat4f translation = Mat4f::translation(1.0f, 2.0f, 3.0f);
    Mat4f scale = Mat4f::scale(2.0f);
    std::cout << "  Identity matrix:" << std::endl << identity << std::endl;
    std::cout << "  Translation(1, 2, 3) * Scale(2):" << std::endl;
    Mat4f combined = translation * scale;
    Vec3f point(1.0f, 1.0f, 1.0f);
    Vec3f transformed = combined.transformPoint(point);
    std::cout << "  Point (1, 1, 1) transformed: " << transformed << std::endl;
    std::cout << std::endl;

    // Test Quaternion
    std::cout << "Quaternion operations:" << std::endl;
    Quatf q1 = Quatf::fromAxisAngle(Vec3f::unitZ(), radians(90.0f));
    Vec3f v5(1.0f, 0.0f, 0.0f);
    Vec3f rotated = q1.rotate(v5);
    std::cout << "  Rotate (1, 0, 0) by 90° around Z: " << rotated << std::endl;
    std::cout << "  Quaternion: " << q1 << std::endl;
    std::cout << std::endl;

    // Test math functions
    std::cout << "Math functions:" << std::endl;
    std::cout << "  lerp(0, 10, 0.5): " << lerp(0.0f, 10.0f, 0.5f) << std::endl;
    std::cout << "  clamp(15, 0, 10): " << clamp(15.0f, 0.0f, 10.0f) << std::endl;
    std::cout << "  radians(180): " << radians(180.0f) << std::endl;
    std::cout << "  degrees(PI): " << degrees(PI_F) << std::endl;
    std::cout << std::endl;

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
