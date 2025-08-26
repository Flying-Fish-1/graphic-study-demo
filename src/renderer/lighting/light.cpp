#include "light.h"
#include <cmath>
#include <algorithm>

namespace Renderer {
namespace Lighting {

// ==================== Light 基类实现 ====================

Light::Light(Type type, const Color& color, float intensity)
    : m_type(type), m_color(color), m_intensity(intensity), m_ambientIntensity(0.1f) {
}

// ==================== PointLight 实现 ====================

PointLight::PointLight(const Vector3& position, const Color& color, float intensity, float range)
    : Light(POINT, color, intensity), 
      m_position(position), 
      m_range(range),
      m_constantAttenuation(1.0f),
      m_linearAttenuation(0.09f),
      m_quadraticAttenuation(0.032f) {
}

Vector3 PointLight::getDirection(const Vector3& worldPos) const {
    return (m_position - worldPos).normalize();
}

float PointLight::getAttenuation(const Vector3& worldPos) const {
    float distance = (m_position - worldPos).length();
    
    if (distance > m_range) {
        return 0.0f;
    }
    
    // 使用物理衰减公式: 1 / (constant + linear * d + quadratic * d^2)
    float attenuation = 1.0f / (m_constantAttenuation + 
                               m_linearAttenuation * distance + 
                               m_quadraticAttenuation * distance * distance);
    
    return std::min(attenuation, 1.0f);
}

bool PointLight::isVisible(const Vector3& worldPos) const {
    float distance = (m_position - worldPos).length();
    return distance <= m_range;
}

void PointLight::setAttenuation(float constant, float linear, float quadratic) {
    m_constantAttenuation = constant;
    m_linearAttenuation = linear;
    m_quadraticAttenuation = quadratic;
}

// ==================== DirectionalLight 实现 ====================

DirectionalLight::DirectionalLight(const Vector3& direction, const Color& color, float intensity)
    : Light(DIRECTIONAL, color, intensity), m_direction(direction.normalize()) {
}

Vector3 DirectionalLight::getDirection(const Vector3& worldPos) const {
    // 方向光的方向不依赖于位置
    return -m_direction; // 返回光线方向（从表面指向光源）
}

float DirectionalLight::getAttenuation(const Vector3& worldPos) const {
    // 方向光没有衰减
    return 1.0f;
}

bool DirectionalLight::isVisible(const Vector3& worldPos) const {
    // 方向光总是可见的
    return true;
}

} // namespace Lighting
} // namespace Renderer
