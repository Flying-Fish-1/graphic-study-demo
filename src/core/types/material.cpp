#include "material.h"
#include "texture.h"
#include <cmath>

namespace Core {
namespace Types {

Material::Material() 
    : m_ambient(0.1f, 0.1f, 0.1f, 1.0f),
      m_diffuse(0.8f, 0.8f, 0.8f, 1.0f),
      m_specular(0.5f, 0.5f, 0.5f, 1.0f),
      m_shininess(32.0f),
      m_diffuseMap(nullptr),
      m_normalMap(nullptr) {
}

Material::Material(const Color& ambient, const Color& diffuse, const Color& specular, float shininess)
    : m_ambient(ambient),
      m_diffuse(diffuse),
      m_specular(specular),
      m_shininess(shininess),
      m_diffuseMap(nullptr),
      m_normalMap(nullptr) {
}

Material::~Material() {
    // 注意：这里不删除纹理，因为纹理可能被多个材质共享
    // 纹理的生命周期应该由纹理管理器负责
}

Color Material::calculateLighting(const Vector3& normal, const Vector3& lightDir, 
                                 const Vector3& viewDir, const Vector3& worldPos, 
                                 const Light& light) const {
    // 简化的光照计算，暂时不依赖Light类的具体实现
    // 环境光分量
    Color ambient = Color(m_ambient.r * 0.1f, m_ambient.g * 0.1f, m_ambient.b * 0.1f, m_ambient.a);
    
    // 漫反射分量 (Lambert)
    float NdotL = std::max(0.0f, normal.dot(lightDir));
    Color diffuse = Color(m_diffuse.r * NdotL, m_diffuse.g * NdotL, m_diffuse.b * NdotL, m_diffuse.a);
    
    // 镜面反射分量 (Blinn-Phong)
    Vector3 halfVector = (lightDir + viewDir).normalize();
    float NdotH = std::max(0.0f, normal.dot(halfVector));
    float specularPower = std::pow(NdotH, m_shininess);
    Color specular = Color(m_specular.r * specularPower, m_specular.g * specularPower, 
                          m_specular.b * specularPower, m_specular.a);
    
    return Color(
        ambient.r + diffuse.r + specular.r,
        ambient.g + diffuse.g + specular.g,
        ambient.b + diffuse.b + specular.b,
        ambient.a
    );
}

Color Material::getDiffuseColor(const Vector2& texCoord) const {
    if (m_diffuseMap) {
        // 如果有漫反射贴图，采样纹理
        Color textureColor = m_diffuseMap->sample(texCoord.x, texCoord.y);
        return Color(
            textureColor.r * m_diffuse.r,
            textureColor.g * m_diffuse.g,
            textureColor.b * m_diffuse.b,
            textureColor.a * m_diffuse.a
        );
    } else {
        // 否则返回材质的漫反射颜色
        return m_diffuse;
    }
}

Vector3 Material::getNormal(const Vector2& texCoord) const {
    if (m_normalMap) {
        // 如果有法线贴图，从贴图中获取法向量
        Color normalColor = m_normalMap->sample(texCoord.x, texCoord.y);
        
        // 将颜色值转换为法向量 [0,1] -> [-1,1]
        Vector3 normal(
            normalColor.r * 2.0f - 1.0f,
            normalColor.g * 2.0f - 1.0f,
            normalColor.b * 2.0f - 1.0f
        );
        
        return normal.normalize();
    } else {
        // 否则返回默认法向量 (0, 0, 1)
        return Vector3(0, 0, 1);
    }
}

Material* Material::createDefaultMaterial() {
    return new Material(
        Color(0.1f, 0.1f, 0.1f, 1.0f),  // 环境光
        Color(0.8f, 0.8f, 0.8f, 1.0f),  // 漫反射
        Color(0.5f, 0.5f, 0.5f, 1.0f),  // 镜面反射
        32.0f                            // 光泽度
    );
}

Material* Material::createRedPlastic() {
    return new Material(
        Color(0.1f, 0.02f, 0.02f, 1.0f), // 环境光
        Color(0.8f, 0.1f, 0.1f, 1.0f),   // 漫反射
        Color(0.9f, 0.9f, 0.9f, 1.0f),   // 镜面反射
        64.0f                             // 光泽度
    );
}

Material* Material::createBlueMetal() {
    return new Material(
        Color(0.02f, 0.02f, 0.1f, 1.0f), // 环境光
        Color(0.1f, 0.2f, 0.8f, 1.0f),   // 漫反射
        Color(0.8f, 0.8f, 0.9f, 1.0f),   // 镜面反射
        128.0f                            // 光泽度
    );
}

Material* Material::createWhiteDiffuse() {
    return new Material(
        Color(0.1f, 0.1f, 0.1f, 1.0f),   // 环境光
        Color(0.9f, 0.9f, 0.9f, 1.0f),   // 漫反射
        Color(0.1f, 0.1f, 0.1f, 1.0f),   // 镜面反射
        8.0f                              // 光泽度
    );
}

} // namespace Types
} // namespace Core
