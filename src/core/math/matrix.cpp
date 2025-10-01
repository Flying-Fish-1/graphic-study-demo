#include "matrix.h"
#include "vector.h"
#include <cmath>

namespace Core {
namespace Math {

// ==================== Matrix3 实现 ====================

Matrix3::Matrix3() {
    // 默认构造为单位矩阵
    for (int i = 0; i < 9; i++) m[i] = 0;
    m[0] = m[4] = m[8] = 1.0f;
}

Matrix3::Matrix3(std::initializer_list<float> values) {
    int i = 0;
    for (float val : values) {
        if (i < 9) m[i++] = val;
    }
    while (i < 9) m[i++] = 0;
}

Matrix3 Matrix3::operator*(const Matrix3& other) const {
    Matrix3 result;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            result.m[row * 3 + col] = 0;
            for (int k = 0; k < 3; k++) {
                result.m[row * 3 + col] += m[row * 3 + k] * other.m[k * 3 + col];
            }
        }
    }
    return result;
}

Vector3 Matrix3::operator*(const Vector3& v) const {
    return Vector3(
        m[0] * v.x + m[1] * v.y + m[2] * v.z,
        m[3] * v.x + m[4] * v.y + m[5] * v.z,
        m[6] * v.x + m[7] * v.y + m[8] * v.z
    );
}

Vector2 Matrix3::operator*(const Vector2& v) const {
    Vector3 homogeneous = *this * Vector3(v.x, v.y, 1.0f);
    return Vector2(homogeneous.x / homogeneous.z, homogeneous.y / homogeneous.z);
}

Vector2 Matrix3::transform(const Vector2& v) const {
    Vector3 homogeneous = *this * Vector3(v.x, v.y, 1.0f);
    return Vector2(homogeneous.x / homogeneous.z, homogeneous.y / homogeneous.z);
}

Matrix3 Matrix3::identity() {
    return Matrix3();
}

Matrix3 Matrix3::translation(float tx, float ty) {
    return Matrix3{
        1, 0, tx,
        0, 1, ty,
        0, 0, 1
    };
}

Matrix3 Matrix3::rotation(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return Matrix3{
        c, -s, 0,
        s,  c, 0,
        0,  0, 1
    };
}

Matrix3 Matrix3::scale(float sx, float sy) {
    return Matrix3{
        sx, 0,  0,
        0,  sy, 0,
        0,  0,  1
    };
}

// ==================== Matrix4 实现 ====================

Matrix4::Matrix4() {
    // 默认构造为单位矩阵
    for (int i = 0; i < 16; i++) m[i] = 0;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

Matrix4::Matrix4(std::initializer_list<float> values) {
    int i = 0;
    for (float val : values) {
        if (i < 16) m[i++] = val;
    }
    while (i < 16) m[i++] = 0;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row * 4 + col] = 0;
            for (int k = 0; k < 4; k++) {
                result.m[row * 4 + col] += m[row * 4 + k] * other.m[k * 4 + col];
            }
        }
    }
    return result;
}

Vector3 Matrix4::operator*(const Vector3& v) const {
    return Vector3(
        m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3],
        m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7],
        m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11]
    );
}

Vector4 Matrix4::operator*(const Vector4& v) const {
    return Vector4(
        m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w,
        m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w,
        m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w,
        m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w
    );
}

Vector3 Matrix4::transform(const Vector3& v) const {
    return *this * v;
}

Vector3 Matrix4::transformPoint(const Vector3& v) const {
    float x = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
    float y = m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7];
    float z = m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11];
    float w = m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15];
    
    if (w != 0) {
        return Vector3(x/w, y/w, z/w);
    }
    return Vector3(x, y, z);
}

Vector3 Matrix4::transformDirection(const Vector3& v) const {
    return Vector3(
        m[0] * v.x + m[1] * v.y + m[2] * v.z,
        m[4] * v.x + m[5] * v.y + m[6] * v.z,
        m[8] * v.x + m[9] * v.y + m[10] * v.z
    );
}

Matrix4 Matrix4::identity() {
    return Matrix4();
}

Matrix4 Matrix4::translation(float tx, float ty, float tz) {
    return Matrix4{
        1, 0, 0, tx,
        0, 1, 0, ty,
        0, 0, 1, tz,
        0, 0, 0, 1
    };
}

Matrix4 Matrix4::rotationX(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return Matrix4{
        1, 0,  0, 0,
        0, c, -s, 0,
        0, s,  c, 0,
        0, 0,  0, 1
    };
}

Matrix4 Matrix4::rotationY(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return Matrix4{
         c, 0, s, 0,
         0, 1, 0, 0,
        -s, 0, c, 0,
         0, 0, 0, 1
    };
}

Matrix4 Matrix4::rotationZ(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return Matrix4{
        c, -s, 0, 0,
        s,  c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1
    };
}

Matrix4 Matrix4::scale(float sx, float sy, float sz) {
    return Matrix4{
        sx, 0,  0,  0,
        0,  sy, 0,  0,
        0,  0,  sz, 0,
        0,  0,  0,  1
    };
}

Matrix4 Matrix4::perspective(float fov, float aspect, float near, float far) {
    float f = 1.0f / std::tan(fov * 0.5f);
    float range = far - near;

    // Left-handed / D3D-style 透视矩阵，NDC Z ∈ [0,1]
    return Matrix4{
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, far / range, -near * far / range,
        0.0f, 0.0f, 1.0f, 0.0f
    };
}

Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float near, float far) {
    return Matrix4{
        2.0f / (right - left), 0, 0, -(right + left) / (right - left),
        0, 2.0f / (top - bottom), 0, -(top + bottom) / (top - bottom),
        0, 0, -2.0f / (far - near), -(far + near) / (far - near),
        0, 0, 0, 1
    };
}

// ==================== Utils 实现 ====================

namespace Utils {

float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float degrees(float radians) {
    return radians * Constants::RAD_TO_DEG;
}

float radians(float degrees) {
    return degrees * Constants::DEG_TO_RAD;
}

} // namespace Utils

} // namespace Math
} // namespace Core
