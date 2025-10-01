#include "mesh.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace Scene {

using Core::Math::Constants::PI;
using Core::Math::Constants::TAU;

BoundingBox::BoundingBox()
    : min(Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max())),
      max(Vector3(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest())) {}

BoundingBox::BoundingBox(const Vector3& minPoint, const Vector3& maxPoint)
    : min(minPoint), max(maxPoint) {}

bool BoundingBox::contains(const Vector3& point) const {
    return point.x >= min.x && point.x <= max.x &&
           point.y >= min.y && point.y <= max.y &&
           point.z >= min.z && point.z <= max.z;
}

bool BoundingBox::intersects(const BoundingBox& other) const {
    return !(other.min.x > max.x || other.max.x < min.x ||
             other.min.y > max.y || other.max.y < min.y ||
             other.min.z > max.z || other.max.z < min.z);
}

Vector3 BoundingBox::getCenter() const {
    return (min + max) * 0.5f;
}

Vector3 BoundingBox::getSize() const {
    return max - min;
}

void BoundingBox::expand(const Vector3& point) {
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    min.z = std::min(min.z, point.z);

    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
    max.z = std::max(max.z, point.z);
}

void BoundingBox::reset() {
    min = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    max = Vector3(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
}

Mesh::Mesh() : m_material(nullptr) {
    m_bounds.reset();
}

int Mesh::addVertex(const Vertex& vertex) {
    m_vertices.push_back(vertex);
    m_bounds.expand(vertex.position);
    return static_cast<int>(m_vertices.size() - 1);
}

void Mesh::addTriangle(uint32_t i0, uint32_t i1, uint32_t i2) {
    m_indices.push_back(i0);
    m_indices.push_back(i1);
    m_indices.push_back(i2);
}

void Mesh::calculateBoundingBox() {
    m_bounds.reset();
    for (const auto& vertex : m_vertices) {
        m_bounds.expand(vertex.position);
    }
}

void Mesh::calculateFaceNormals() {
    if (m_vertices.empty() || m_indices.empty()) return;
    for (auto& vertex : m_vertices) {
        vertex.normal = Vector3(0.0f, 0.0f, 0.0f);
    }

    for (std::size_t i = 0; i + 2 < m_indices.size(); i += 3) {
        Vertex& v0 = m_vertices[m_indices[i]];
        Vertex& v1 = m_vertices[m_indices[i + 1]];
        Vertex& v2 = m_vertices[m_indices[i + 2]];

        Vector3 edge1 = v1.position - v0.position;
        Vector3 edge2 = v2.position - v0.position;
        Vector3 normal = edge1.cross(edge2);

        v0.normal = v0.normal + normal;
        v1.normal = v1.normal + normal;
        v2.normal = v2.normal + normal;
    }

    for (auto& vertex : m_vertices) {
        if (vertex.normal.lengthSquared() > 0.0f) {
            vertex.normal = vertex.normal.normalize();
        } else {
            vertex.normal = Vector3(0.0f, 0.0f, 1.0f);
        }
    }
}

void Mesh::calculateVertexNormals() {
    calculateFaceNormals();
}

void Mesh::ensureTangents() {
    if (m_vertices.empty() || m_indices.empty()) return;

    for (auto& vertex : m_vertices) {
        vertex.tangent = Vector3(1.0f, 0.0f, 0.0f);
        vertex.bitangent = Vector3(0.0f, 1.0f, 0.0f);
    }

    for (std::size_t i = 0; i + 2 < m_indices.size(); i += 3) {
        Vertex& v0 = m_vertices[m_indices[i]];
        Vertex& v1 = m_vertices[m_indices[i + 1]];
        Vertex& v2 = m_vertices[m_indices[i + 2]];

        Vector3 deltaPos1 = v1.position - v0.position;
        Vector3 deltaPos2 = v2.position - v0.position;
        Vector2 deltaUV1 = v1.texCoord - v0.texCoord;
        Vector2 deltaUV2 = v2.texCoord - v0.texCoord;

        float denom = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
        if (std::fabs(denom) < 1e-6f) {
            continue;
        }
        float r = 1.0f / denom;
        Vector3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        Vector3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

        v0.tangent = (v0.tangent + tangent).normalize();
        v1.tangent = (v1.tangent + tangent).normalize();
        v2.tangent = (v2.tangent + tangent).normalize();

        v0.bitangent = (v0.bitangent + bitangent).normalize();
        v1.bitangent = (v1.bitangent + bitangent).normalize();
        v2.bitangent = (v2.bitangent + bitangent).normalize();
    }
}

Mesh* Mesh::createCube(float size) {
    Mesh* mesh = new Mesh();
    float half = size * 0.5f;

    std::vector<Vector3> positions = {
        {-half, -half, -half}, {half, -half, -half}, {half, half, -half}, {-half, half, -half},
        {-half, -half, half},  {half, -half, half},  {half, half, half},  {-half, half, half}
    };

    std::vector<Vector3> normals = {
        {0, 0, -1}, {0, 0, 1}, {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}
    };

    std::vector<Vector2> uvs = {
        {0, 0}, {1, 0}, {1, 1}, {0, 1}
    };

    struct Face { int a, b, c, d; Vector3 normal; Color color; };
    std::vector<Face> faces = {
        {1, 0, 3, 2, normals[0], Color(1.0f, 0.2f, 0.2f)}, // back  - red
        {4, 5, 6, 7, normals[1], Color(0.2f, 1.0f, 0.2f)}, // front - green
        {4, 0, 3, 7, normals[2], Color(0.2f, 0.2f, 1.0f)}, // left  - blue
        {5, 1, 2, 6, normals[3], Color(1.0f, 1.0f, 0.2f)}, // right - yellow
        {3, 7, 6, 2, normals[4], Color(1.0f, 0.5f, 0.2f)}, // top   - orange
        {0, 1, 5, 4, normals[5], Color(0.2f, 1.0f, 1.0f)}  // bottom- cyan
    };

    for (const auto& face : faces) {
        Vector3 tangent(1.0f, 0.0f, 0.0f);
        Vector3 bitangent(0.0f, 1.0f, 0.0f);
        int baseIndex = static_cast<int>(mesh->m_vertices.size());

        mesh->m_vertices.emplace_back(positions[face.a], face.normal, uvs[0], face.color, tangent, bitangent);
        mesh->m_vertices.emplace_back(positions[face.b], face.normal, uvs[1], face.color, tangent, bitangent);
        mesh->m_vertices.emplace_back(positions[face.c], face.normal, uvs[2], face.color, tangent, bitangent);
        mesh->m_vertices.emplace_back(positions[face.d], face.normal, uvs[3], face.color, tangent, bitangent);

        mesh->addTriangle(baseIndex + 0, baseIndex + 1, baseIndex + 2);
        mesh->addTriangle(baseIndex + 0, baseIndex + 2, baseIndex + 3);
    }

    mesh->calculateBoundingBox();
    mesh->calculateFaceNormals();
    mesh->ensureTangents();
    return mesh;
}

Mesh* Mesh::createQuad(float width, float height) {
    Mesh* mesh = new Mesh();
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    mesh->m_vertices.emplace_back(Vector3(-hw, -hh, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 1.0f), Color::WHITE);
    mesh->m_vertices.emplace_back(Vector3(hw, -hh, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f), Color::WHITE);
    mesh->m_vertices.emplace_back(Vector3(hw, hh, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 0.0f), Color::WHITE);
    mesh->m_vertices.emplace_back(Vector3(-hw, hh, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 0.0f), Color::WHITE);

    mesh->addTriangle(0, 1, 2);
    mesh->addTriangle(0, 2, 3);

    mesh->calculateBoundingBox();
    mesh->calculateFaceNormals();
    mesh->ensureTangents();
    return mesh;
}

Mesh* Mesh::createTriangle(float size) {
    Mesh* mesh = new Mesh();
    float half = size * 0.5f;

    mesh->m_vertices.emplace_back(Vector3(0.0f, half, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.5f, 0.0f), Color::WHITE);
    mesh->m_vertices.emplace_back(Vector3(-half, -half, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 1.0f), Color::WHITE);
    mesh->m_vertices.emplace_back(Vector3(half, -half, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f), Color::WHITE);

    mesh->addTriangle(0, 1, 2);
    mesh->calculateBoundingBox();
    mesh->calculateFaceNormals();
    mesh->ensureTangents();
    return mesh;
}

Mesh* Mesh::createSphere(float radius, int segments) {
    Mesh* mesh = new Mesh();
    int rings = std::max(1, segments);

    for (int y = 0; y <= rings; ++y) {
        float v = static_cast<float>(y) / static_cast<float>(rings);
        float theta = v * PI;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int x = 0; x <= segments; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(segments);
            float phi = u * TAU;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            Vector3 normal(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
            Vector3 position = normal * radius;
            mesh->m_vertices.emplace_back(position, normal, Vector2(u, 1.0f - v), Color::WHITE);
        }
    }

    int columns = segments + 1;
    for (int y = 0; y < rings; ++y) {
        for (int x = 0; x < segments; ++x) {
            int i0 = y * columns + x;
            int i1 = i0 + 1;
            int i2 = i0 + columns;
            int i3 = i2 + 1;

            mesh->addTriangle(i0, i2, i1);
            mesh->addTriangle(i1, i2, i3);
        }
    }

    mesh->calculateBoundingBox();
    mesh->calculateFaceNormals();
    mesh->ensureTangents();
    return mesh;
}

Mesh* Mesh::createPlane(float width, float height, int subdivisions) {
    Mesh* mesh = new Mesh();
    int steps = std::max(1, subdivisions);

    for (int y = 0; y <= steps; ++y) {
        float fy = static_cast<float>(y) / static_cast<float>(steps);
        float posY = height * (fy - 0.5f);
        for (int x = 0; x <= steps; ++x) {
            float fx = static_cast<float>(x) / static_cast<float>(steps);
            float posX = width * (fx - 0.5f);
            mesh->m_vertices.emplace_back(Vector3(posX, posY, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector2(fx, 1.0f - fy), Color::WHITE);
        }
    }

    int stride = steps + 1;
    for (int y = 0; y < steps; ++y) {
        for (int x = 0; x < steps; ++x) {
            int i0 = y * stride + x;
            int i1 = i0 + 1;
            int i2 = i0 + stride;
            int i3 = i2 + 1;

            mesh->addTriangle(i0, i2, i1);
            mesh->addTriangle(i1, i2, i3);
        }
    }

    mesh->calculateBoundingBox();
    mesh->calculateFaceNormals();
    mesh->ensureTangents();
    return mesh;
}

Mesh* Mesh::loadFromFile(const std::string& /*filename*/) {
    return createQuad();
}

} // namespace Scene
