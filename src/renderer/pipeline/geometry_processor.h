#ifndef RENDERER_PIPELINE_GEOMETRY_PROCESSOR_H
#define RENDERER_PIPELINE_GEOMETRY_PROCESSOR_H

#include <vector>

#include "screen_vertex.h"
#include "../../core/math/matrix.h"

namespace Scene {
struct SceneObject;
}

namespace Renderer {
namespace Pipeline {

struct SoftwareRendererSettings;

class GeometryProcessor {
public:
    explicit GeometryProcessor(const SoftwareRendererSettings& settings);

    std::vector<ScreenVertex> process(const Scene::SceneObject& object,
                                      const Core::Math::Matrix4& viewMatrix,
                                      const Core::Math::Matrix4& projectionMatrix) const;

private:
    const SoftwareRendererSettings& m_settings;
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_GEOMETRY_PROCESSOR_H
