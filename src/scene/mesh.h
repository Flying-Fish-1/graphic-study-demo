#ifndef SCENE_MESH_H
#define SCENE_MESH_H

#include "../core/types/vertex.h"
#include "../core/types/material.h"
#include "../core/math/vector.h"
#include "../core/math/matrix.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Scene {

using namespace Core::Types;
using namespace Core::Math;

struct BoundingBox {
    Vector3 min;
    Vector3 max;

    BoundingBox();
    BoundingBox(const Vector3& minPoint, const Vector3& maxPoint);

    bool contains(const Vector3& point) const;
    bool intersects(const BoundingBox& other) const;
    Vector3 getCenter() const;
    Vector3 getSize() const;
    void expand(const Vector3& point);
    void reset();
};

class Mesh {
private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    Material* m_material;
    BoundingBox m_bounds;

public:
    Mesh();
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    int addVertex(const Vertex& vertex);
    void addTriangle(uint32_t i0, uint32_t i1, uint32_t i2);

    void setMaterial(Material* material) { m_material = material; }
    Material* getMaterial() const { return m_material; }

    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    std::vector<Vertex>& accessVertices() { return m_vertices; }

    const std::vector<uint32_t>& getIndices() const { return m_indices; }
    std::vector<uint32_t>& accessIndices() { return m_indices; }

    const BoundingBox& getBoundingBox() const { return m_bounds; }
    void calculateBoundingBox();

    void calculateFaceNormals();
    void calculateVertexNormals();
    void ensureTangents();

    static Mesh* createCube(float size = 1.0f);
    static Mesh* createQuad(float width = 1.0f, float height = 1.0f);
    static Mesh* createTriangle(float size = 1.0f);
    static Mesh* createSphere(float radius = 1.0f, int segments = 16);
    static Mesh* createPlane(float width = 1.0f, float height = 1.0f, int subdivisions = 1);

    static Mesh* loadFromFile(const std::string& filename);
};

} // namespace Scene

#endif // SCENE_MESH_H
