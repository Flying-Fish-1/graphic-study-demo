#ifndef RENDERER_PIPELINE_SHADING_PIPELINE_H
#define RENDERER_PIPELINE_SHADING_PIPELINE_H

#include <vector>

#include "screen_vertex.h"
#include "../../core/types/color.h"

namespace Core {
namespace Types {
class Material;
}
namespace Math {
struct Vector3;
}
}

namespace Renderer {
namespace Lighting {
class Light;
}
namespace Pipeline {

struct SoftwareRendererSettings;

class ShadingPipeline {
public:
    explicit ShadingPipeline(const SoftwareRendererSettings& settings);

    Core::Types::Color shade(const GeometryVertex& interpolated,
                             Core::Types::Material* material,
                             const std::vector<Renderer::Lighting::Light*>& lights,
                             const Core::Math::Vector3& viewPos,
                             const Core::Types::Color& sceneAmbient,
                             const RasterDerivatives& derivs) const;

private:
    const SoftwareRendererSettings& m_settings;
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_SHADING_PIPELINE_H
