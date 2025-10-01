#include "triangle.h"
#include <algorithm>
#include <cmath>

namespace Core {
namespace Types {

Triangle::Triangle() : faceNormal(0.0f, 1.0f, 0.0f), depth(0.0f), material(nullptr) {}

Triangle::Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
    : faceNormal(0.0f, 1.0f, 0.0f), depth(0.0f), material(nullptr) {
    vertices[0] = v0;
    vertices[1] = v1;
    vertices[2] = v2;

    Vector3 edge1 = vertices[1].position - vertices[0].position;
    Vector3 edge2 = vertices[2].position - vertices[0].position;
    faceNormal = edge1.cross(edge2).normalize();

    depth = (vertices[0].position.z + vertices[1].position.z + vertices[2].position.z) / 3.0f;
}

bool Triangle::isBackface(const Vector3& viewDir) const {
    return faceNormal.dot(viewDir) <= 0.0f;
}

void Triangle::getBoundingBox(int& minX, int& minY, int& maxX, int& maxY) const {
    minX = static_cast<int>(std::floor(std::min({vertices[0].position.x, vertices[1].position.x, vertices[2].position.x})));
    minY = static_cast<int>(std::floor(std::min({vertices[0].position.y, vertices[1].position.y, vertices[2].position.y})));
    maxX = static_cast<int>(std::ceil(std::max({vertices[0].position.x, vertices[1].position.x, vertices[2].position.x})));
    maxY = static_cast<int>(std::ceil(std::max({vertices[0].position.y, vertices[1].position.y, vertices[2].position.y})));
}

bool Triangle::getBarycentricCoords(const Vector2& point, float& u, float& v, float& w) const {
    Vector2 a(vertices[0].position.x, vertices[0].position.y);
    Vector2 b(vertices[1].position.x, vertices[1].position.y);
    Vector2 c(vertices[2].position.x, vertices[2].position.y);

    Vector2 v0 = b - a;
    Vector2 v1 = c - a;
    Vector2 v2 = point - a;

    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);

    float denom = d00 * d11 - d01 * d01;
    if (std::abs(denom) < 1e-8f) {
        u = v = w = 0.0f;
        return false;
    }

    float invDenom = 1.0f / denom;
    v = (d11 * d20 - d01 * d21) * invDenom;
    w = (d00 * d21 - d01 * d20) * invDenom;
    u = 1.0f - v - w;

    return (u >= 0.0f) && (v >= 0.0f) && (w >= 0.0f);
}

float Triangle::getArea() const {
    Vector3 edge1 = vertices[1].position - vertices[0].position;
    Vector3 edge2 = vertices[2].position - vertices[0].position;
    return edge1.cross(edge2).length() * 0.5f;
}

} // namespace Types
} // namespace Core
