#include "mesh.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <array>

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
        {1, 0, 3, 2, normals[0], Color(1.0f, 0.2f, 0.2f, 0.6f)}, // back  - red, semi-transparent
        {4, 5, 6, 7, normals[1], Color(0.2f, 1.0f, 0.8f, 0.6f)}, // front - green, semi-transparent
        {4, 0, 3, 7, normals[2], Color(0.2f, 0.2f, 1.0f, 0.6f)}, // left  - blue, semi-transparent
        {5, 1, 2, 6, normals[3], Color(1.0f, 1.0f, 0.2f, 0.6f)}, // right - yellow, semi-transparent
        {3, 7, 6, 2, normals[4], Color(1.0f, 0.5f, 0.2f, 0.6f)}, // top   - orange, semi-transparent
        {0, 1, 5, 4, normals[5], Color(0.2f, 1.0f, 1.0f, 0.6f)}  // bottom- cyan, semi-transparent
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

static void emitCubeFaces(Mesh* mesh, float half, const std::vector<Color>& faceColors, bool inwardNormals) {
    std::vector<Vector3> positions = {
        {-half, -half, -half}, {half, -half, -half}, {half, half, -half}, {-half, half, -half},
        {-half, -half, half},  {half, -half, half},  {half, half, half},  {-half, half, half}
    };

    std::vector<Vector3> normals = {
        {0, 0, -1}, {0, 0, 1}, {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}
    };
    if (inwardNormals) {
        for (auto& n : normals) n = n * -1.0f;
    }

    std::vector<Vector2> uvs = {{0,0},{1,0},{1,1},{0,1}};
    struct Face { int a,b,c,d; int n; };
    std::vector<Face> faces = {
        {1,0,3,2,0}, // back
        {4,5,6,7,1}, // front
        {4,0,3,7,2}, // left
        {5,1,2,6,3}, // right
        {3,7,6,2,4}, // top
        {0,1,5,4,5}  // bottom
    };

    for (size_t i = 0; i < faces.size(); ++i) {
        auto f = faces[i];
        Vector3 n = normals[f.n];
        Vector3 t(1.0f, 0.0f, 0.0f);
        Vector3 b(0.0f, 1.0f, 0.0f);
        Color col = i < faceColors.size() ? faceColors[i] : Color::WHITE;
        int i0 = mesh->addVertex(Vertex(positions[f.a], n, uvs[0], col, t, b));
        int i1 = mesh->addVertex(Vertex(positions[f.b], n, uvs[1], col, t, b));
        int i2 = mesh->addVertex(Vertex(positions[f.c], n, uvs[2], col, t, b));
        int i3 = mesh->addVertex(Vertex(positions[f.d], n, uvs[3], col, t, b));
        if (!inwardNormals) {
            mesh->addTriangle(i0, i1, i2);
            mesh->addTriangle(i0, i2, i3);
        } else {
            // 内表面反向缝合，保持从内看为正面
            mesh->addTriangle(i0, i2, i1);
            mesh->addTriangle(i0, i3, i2);
        }
    }
}

namespace {

using Core::Math::Vector2;
using Core::Math::Vector3;
using Core::Types::Color;
using Core::Types::Vertex;

static const Vector2 kQuadUVs[4] = {
    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 0.0f),
    Vector2(1.0f, 1.0f),
    Vector2(0.0f, 1.0f)
};

inline Vector3 defaultTangent(const Vector3& normal) {
    Vector3 tangent = std::fabs(normal.x) > 0.5f ? Vector3(0.0f, 1.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
    Vector3 adjusted = tangent - normal * tangent.dot(normal);
    if (adjusted.lengthSquared() <= 1e-8f) {
        adjusted = Vector3(0.0f, 0.0f, 1.0f);
    }
    return adjusted.normalize();
}

inline Vector3 defaultBitangent(const Vector3& normal, const Vector3& tangent) {
    Vector3 bitangent = normal.cross(tangent);
    if (bitangent.lengthSquared() <= 1e-8f) {
        bitangent = Vector3(0.0f, 1.0f, 0.0f);
    }
    return bitangent.normalize();
}

void appendQuad(Scene::Mesh* mesh,
                const std::array<Vector3, 4>& positions,
                const Vector3& normal,
                const Color& color,
                bool invertWinding = false) {
    Vector3 n = normal.normalize();
    Vector3 tangent = defaultTangent(n);
    Vector3 bitangent = defaultBitangent(n, tangent);

    int indices[4];
    for (int i = 0; i < 4; ++i) {
        Vector3 finalNormal = invertWinding ? n * -1.0f : n;
        Vector3 finalTangent = invertWinding ? tangent * -1.0f : tangent;
        Vector3 finalBitangent = invertWinding ? bitangent * -1.0f : bitangent;
        indices[i] = mesh->addVertex(Vertex(positions[i], finalNormal, kQuadUVs[i], color, finalTangent, finalBitangent));
    }

    if (!invertWinding) {
        mesh->addTriangle(indices[0], indices[1], indices[2]);
        mesh->addTriangle(indices[0], indices[2], indices[3]);
    } else {
        mesh->addTriangle(indices[0], indices[2], indices[1]);
        mesh->addTriangle(indices[0], indices[3], indices[2]);
    }
}

Vector3 edgeWallNormal(const Vector3& outerA,
                       const Vector3& outerB,
                       const Vector3& innerA) {
    Vector3 edgeDir = outerB - outerA;
    Vector3 inward = innerA - outerA;
    Vector3 n = edgeDir.cross(inward);
    if (n.lengthSquared() <= 1e-8f) {
        // fallback：避免零向量
        n = edgeDir.cross(Vector3(0.0f, 1.0f, 0.0f));
        if (n.lengthSquared() <= 1e-8f) {
            n = edgeDir.cross(Vector3(0.0f, 0.0f, 1.0f));
        }
    }
    return n.normalize();
}

} // namespace

Mesh* Mesh::createHollowCube(float outerSize, float innerSize) {
    using Core::Math::Vector2;
    using Core::Math::Vector3;
    using Core::Types::Color;

    if (innerSize >= outerSize || innerSize <= 0.0f) {
        return createCube(outerSize);
    }

    Mesh* mesh = new Mesh();
    float ho = outerSize * 0.5f;
    float hi = innerSize * 0.5f;

    std::vector<Vector3> outer = {
        {-ho, -ho, -ho}, {ho, -ho, -ho}, {ho, ho, -ho}, {-ho, ho, -ho},
        {-ho, -ho,  ho}, {ho, -ho,  ho}, {ho, ho,  ho}, {-ho, ho,  ho}
    };
    std::vector<Vector3> inner = {
        {-hi, -hi, -hi}, {hi, -hi, -hi}, {hi, hi, -hi}, {-hi, hi, -hi},
        {-hi, -hi,  hi}, {hi, -hi,  hi}, {hi, hi,  hi}, {-hi, hi,  hi}
    };

    const std::vector<Vector3> faceNormals = {
        {0, 0, -1}, {0, 0, 1}, {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}
    };
    const std::vector<Color> faceColors = {
        Color(1.0f, 0.2f, 0.2f, 0.6f),
        Color(0.2f, 1.0f, 0.8f, 0.6f),
        Color(0.2f, 0.2f, 1.0f, 0.6f),
        Color(1.0f, 1.0f, 0.2f, 0.6f),
        Color(1.0f, 0.5f, 0.2f, 0.6f),
        Color(0.2f, 1.0f, 1.0f, 0.6f)
    };

    const int faces[6][4] = {
        {1, 0, 3, 2}, // back
        {4, 5, 6, 7}, // front
        {4, 0, 3, 7}, // left
        {5, 1, 2, 6}, // right
        {3, 7, 6, 2}, // top
        {0, 1, 5, 4}  // bottom
    };

    // 外层
    for (int f = 0; f < 6; ++f) {
        std::array<Vector3, 4> quad{};
        for (int i = 0; i < 4; ++i) {
            quad[i] = outer[faces[f][i]];
        }
        appendQuad(mesh, quad, faceNormals[f], faceColors[f]);
    }

    for (int f = 0; f < 6; ++f) {
        std::array<Vector3, 4> quad{};
        for (int i = 0; i < 4; ++i) {
            quad[i] = inner[faces[f][i]];
        }
        appendQuad(mesh, quad, faceNormals[f], faceColors[f], true);
    }

    const std::array<std::pair<int, int>, 12> uniqueEdges = {
        std::make_pair(0, 1), std::make_pair(1, 2), std::make_pair(2, 3), std::make_pair(3, 0),
        std::make_pair(4, 5), std::make_pair(5, 6), std::make_pair(6, 7), std::make_pair(7, 4),
        std::make_pair(0, 4), std::make_pair(1, 5), std::make_pair(2, 6), std::make_pair(3, 7)
    };

    auto edgeColors = [&](int a, int b) {
        Color accum(0.0f, 0.0f, 0.0f, 0.0f);
        int count = 0;
        for (int f = 0; f < 6; ++f) {
            bool hasA = false;
            bool hasB = false;
            for (int i = 0; i < 4; ++i) {
                if (faces[f][i] == a) hasA = true;
                if (faces[f][i] == b) hasB = true;
            }
            if (hasA && hasB) {
                accum = accum + faceColors[f];
                ++count;
            }
        }
        if (count == 0) {
            return Color::WHITE;
        }
        float inv = 1.0f / static_cast<float>(count);
        return Color(accum.r * inv, accum.g * inv, accum.b * inv, accum.a * inv);
    };

    for (const auto& edge : uniqueEdges) {
        int a = edge.first;
        int b = edge.second;
        Vector3 p0 = outer[a];
        Vector3 p1 = outer[b];
        Vector3 p2 = inner[b];
        Vector3 p3 = inner[a];

        Vector3 normal = edgeWallNormal(p0, p1, p3);
        Color color = edgeColors(a, b);
        std::array<Vector3, 4> quad = {p0, p1, p2, p3};
        appendQuad(mesh, quad, normal, color);
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

Mesh* Mesh::createGradientSphere(float radius, int segments,
                                 const Color& topColor,
                                 const Color& bottomColor) {
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
            // 纵向渐变：v=0 顶部→topColor，v=1 底部→bottomColor
            Color col = topColor * (1.0f - v) + bottomColor * v;
            mesh->m_vertices.emplace_back(position, normal, Vector2(u, 1.0f - v), col);
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
