#include "geometry_processor.h"

#include "screen_vertex.h"
#include "software_renderer.h"

#include "../../scene/scene.h"
#include "../../scene/mesh.h"

namespace Renderer {
namespace Pipeline {

GeometryProcessor::GeometryProcessor(const SoftwareRendererSettings& settings)
    : m_settings(settings) {}

std::vector<ScreenVertex> GeometryProcessor::process(const Scene::SceneObject& object,
                                                     const Core::Math::Matrix4& viewMatrix,
                                                     const Core::Math::Matrix4& projectionMatrix) const {
    const auto& vertices = object.mesh->getVertices();
    std::vector<ScreenVertex> results(vertices.size());

    for (std::size_t i = 0; i < vertices.size(); ++i) {
        GeometryVertex geomVertex = GeometryVertex::fromVertex(vertices[i],
                                                               object.transform,
                                                               viewMatrix,
                                                               projectionMatrix);

        float w = geomVertex.clipPosition.w;
        if (w <= 1e-6f) {
            ScreenVertex invalid{};
            invalid.valid = false;
            results[i] = invalid;
            continue;
        }

        float invW = 1.0f / w;
        Core::Math::Vector3 ndc(geomVertex.clipPosition.x * invW,
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

} // namespace Pipeline
} // namespace Renderer
