#ifndef CORE_TYPES_MATERIAL_H
#define CORE_TYPES_MATERIAL_H

#include "color.h"
#include "../math/vector.h"

namespace Core {
namespace Types {

class Texture;

using Core::Math::Vector2;
using Core::Math::Vector3;

class Material {
private:
    Color m_ambient;
    Color m_diffuse;
    Color m_specular;
    float m_shininess;
    Texture* m_diffuseMap;
    Texture* m_normalMap;

public:
    Material();
    Material(const Color& ambient, const Color& diffuse, const Color& specular, float shininess);
    ~Material();

    const Color& getAmbient() const { return m_ambient; }
    const Color& getDiffuse() const { return m_diffuse; }
    const Color& getSpecular() const { return m_specular; }
    float getShininess() const { return m_shininess; }
    Texture* getDiffuseMap() const { return m_diffuseMap; }
    Texture* getNormalMap() const { return m_normalMap; }

    void setAmbient(const Color& ambient) { m_ambient = ambient; }
    void setDiffuse(const Color& diffuse) { m_diffuse = diffuse; }
    void setSpecular(const Color& specular) { m_specular = specular; }
    void setShininess(float shininess) { m_shininess = shininess; }
    void setDiffuseMap(Texture* texture) { m_diffuseMap = texture; }
    void setNormalMap(Texture* texture) { m_normalMap = texture; }

    Color calculateLighting(const Vector3& normal, const Vector3& lightDir,
                            const Vector3& viewDir, const Vector3& worldPos) const;

    Color sampleAlbedo(const Vector2& texCoord) const;
    Color sampleAlbedo(const Vector2& texCoord,
                       float dudx, float dudy,
                       float dvdx, float dvdy) const;

    Vector3 sampleNormal(const Vector2& texCoord) const;

    const uint32_t* getDiffuseMapPixels() const;

    static Material* createDefaultMaterial();
    static Material* createRedPlastic();
    static Material* createBlueMetal();
    static Material* createWhiteDiffuse();
};

} // namespace Types
} // namespace Core

#endif // CORE_TYPES_MATERIAL_H
