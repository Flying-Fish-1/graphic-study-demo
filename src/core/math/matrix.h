#ifndef CORE_MATH_MATRIX_H
#define CORE_MATH_MATRIX_H

#include "vector.h"
#include <initializer_list>

namespace Core {
namespace Math {

/**
 * @brief 3x3矩阵类 - 纯数学实现
 */
struct Matrix3 {
    float m[9]; // 按行存储
    
    Matrix3();
    Matrix3(std::initializer_list<float> values);
    
    // 数学运算
    Matrix3 operator*(const Matrix3& other) const;
    Vector2 operator*(const Vector2& v) const;
    Vector3 operator*(const Vector3& v) const;
    
    // 静态创建方法 (这些是图形学概念，但在这里作为数学工具)
    static Matrix3 identity();
    static Matrix3 translation(float tx, float ty);
    static Matrix3 rotation(float angle);
    static Matrix3 scale(float sx, float sy);
    
    // 辅助方法
    Vector2 transform(const Vector2& v) const;
};

/**
 * @brief 4x4矩阵类 - 纯数学实现
 */
struct Matrix4 {
    float m[16]; // 按行存储
    
    Matrix4();
    Matrix4(std::initializer_list<float> values);
    
    // 数学运算
    Matrix4 operator*(const Matrix4& other) const;
    Vector3 operator*(const Vector3& v) const;
    
    // 静态创建方法
    static Matrix4 identity();
    static Matrix4 translation(float tx, float ty, float tz);
    static Matrix4 rotationX(float angle);
    static Matrix4 rotationY(float angle);
    static Matrix4 rotationZ(float angle);
    static Matrix4 scale(float sx, float sy, float sz);
    static Matrix4 perspective(float fov, float aspect, float near, float far);
    static Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far);
    
    // 辅助方法
    Vector3 transform(const Vector3& v) const;
    Vector3 transformPoint(const Vector3& v) const;
    Vector3 transformDirection(const Vector3& v) const;
};

/**
 * @brief 数学工具函数
 */
namespace Utils {
    float clamp(float value, float min, float max);
    float lerp(float a, float b, float t);
    float degrees(float radians);
    float radians(float degrees);
}

} // namespace Math
} // namespace Core

#endif // CORE_MATH_MATRIX_H
