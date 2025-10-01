#ifndef RENDERER_PIPELINE_TRIANGLE_RASTERIZER_H
#define RENDERER_PIPELINE_TRIANGLE_RASTERIZER_H

#include <vector>

#include "render_queue.h"

#include "../../core/types/color.h"

namespace Renderer {
namespace Lighting {
class Light;
}
namespace Pipeline {

class ShadingPipeline;
class RenderTarget;
struct SoftwareRendererSettings;

class TriangleRasterizer {
public:
    TriangleRasterizer(RenderTarget& target, const SoftwareRendererSettings& settings);

    void rasterize(const TriangleWorkItem& tri,
                   Core::Types::Material* material,
                   const std::vector<Renderer::Lighting::Light*>& lights,
                   const Core::Math::Vector3& cameraPos,
                   const Core::Types::Color& ambientLight,
                   const ShadingPipeline& shading) const;

private:
    RenderTarget& m_target;
    const SoftwareRendererSettings& m_settings;
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_TRIANGLE_RASTERIZER_H
