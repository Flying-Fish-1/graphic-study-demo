#include "software_renderer.h"
#include <algorithm>
#include <cmath>

namespace Renderer {
namespace Pipeline {

using Core::Math::Vector2;
using Core::Math::Vector3;
using Core::Math::Vector4;
using Core::Math::Matrix4;
using Core::Types::Color;

SoftwareRenderer::SoftwareRenderer(const SoftwareRendererSettings& settings)
    : m_settings(settings), m_target(settings.width, settings.height) {}

void SoftwareRenderer::setSettings(const SoftwareRendererSettings& settings) {
    m_settings = settings;
    m_target.resize(m_settings.width, m_settings.height);
}

void SoftwareRenderer::render(const Scene::Scene& scene) {
    Scene::Camera* camera = scene.getCamera();
    if (!camera) {
        return;
    }

    if (m_target.getWidth() != m_settings.width || m_target.getHeight() != m_settings.height) {
        m_target.resize(m_settings.width, m_settings.height);
    }

    m_target.clear(scene.getBackgroundColor(), 1.0f);

    const Matrix4 viewMatrix = camera->getViewMatrix();
    const Matrix4 projectionMatrix = camera->getProjectionMatrix();
    const Vector3 cameraPosition = camera->getPosition();

    const auto& objects = scene.getObjects();
    const auto& lights = scene.getLights();

    for (const auto& object : objects) {
        if (!object.visible || !object.mesh) {
            continue;
        }

        Scene::Mesh* mesh = object.mesh;
        Material* material = object.materialOverride ? object.materialOverride : mesh->getMaterial();
        const auto& vertices = mesh->getVertices();
        const auto& indices = mesh->getIndices();
        if (vertices.empty() || indices.size() < 3) {
            continue;
        }

        std::vector<ScreenVertex> transformed = runGeometryStage(object, viewMatrix, projectionMatrix);

        for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            const ScreenVertex& v0 = transformed[i0];
            const ScreenVertex& v1 = transformed[i1];
            const ScreenVertex& v2 = transformed[i2];

            if (!v0.valid || !v1.valid || !v2.valid) {
                continue;
            }

            Vector3 faceNormal;
            if (!runPrimitiveAssembly(v0, v1, v2, cameraPosition, faceNormal)) {
                continue;
            }

            RasterDerivatives derivs = computeRasterDerivatives(v0, v1, v2);
            if (!std::isfinite(derivs.dudx) || !std::isfinite(derivs.dudy) ||
                !std::isfinite(derivs.dvdx) || !std::isfinite(derivs.dvdy)) {
                continue;
            }

            runRasterStage(v0, v1, v2, material, lights, cameraPosition, scene.getAmbientLight(), derivs);
        }
    }
}

std::vector<SoftwareRenderer::ScreenVertex> SoftwareRenderer::runGeometryStage(
    const Scene::SceneObject& object,
    const Matrix4& viewMatrix,
    const Matrix4& projectionMatrix) const {
    const auto& vertices = object.mesh->getVertices();
    std::vector<ScreenVertex> results(vertices.size());
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        GeometryVertex geomVertex = GeometryVertex::fromVertex(vertices[i],
                                                               object.transform,
                                                               viewMatrix,
                                                               projectionMatrix);

        float w = geomVertex.clipPosition.w;
        if (w <= 1e-6f) {
            // 标记为无效顶点，后续三角形会被剔除
            ScreenVertex invalid{};
            invalid.valid = false;
            results[i] = invalid;
            continue;
        }

        float invW = 1.0f / w;
        Vector3 ndc(geomVertex.clipPosition.x * invW,
                    geomVertex.clipPosition.y * invW,
                    geomVertex.clipPosition.z * invW);

        ScreenVertex screenVertex;
        screenVertex.attributes = geomVertex;
        screenVertex.ndcZ = geomVertex.ndcZ;
        screenVertex.screenX = (ndc.x * 0.5f + 0.5f) * (m_settings.width - 1);
        screenVertex.screenY = (1.0f - (ndc.y * 0.5f + 0.5f)) * (m_settings.height - 1);

        screenVertex.valid = true;
        results[i] = screenVertex;

    }

    return results;
}

bool SoftwareRenderer::runPrimitiveAssembly(const ScreenVertex& v0,
                                            const ScreenVertex& v1,
                                            const ScreenVertex& v2,
                                            const Vector3& cameraPos,
                                            Vector3& faceNormal) const {
    Vector3 p0 = v0.attributes.worldPosition;
    Vector3 p1 = v1.attributes.worldPosition;
    Vector3 p2 = v2.attributes.worldPosition;
    faceNormal = (p1 - p0).cross(p2 - p0);
    if (faceNormal.lengthSquared() < 1e-8f) {
        return false;
    }
    faceNormal = faceNormal.normalize();

    if (m_settings.backfaceCulling) {
        Vector3 viewDir = (cameraPos - p0).normalize();
        if (faceNormal.dot(viewDir) <= 0.0f) {
            return false;
        }
    }

    return true;
}

SoftwareRenderer::RasterDerivatives SoftwareRenderer::computeRasterDerivatives(
    const ScreenVertex& v0,
    const ScreenVertex& v1,
    const ScreenVertex& v2) const {
    float x0 = v0.screenX;
    float y0 = v0.screenY;
    float x1 = v1.screenX;
    float y1 = v1.screenY;
    float x2 = v2.screenX;
    float y2 = v2.screenY;

    float det = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
    if (std::fabs(det) < 1e-6f) {
        return {0.0f, 0.0f, 0.0f, 0.0f};
    }

    RasterDerivatives derivs{};
    Vector2 uv0 = v0.attributes.texCoord;
    Vector2 uv1 = v1.attributes.texCoord;
    Vector2 uv2 = v2.attributes.texCoord;

    derivs.dudx = ((uv1.x - uv0.x) * (y2 - y0) - (uv2.x - uv0.x) * (y1 - y0)) / det;
    derivs.dudy = (-(uv1.x - uv0.x) * (x2 - x0) + (uv2.x - uv0.x) * (x1 - x0)) / det;
    derivs.dvdx = ((uv1.y - uv0.y) * (y2 - y0) - (uv2.y - uv0.y) * (y1 - y0)) / det;
    derivs.dvdy = (-(uv1.y - uv0.y) * (x2 - x0) + (uv2.y - uv0.y) * (x1 - x0)) / det;
    return derivs;
}

void SoftwareRenderer::runRasterStage(const ScreenVertex& v0,
                                      const ScreenVertex& v1,
                                      const ScreenVertex& v2,
                                      Material* material,
                                      const std::vector<Renderer::Lighting::Light*>& lights,
                                      const Vector3& cameraPos,
                                      const Color& ambientLight,
                                      const RasterDerivatives& derivs) {
    float minX = std::floor(std::min({v0.screenX, v1.screenX, v2.screenX}));
    float maxX = std::ceil(std::max({v0.screenX, v1.screenX, v2.screenX}));
    float minY = std::floor(std::min({v0.screenY, v1.screenY, v2.screenY}));
    float maxY = std::ceil(std::max({v0.screenY, v1.screenY, v2.screenY}));

    int xStart = std::max(0, static_cast<int>(minX));
    int xEnd = std::min(m_settings.width - 1, static_cast<int>(maxX));
    int yStart = std::max(0, static_cast<int>(minY));
    int yEnd = std::min(m_settings.height - 1, static_cast<int>(maxY));

    float denom = (v1.screenY - v2.screenY) * (v0.screenX - v2.screenX) +
                  (v2.screenX - v1.screenX) * (v0.screenY - v2.screenY);
    if (std::fabs(denom) < 1e-6f) {
        return;
    }

    float w0 = v0.attributes.reciprocalW;
    float w1 = v1.attributes.reciprocalW;
    float w2 = v2.attributes.reciprocalW;

    for (int y = yStart; y <= yEnd; ++y) {
        for (int x = xStart; x <= xEnd; ++x) {
            float px = static_cast<float>(x) + 0.5f;
            float py = static_cast<float>(y) + 0.5f;

            float alpha = ((v1.screenY - v2.screenY) * (px - v2.screenX) +
                           (v2.screenX - v1.screenX) * (py - v2.screenY)) / denom;
            float beta = ((v2.screenY - v0.screenY) * (px - v2.screenX) +
                          (v0.screenX - v2.screenX) * (py - v2.screenY)) / denom;
            float gamma = 1.0f - alpha - beta;

            bool hasNeg = (alpha < 0.0f) || (beta < 0.0f) || (gamma < 0.0f);
            bool hasPos = (alpha > 0.0f) || (beta > 0.0f) || (gamma > 0.0f);
            if (hasNeg && hasPos) {
                continue;
            }

            float invZ = alpha * w0 + beta * w1 + gamma * w2;
            if (invZ <= 0.0f) {
                continue;
            }

            float depthNDC = alpha * v0.ndcZ + beta * v1.ndcZ + gamma * v2.ndcZ;
            if (!std::isfinite(depthNDC)) {
                continue;
            }
            float depth01 = depthNDC * 0.5f + 0.5f;

            if (!m_target.depthTestAndSet(x, y, depth01)) {
                continue;
            }

            GeometryVertex interpolated = GeometryVertex::interpolate(v0.attributes, v1.attributes, v2.attributes,
                                                                       alpha, beta, gamma, m_settings.perspectiveCorrect);

            Color shaded = runShadingStage(interpolated, material, lights, cameraPos, ambientLight, derivs);
            m_target.setPixel(x, y, shaded);
            // Track total fragments for debugging
        }
    }
}

Color SoftwareRenderer::runShadingStage(const GeometryVertex& interpolated,
                                        Material* material,
                                        const std::vector<Renderer::Lighting::Light*>& lights,
                                        const Vector3& viewPos,
                                        const Color& sceneAmbient,
                                        const RasterDerivatives& derivs) const {
    Color baseColor = interpolated.color;
    if (material) {
        Color albedo = material->sampleAlbedo(interpolated.texCoord, derivs.dudx, derivs.dudy, derivs.dvdx, derivs.dvdy);
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

    Color ambient = sceneAmbient * baseColor;
    Color finalColor = ambient;

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
        Color diffuse = baseColor * lightColor * NdotL;

        Vector3 halfVector = (lightDir + viewDir).normalize();
        float specPower = material ? material->getShininess() : 32.0f;
        float NdotH = std::max(0.0f, normal.dot(halfVector));
        float specularFactor = std::pow(NdotH, specPower);
        Color specularColor = material ? material->getSpecular() : Color(1.0f, 1.0f, 1.0f, 1.0f);
        Color specular = specularColor * lightColor * specularFactor;

        finalColor = finalColor + diffuse + specular;
    }

    finalColor.r = std::clamp(finalColor.r, 0.0f, 1.0f);
    finalColor.g = std::clamp(finalColor.g, 0.0f, 1.0f);
    finalColor.b = std::clamp(finalColor.b, 0.0f, 1.0f);
    finalColor.a = 1.0f;
    return finalColor;
}

} // namespace Pipeline
} // namespace Renderer
