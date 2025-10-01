#include "geometry_stage.h"
#include <cmath>

namespace Renderer {
namespace Pipeline {

GeometryVertex GeometryVertex::fromVertex(const Core::Types::Vertex& vertex,
                                          const Core::Math::Matrix4& modelMatrix,
                                          const Core::Math::Matrix4& viewMatrix,
                                          const Core::Math::Matrix4& projectionMatrix) {
    GeometryVertex out{};

    Core::Math::Vector4 worldPos4 = modelMatrix * Core::Math::Vector4(vertex.position, 1.0f);
    out.worldPosition = Core::Math::Vector3(worldPos4.x, worldPos4.y, worldPos4.z);

    Core::Math::Vector4 viewPos = viewMatrix * worldPos4;
    out.clipPosition = projectionMatrix * viewPos;

    out.reciprocalW = (std::fabs(out.clipPosition.w) > 1e-6f) ? (1.0f / out.clipPosition.w) : 0.0f;
    out.ndcZ = out.reciprocalW != 0.0f ? out.clipPosition.z * out.reciprocalW : 0.0f;

    out.normal = modelMatrix.transformDirection(vertex.normal).normalize();
    out.tangent = modelMatrix.transformDirection(vertex.tangent).normalize();
    out.bitangent = modelMatrix.transformDirection(vertex.bitangent).normalize();

    out.texCoord = vertex.texCoord;
    out.color = vertex.color;

    return out;
}

GeometryVertex GeometryVertex::interpolate(const GeometryVertex& v0,
                                            const GeometryVertex& v1,
                                            const GeometryVertex& v2,
                                            float u, float v, float w,
                                            bool perspectiveCorrect) {
    float alpha = u;
    float beta = v;
    float gamma = w;

    if (perspectiveCorrect) {
        float w0 = v0.reciprocalW;
        float w1 = v1.reciprocalW;
        float w2 = v2.reciprocalW;
        float denom = alpha * w0 + beta * w1 + gamma * w2;
        if (std::fabs(denom) > 1e-8f) {
            alpha = (alpha * w0) / denom;
            beta = (beta * w1) / denom;
            gamma = (gamma * w2) / denom;
        }
    }

    GeometryVertex out{};
    out.worldPosition = v0.worldPosition * alpha + v1.worldPosition * beta + v2.worldPosition * gamma;
    out.normal = (v0.normal * alpha + v1.normal * beta + v2.normal * gamma).normalize();
    out.tangent = (v0.tangent * alpha + v1.tangent * beta + v2.tangent * gamma).normalize();
    out.bitangent = (v0.bitangent * alpha + v1.bitangent * beta + v2.bitangent * gamma).normalize();
    out.texCoord = v0.texCoord * alpha + v1.texCoord * beta + v2.texCoord * gamma;
    out.color = v0.color * alpha + v1.color * beta + v2.color * gamma;
    out.clipPosition = v0.clipPosition * alpha + v1.clipPosition * beta + v2.clipPosition * gamma;
    out.reciprocalW = v0.reciprocalW * alpha + v1.reciprocalW * beta + v2.reciprocalW * gamma;
    out.ndcZ = v0.ndcZ * alpha + v1.ndcZ * beta + v2.ndcZ * gamma;
    return out;
}

} // namespace Pipeline
} // namespace Renderer
