#include "software_renderer.h"
#include <algorithm>
#include <cmath>
#include "../effects/ssaa.h"

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

    // 处理 SSAA：当 ssaaFactor > 1 时，临时使用更高分辨率渲染
    const int ssaaFactor = std::max(1, m_settings.ssaaFactor);
    const int baseWidth = m_settings.width;
    const int baseHeight = m_settings.height;
    if (ssaaFactor > 1) {
        // 切换到高分辨率设置
        m_settings.width = baseWidth * ssaaFactor;
        m_settings.height = baseHeight * ssaaFactor;
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

    struct TransparentTri {
        ScreenVertex v0, v1, v2;
        Material* material;
        RasterDerivatives derivs;
        float depthKey; // 平均NDC深度，远->近排序用
    };
    std::vector<TransparentTri> transparentTris;

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
            // 先根据顶点与材质alpha判断是否透明，再决定是否进行背面剔除
            float triVertexAlpha = (v0.attributes.color.a + v1.attributes.color.a + v2.attributes.color.a) / 3.0f;
            float triMaterialAlpha = material ? material->getDiffuse().a : 1.0f;
            float effectiveAlpha = triVertexAlpha * triMaterialAlpha;

            if (!runPrimitiveAssembly(v0, v1, v2, cameraPosition, faceNormal, effectiveAlpha >= 0.999f)) {
                continue;
            }

            RasterDerivatives derivs = computeRasterDerivatives(v0, v1, v2);
            if (!std::isfinite(derivs.dudx) || !std::isfinite(derivs.dudy) ||
                !std::isfinite(derivs.dvdx) || !std::isfinite(derivs.dvdy)) {
                continue;
            }

            // 基于三角形有效alpha判断透明/不透明
            if (effectiveAlpha >= 0.999f) {
                // 不透明：直接渲染（深度测试+写入由 runRasterStage 内处理）
                runRasterStage(v0, v1, v2, material, lights, cameraPosition, scene.getAmbientLight(), derivs);
            } else {
                // 半透明：收集，稍后按距离排序后再渲染
                float depthKey = (v0.ndcZ + v1.ndcZ + v2.ndcZ) / 3.0f;
                transparentTris.push_back(TransparentTri{v0, v1, v2, material, derivs, depthKey});
            }
        }
    }

    // 二阶段：半透明三角形按远->近排序后渲染（深度测试开，深度写入关由 runRasterStage 内阈值控制）
    std::sort(transparentTris.begin(), transparentTris.end(), [](const TransparentTri& a, const TransparentTri& b) {
        return a.depthKey > b.depthKey; // NDC 1远 0近：从大到小（远到近）
    });
    for (const auto& t : transparentTris) {
        runRasterStage(t.v0, t.v1, t.v2, t.material, lights, cameraPosition, scene.getAmbientLight(), t.derivs);
    }

    // 若使用了 SSAA，则在渲染后进行低通+下采样回基准分辨率
    if (ssaaFactor > 1) {
        Renderer::Pipeline::RenderTarget lowRes(baseWidth, baseHeight);
        lowRes.clear(scene.getBackgroundColor(), 1.0f);
        Renderer::Effects::resolveBox(m_target, lowRes, ssaaFactor);
        // 覆盖为低分辨率结果
        m_target = lowRes;
        // 恢复设置
        m_settings.width = baseWidth;
        m_settings.height = baseHeight;
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
                                            Vector3& faceNormal,
                                            bool applyCulling) const {
    Vector3 p0 = v0.attributes.worldPosition;
    Vector3 p1 = v1.attributes.worldPosition;
    Vector3 p2 = v2.attributes.worldPosition;
    faceNormal = (p1 - p0).cross(p2 - p0);
    if (faceNormal.lengthSquared() < 1e-8f) {
        return false;
    }
    faceNormal = faceNormal.normalize();

    if (applyCulling && m_settings.backfaceCulling) {
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

            // 先仅做深度测试，不立即写入。透明片元不写深度，避免遮挡后续片元；
            // 不透明片元再写深度。
            if (!m_target.depthPasses(x, y, depth01)) {
                continue;
            }

            GeometryVertex interpolated = GeometryVertex::interpolate(v0.attributes, v1.attributes, v2.attributes,
                                                                       alpha, beta, gamma, m_settings.perspectiveCorrect);

            Color shaded = runShadingStage(interpolated, material, lights, cameraPos, ambientLight, derivs);
            // 预乘Alpha混合：out = src + dst * (1 - src.a)
            Color dst = m_target.getPixel(x, y);
            float srcA = std::clamp(shaded.a, 0.0f, 1.0f);
            Color out(
                shaded.r + dst.r * (1.0f - srcA),
                shaded.g + dst.g * (1.0f - srcA),
                shaded.b + dst.b * (1.0f - srcA),
                srcA + dst.a * (1.0f - srcA)
            );
            m_target.setPixel(x, y, out);
            // 仅当片元基本不透明时写入深度（简单阈值）
            if (srcA >= 0.999f) {
                m_target.setDepth(x, y, depth01);
            }
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

    // 取来自反照率的透明度（如使用纹理，则来自纹理A通道）
    finalColor.a = std::clamp(baseColor.a, 0.0f, 1.0f);
    // 预乘Alpha
    finalColor.r = std::clamp(finalColor.r, 0.0f, 1.0f) * finalColor.a;
    finalColor.g = std::clamp(finalColor.g, 0.0f, 1.0f) * finalColor.a;
    finalColor.b = std::clamp(finalColor.b, 0.0f, 1.0f) * finalColor.a;
    return finalColor;
}

} // namespace Pipeline
} // namespace Renderer
