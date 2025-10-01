#include "software_renderer.h"

#include <algorithm>
#include <cmath>

#include "geometry_processor.h"
#include "render_queue.h"
#include "shading_pipeline.h"
#include "triangle_rasterizer.h"
#include "screen_vertex.h"
#include "../effects/ssaa.h"
#include "../../core/types/material.h"
#include "../../renderer/lighting/light.h"

namespace Renderer {
namespace Pipeline {

using Core::Math::Vector2;
using Core::Math::Vector3;
using Core::Math::Vector4;
using Core::Math::Matrix4;
using Core::Types::Color;

namespace {

RasterDerivatives computeRasterDerivatives(const ScreenVertex& v0,
                                           const ScreenVertex& v1,
                                           const ScreenVertex& v2) {
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

bool assemblePrimitive(const SoftwareRendererSettings& settings,
                       const ScreenVertex& v0,
                       const ScreenVertex& v1,
                       const ScreenVertex& v2,
                       const Vector3& cameraPos,
                       Vector3& faceNormal,
                       bool applyCulling) {
    Vector3 p0 = v0.attributes.worldPosition;
    Vector3 p1 = v1.attributes.worldPosition;
    Vector3 p2 = v2.attributes.worldPosition;
    faceNormal = (p1 - p0).cross(p2 - p0);
    if (faceNormal.lengthSquared() < 1e-8f) {
        return false;
    }
    faceNormal = faceNormal.normalize();

    if (applyCulling && settings.backfaceCulling) {
        Vector3 viewDir = (cameraPos - p0).normalize();
        if (faceNormal.dot(viewDir) <= 0.0f) {
            return false;
        }
    }

    return true;
}

} // namespace

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

    GeometryProcessor geometryProcessor(m_settings);
    ShadingPipeline shadingPipeline(m_settings);
    RenderQueue renderQueue;
    TriangleRasterizer rasterizer(m_target, m_settings);

    for (const auto& object : objects) {
        if (!object.visible || !object.mesh) {
            continue;
        }

        Scene::Mesh* mesh = object.mesh;
        Core::Types::Material* material = object.materialOverride ? object.materialOverride : mesh->getMaterial();
        const auto& vertices = mesh->getVertices();
        const auto& indices = mesh->getIndices();
        if (vertices.empty() || indices.size() < 3) {
            continue;
        }

        std::vector<ScreenVertex> transformed = geometryProcessor.process(object, viewMatrix, projectionMatrix);

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

            if (!assemblePrimitive(m_settings, v0, v1, v2, cameraPosition, faceNormal, effectiveAlpha >= 0.999f)) {
                continue;
            }

            RasterDerivatives derivs = computeRasterDerivatives(v0, v1, v2);
            if (!std::isfinite(derivs.dudx) || !std::isfinite(derivs.dudy) ||
                !std::isfinite(derivs.dvdx) || !std::isfinite(derivs.dvdy)) {
                continue;
            }

            float depthKey = (v0.ndcZ + v1.ndcZ + v2.ndcZ) / 3.0f;

            TriangleWorkItem item{};
            item.v0 = v0;
            item.v1 = v1;
            item.v2 = v2;
            item.material = material;
            item.derivs = derivs;
            item.depthKey = depthKey;

            if (effectiveAlpha >= 0.999f) {
                renderQueue.addOpaque(item);
            } else {
                renderQueue.addTransparent(item);
            }
        }
    }

    renderQueue.finalize();

    for (const auto& tri : renderQueue.getOpaque()) {
        rasterizer.rasterize(tri,
                             tri.material,
                             lights,
                             cameraPosition,
                             scene.getAmbientLight(),
                             shadingPipeline);
    }

    for (const auto& tri : renderQueue.getTransparent()) {
        rasterizer.rasterize(tri,
                             tri.material,
                             lights,
                             cameraPosition,
                             scene.getAmbientLight(),
                             shadingPipeline);
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

} // namespace Pipeline
} // namespace Renderer
