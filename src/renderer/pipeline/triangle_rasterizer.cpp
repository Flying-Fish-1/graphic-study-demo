#include "triangle_rasterizer.h"

#include <algorithm>
#include <cmath>

#include "render_target.h"
#include "shading_pipeline.h"
#include "geometry_stage.h"
#include "software_renderer.h"

namespace Renderer {
namespace Pipeline {

TriangleRasterizer::TriangleRasterizer(RenderTarget& target, const SoftwareRendererSettings& settings)
    : m_target(target), m_settings(settings) {}

void TriangleRasterizer::rasterize(const TriangleWorkItem& tri,
                                   Core::Types::Material* material,
                                   const std::vector<Renderer::Lighting::Light*>& lights,
                                   const Core::Math::Vector3& cameraPos,
                                   const Core::Types::Color& ambientLight,
                                   const ShadingPipeline& shading) const {
    const ScreenVertex& v0 = tri.v0;
    const ScreenVertex& v1 = tri.v1;
    const ScreenVertex& v2 = tri.v2;

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

    const float dAlphaDx = (v1.screenY - v2.screenY) / denom;
    const float dBetaDx  = (v2.screenY - v0.screenY) / denom;

    for (int y = yStart; y <= yEnd; ++y) {
        float py = static_cast<float>(y) + 0.5f;
        float alpha = ((v1.screenY - v2.screenY) * ((static_cast<float>(xStart) + 0.5f) - v2.screenX) +
                       (v2.screenX - v1.screenX) * (py - v2.screenY)) / denom;
        float beta  = ((v2.screenY - v0.screenY) * ((static_cast<float>(xStart) + 0.5f) - v2.screenX) +
                       (v0.screenX - v2.screenX) * (py - v2.screenY)) / denom;
        float gamma = 1.0f - alpha - beta;

        for (int x = xStart; x <= xEnd; ++x) {
            bool hasNeg = (alpha < 0.0f) || (beta < 0.0f) || (gamma < 0.0f);
            bool hasPos = (alpha > 0.0f) || (beta > 0.0f) || (gamma > 0.0f);
            if (!(hasNeg && hasPos)) {
                float invZ = alpha * w0 + beta * w1 + gamma * w2;
                if (invZ > 0.0f) {
                    float depthNDC = alpha * v0.ndcZ + beta * v1.ndcZ + gamma * v2.ndcZ;
                    if (std::isfinite(depthNDC)) {
                        float depth01 = depthNDC * 0.5f + 0.5f;
                        if (m_target.depthPasses(x, y, depth01)) {
                            GeometryVertex interpolated = GeometryVertex::interpolate(
                                v0.attributes, v1.attributes, v2.attributes,
                                alpha, beta, gamma, m_settings.perspectiveCorrect);

                            Core::Types::Color shaded = shading.shade(interpolated,
                                                                       material,
                                                                       lights,
                                                                       cameraPos,
                                                                       ambientLight,
                                                                       tri.derivs);
                            Core::Types::Color dst = m_target.getPixel(x, y);
                            float srcA = std::clamp(shaded.a, 0.0f, 1.0f);
                            Core::Types::Color out(
                                shaded.r + dst.r * (1.0f - srcA),
                                shaded.g + dst.g * (1.0f - srcA),
                                shaded.b + dst.b * (1.0f - srcA),
                                srcA + dst.a * (1.0f - srcA)
                            );
                            m_target.setPixel(x, y, out);
                            if (srcA >= 0.999f) {
                                m_target.setDepth(x, y, depth01);
                            }
                        }
                    }
                }
            }

            alpha += dAlphaDx;
            beta  += dBetaDx;
            gamma = 1.0f - alpha - beta;
        }
    }
}

} // namespace Pipeline
} // namespace Renderer
