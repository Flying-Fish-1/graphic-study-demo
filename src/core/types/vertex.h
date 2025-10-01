#ifndef CORE_TYPES_VERTEX_H
#define CORE_TYPES_VERTEX_H

#include "../math/vector.h"
#include "../math/matrix.h"
#include "color.h"

namespace Core {
namespace Types {

using namespace Core::Math;

/**
 * @brief Object-space vertex attributes.
 */
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
    Vector2 texCoord;
    Color color;

    Vertex();
    Vertex(const Vector3& pos,
           const Vector3& norm,
           const Vector2& uv,
           const Color& col,
           const Vector3& tan = Vector3(1.0f, 0.0f, 0.0f),
           const Vector3& bitan = Vector3(0.0f, 1.0f, 0.0f));

    void applyTransform(const Matrix4& transform);
    void applyNormalTransform(const Matrix3& transform);

    static Vertex interpolate(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                              float u, float v, float w);
};

} // namespace Types
} // namespace Core

#endif // CORE_TYPES_VERTEX_H
