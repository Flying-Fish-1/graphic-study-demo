#include "vector.h"

namespace Core {
namespace Math {

// ==================== Vector2 实现 ====================

float Vector2::dot(const Vector2& v) const { 
    return x * v.x + y * v.y; 
}

float Vector2::length() const { 
    return std::sqrt(x * x + y * y); 
}

float Vector2::lengthSquared() const { 
    return x * x + y * y; 
}

Vector2 Vector2::normalize() const {
    float len = length();
    return len > 0 ? *this / len : Vector2(0, 0);
}

// ==================== Vector3 实现 ====================

float Vector3::dot(const Vector3& v) const { 
    return x * v.x + y * v.y + z * v.z; 
}

Vector3 Vector3::cross(const Vector3& v) const {
    return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

float Vector3::length() const { 
    return std::sqrt(x * x + y * y + z * z); 
}

float Vector3::lengthSquared() const { 
    return x * x + y * y + z * z; 
}

Vector3 Vector3::normalize() const {
    float len = length();
    return len > 0 ? *this / len : Vector3(0, 0, 0);
}

Vector2 Vector3::xy() const { 
    return Vector2(x, y); 
}

// ==================== Vector4 实现 ====================

float Vector4::dot(const Vector4& v) const { 
    return x * v.x + y * v.y + z * v.z + w * v.w; 
}

float Vector4::length() const { 
    return std::sqrt(x * x + y * y + z * z + w * w); 
}

float Vector4::lengthSquared() const { 
    return x * x + y * y + z * z + w * w; 
}

Vector4 Vector4::normalize() const {
    float len = length();
    return len > 0 ? *this / len : Vector4(0, 0, 0, 0);
}

Vector3 Vector4::xyz() const { 
    return Vector3(x, y, z); 
}

} // namespace Math
} // namespace Core
