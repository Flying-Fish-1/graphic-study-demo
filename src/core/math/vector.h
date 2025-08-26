#ifndef CORE_MATH_VECTOR_H
#define CORE_MATH_VECTOR_H

#include <cmath>

namespace Core {
namespace Math {

/**
 * @brief 数学常量
 */
namespace Constants {
    constexpr float PI = 3.14159265359f;
    constexpr float TAU = 2.0f * PI;
    constexpr float PI_2 = PI / 2.0f;
    constexpr float PI_4 = PI / 4.0f;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;
}

/**
 * @brief 2D向量类 - 纯数学实现
 */
struct Vector2 {
    float x, y;
    
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
    
    // 基本运算
    Vector2 operator+(const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
    Vector2 operator-(const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
    Vector2 operator*(float s) const { return Vector2(x * s, y * s); }
    Vector2 operator/(float s) const { return Vector2(x / s, y / s); }
    
    // 数学运算
    float dot(const Vector2& v) const;
    float length() const;
    float lengthSquared() const;
    Vector2 normalize() const;
};

/**
 * @brief 3D向量类 - 纯数学实现
 */
struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(const Vector2& v, float z = 0) : x(v.x), y(v.y), z(z) {}
    
    // 基本运算
    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator-() const { return Vector3(-x, -y, -z); }  // 一元负号运算符
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }
    
    // 数学运算
    float dot(const Vector3& v) const;
    Vector3 cross(const Vector3& v) const;
    float length() const;
    float lengthSquared() const;
    Vector3 normalize() const;
    Vector2 xy() const;
};

/**
 * @brief 4D向量类 - 纯数学实现
 */
struct Vector4 {
    float x, y, z, w;
    
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(const Vector3& v, float w = 1.0f) : x(v.x), y(v.y), z(v.z), w(w) {}
    
    // 基本运算
    Vector4 operator+(const Vector4& v) const { return Vector4(x + v.x, y + v.y, z + v.z, w + v.w); }
    Vector4 operator-(const Vector4& v) const { return Vector4(x - v.x, y - v.y, z - v.z, w - v.w); }
    Vector4 operator*(float s) const { return Vector4(x * s, y * s, z * s, w * s); }
    Vector4 operator/(float s) const { return Vector4(x / s, y / s, z / s, w / s); }
    
    // 数学运算
    float dot(const Vector4& v) const;
    float length() const;
    float lengthSquared() const;
    Vector4 normalize() const;
    Vector3 xyz() const;
};

} // namespace Math
} // namespace Core

#endif // CORE_MATH_VECTOR_H
