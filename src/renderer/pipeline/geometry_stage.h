#ifndef RENDERER_PIPELINE_GEOMETRY_STAGE_H
#define RENDERER_PIPELINE_GEOMETRY_STAGE_H

#include "../../core/math/vector.h"
#include "../../core/math/matrix.h"
#include "../../core/types/vertex.h"
#include "../../core/types/color.h"

namespace Renderer {
namespace Pipeline {

struct GeometryVertex {
    Core::Math::Vector4 clipPosition;
    Core::Math::Vector3 worldPosition;
    Core::Math::Vector3 normal;
    Core::Math::Vector3 tangent;
    Core::Math::Vector3 bitangent;
    Core::Math::Vector2 texCoord;
    Core::Types::Color color;
    float reciprocalW;
    float ndcZ;

    static GeometryVertex fromVertex(const Core::Types::Vertex& vertex,
                                     const Core::Math::Matrix4& modelMatrix,
                                     const Core::Math::Matrix4& viewMatrix,
                                     const Core::Math::Matrix4& projectionMatrix);

    static GeometryVertex interpolate(const GeometryVertex& v0,
                                      const GeometryVertex& v1,
                                      const GeometryVertex& v2,
                                      float u, float v, float w,
                                      bool perspectiveCorrect = true);
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_GEOMETRY_STAGE_H
