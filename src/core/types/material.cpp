#include "material.h"
#include "texture.h"
#include <algorithm>
#include <cmath>

namespace Core {
namespace Types {

Material::Material()
    : m_ambient(0.1f, 0.1f, 0.1f, 1.0f),
      m_diffuse(0.8f, 0.8f, 0.8f, 1.0f),
      m_specular(0.5f, 0.5f, 0.5f, 1.0f),
      m_shininess(32.0f),
      m_diffuseMap(nullptr),
      m_normalMap(nullptr) {}

Material::Material(const Color& ambient, const Color& diffuse, const Color& specular, float shininess)
    : m_ambient(ambient),
      m_diffuse(diffuse),
      m_specular(specular),
      m_shininess(shininess),
      m_diffuseMap(nullptr),
      m_normalMap(nullptr) {}

Material::~Material() = default;

Color Material::calculateLighting(const Vector3& normal, const Vector3& lightDir,
                                  const Vector3& viewDir, const Vector3& /*worldPos*/) const {
    Color ambient = m_ambient * 0.1f;

    float NdotL = std::max(0.0f, normal.dot(lightDir));
    Color diffuse = m_diffuse * NdotL;

    Vector3 halfVector = (lightDir + viewDir).normalize();
    float NdotH = std::max(0.0f, normal.dot(halfVector));
    float specularPower = std::pow(NdotH, m_shininess);
    Color specular = m_specular * specularPower;

    return ambient + diffuse + specular;
}

Color Material::sampleAlbedo(const Vector2& texCoord) const {
    if (m_diffuseMap) {
        return m_diffuseMap->sample(texCoord.x, texCoord.y);
    }
    return m_diffuse;
}

Color Material::sampleAlbedo(const Vector2& texCoord,
                             float dudx, float dudy,
                             float dvdx, float dvdy) const {
    if (m_diffuseMap) {
        return m_diffuseMap->sample(texCoord.x, texCoord.y, dudx, dudy, dvdx, dvdy);
    }
    return m_diffuse;
}

Vector3 Material::sampleNormal(const Vector2& texCoord) const {
    if (m_normalMap) {
        Color normalColor = m_normalMap->sample(texCoord.x, texCoord.y);
        Vector3 normal(
            normalColor.r * 2.0f - 1.0f,
            normalColor.g * 2.0f - 1.0f,
            normalColor.b * 2.0f - 1.0f
        );
        return normal.normalize();
    }
    return Vector3(0.0f, 0.0f, 1.0f);
}

const uint32_t* Material::getDiffuseMapPixels() const {
    if (m_diffuseMap) {
        return m_diffuseMap->getPixels();
    }
    return nullptr;
}

Material* Material::createDefaultMaterial() {
    return new Material(
        Color(0.1f, 0.1f, 0.1f, 1.0f),
        Color(0.8f, 0.8f, 0.8f, 1.0f),
        Color(0.5f, 0.5f, 0.5f, 1.0f),
        32.0f
    );
}

Material* Material::createRedPlastic() {
    return new Material(
        Color(0.1f, 0.02f, 0.02f, 1.0f),
        Color(0.8f, 0.1f, 0.1f, 1.0f),
        Color(0.9f, 0.9f, 0.9f, 1.0f),
        64.0f
    );
}

Material* Material::createBlueMetal() {
    return new Material(
        Color(0.02f, 0.02f, 0.1f, 1.0f),
        Color(0.1f, 0.2f, 0.8f, 1.0f),
        Color(0.8f, 0.8f, 0.9f, 1.0f),
        128.0f
    );
}

Material* Material::createWhiteDiffuse() {
    return new Material(
        Color(0.1f, 0.1f, 0.1f, 1.0f),
        Color(0.9f, 0.9f, 0.9f, 1.0f),
        Color(0.1f, 0.1f, 0.1f, 1.0f),
        8.0f
    );
}

} // namespace Types
} // namespace Core
