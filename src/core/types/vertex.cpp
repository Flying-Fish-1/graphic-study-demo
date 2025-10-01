#include "vertex.h"

namespace Core {
namespace Types {

Vertex::Vertex()
    : position(0.0f, 0.0f, 0.0f),
      normal(0.0f, 0.0f, 1.0f),
      tangent(1.0f, 0.0f, 0.0f),
      bitangent(0.0f, 1.0f, 0.0f),
      texCoord(0.0f, 0.0f),
      color(Color::WHITE) {}

Vertex::Vertex(const Vector3& pos,
               const Vector3& norm,
               const Vector2& uv,
               const Color& col,
               const Vector3& tan,
               const Vector3& bitan)
    : position(pos),
      normal(norm),
      tangent(tan),
      bitangent(bitan),
      texCoord(uv),
      color(col) {}

void Vertex::applyTransform(const Matrix4& transform) {
    Vector4 transformed = transform * Vector4(position, 1.0f);
    position = Vector3(transformed.x, transformed.y, transformed.z);
}

void Vertex::applyNormalTransform(const Matrix3& transform) {
    normal = (transform * normal).normalize();
    tangent = (transform * tangent).normalize();
    bitangent = (transform * bitangent).normalize();
}

Vertex Vertex::interpolate(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                           float u, float v, float w) {
    Vertex result;
    result.position = v0.position * w + v1.position * v + v2.position * u;
    result.normal = (v0.normal * w + v1.normal * v + v2.normal * u).normalize();
    result.tangent = (v0.tangent * w + v1.tangent * v + v2.tangent * u).normalize();
    result.bitangent = (v0.bitangent * w + v1.bitangent * v + v2.bitangent * u).normalize();
    result.texCoord = v0.texCoord * w + v1.texCoord * v + v2.texCoord * u;
    result.color = v0.color * w + v1.color * v + v2.color * u;
    return result;
}

} // namespace Types
} // namespace Core
