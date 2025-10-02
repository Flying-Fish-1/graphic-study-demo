#include "shading_pipeline.h"

#include <algorithm>
#include <cmath>

#include "software_renderer.h"

#include "../../core/types/material.h"
#include "../../core/types/color.h"
#include "../../renderer/lighting/light.h"

namespace Renderer {
namespace Pipeline {

ShadingPipeline::ShadingPipeline(const SoftwareRendererSettings& settings)
    : m_settings(settings) {}

Core::Types::Color ShadingPipeline::shade(const GeometryVertex& interpolated,
                                          Core::Types::Material* material,
                                          const std::vector<Renderer::Lighting::Light*>& lights,
                                          const Core::Math::Vector3& viewPos,
                                          const Core::Types::Color& sceneAmbient,
                                          const RasterDerivatives& derivs) const {
    using Core::Types::Color;
    using Core::Math::Vector3;

    Color baseColor = interpolated.color;
    if (material) {
        Color albedo = material->sampleAlbedo(interpolated.texCoord,
                                              derivs.dudx, derivs.dudy,
                                              derivs.dvdx, derivs.dvdy);
        baseColor = albedo * baseColor;
    }

    Vector3 normal = interpolated.normal;
    if (material && material->getNormalMap()) {
        Vector3 tangentSpaceNormal = material->sampleNormal(interpolated.texCoord);
        Vector3 t = interpolated.tangent.normalize();
        Vector3 b = interpolated.bitangent.normalize();
        Vector3 n = interpolated.normal.normalize();
        normal = (t * tangentSpaceNormal.x + b * tangentSpaceNormal.y + n * tangentSpaceNormal.z).normalize();
    } else {
        normal = normal.normalize();
    }

    Vector3 viewDir = (viewPos - interpolated.worldPosition).normalize();

    float baseAlpha = std::clamp(baseColor.a, 0.0f, 1.0f);
    Color ambient = sceneAmbient * baseColor;
    ambient.a = 0.0f;

    Color diffuseAccum = ambient;
    Color specularAccum(0.0f, 0.0f, 0.0f, 0.0f);

    for (const auto& light : lights) {
        if (!light || !light->isVisible(interpolated.worldPosition)) {
            continue;
        }

        Vector3 lightDir = light->getDirection(interpolated.worldPosition).normalize();
        float attenuation = light->getAttenuation(interpolated.worldPosition);
        if (attenuation <= 0.0f) {
            continue;
        }

        float NdotL = std::max(0.0f, normal.dot(lightDir));
        if (NdotL <= 0.0f) {
            continue;
        }

        Color lightColor = light->getColor() * (light->getIntensity() * attenuation);
        Vector3 halfVector = (lightDir + viewDir).normalize();
        Color diffuse = baseColor * lightColor * NdotL;
        diffuse.a = 0.0f;

        diffuseAccum = diffuseAccum + diffuse;
        float specPower = material ? material->getShininess() : 32.0f;
        float NdotH = std::max(0.0f, normal.dot(halfVector));
        float specularFactor = std::pow(NdotH, specPower);
        if (specularFactor > 0.0f) {
            Color specular = lightColor * specularFactor;
            Color specularColor = material ? material->getSpecular() : Color(1.0f, 1.0f, 1.0f, 1.0f);
            specular = specular * specularColor;
            specular.a = 0.0f;
            specularAccum = specularAccum + specular;
        }
    }

    Color diffuseClamped(
        std::clamp(diffuseAccum.r, 0.0f, 1.0f),
        std::clamp(diffuseAccum.g, 0.0f, 1.0f),
        std::clamp(diffuseAccum.b, 0.0f, 1.0f),
        0.0f);

    Color specularClamped(
        std::clamp(specularAccum.r, 0.0f, 1.0f),
        std::clamp(specularAccum.g, 0.0f, 1.0f),
        std::clamp(specularAccum.b, 0.0f, 1.0f),
        0.0f);

    Color finalColor(
        diffuseClamped.r * baseAlpha + specularClamped.r,
        diffuseClamped.g * baseAlpha + specularClamped.g,
        diffuseClamped.b * baseAlpha + specularClamped.b,
        baseAlpha);

    finalColor.r = std::clamp(finalColor.r, 0.0f, 1.0f);
    finalColor.g = std::clamp(finalColor.g, 0.0f, 1.0f);
    finalColor.b = std::clamp(finalColor.b, 0.0f, 1.0f);

    return finalColor;
}

} // namespace Pipeline
} // namespace Renderer
