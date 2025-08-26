#include "mesh.h"
#include <algorithm>

namespace Scene {

// ==================== BoundingBox 实现 ====================

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

void BoundingBox::expand(const Vector3& point) {
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    min.z = std::min(min.z, point.z);
    
    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
    max.z = std::max(max.z, point.z);
}

void BoundingBox::reset() {
    min = Vector3(999999, 999999, 999999);
    max = Vector3(-999999, -999999, -999999);
}

// ==================== Mesh 实现 ====================

Mesh::Mesh() : m_material(nullptr), m_transform(Matrix4::identity()), m_useIndices(false) {
}

Mesh::~Mesh() {
    // 注意：不删除材质，因为材质可能被多个网格共享
}

int Mesh::addVertex(const Vertex& vertex) {
    m_vertices.push_back(vertex);
    return static_cast<int>(m_vertices.size() - 1);
}

void Mesh::addTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
    Triangle triangle(v0, v1, v2);
    triangle.material = m_material;
    m_triangles.push_back(triangle);
}

void Mesh::addTriangle(int i0, int i1, int i2) {
    if (i0 < m_vertices.size() && i1 < m_vertices.size() && i2 < m_vertices.size()) {
        m_indices.push_back(i0);
        m_indices.push_back(i1);
        m_indices.push_back(i2);
        m_useIndices = true;
        
        // 同时创建三角形
        Triangle triangle(m_vertices[i0], m_vertices[i1], m_vertices[i2]);
        triangle.material = m_material;
        m_triangles.push_back(triangle);
    }
}

void Mesh::calculateNormals() {
    // 先计算面法向量
    for (auto& triangle : m_triangles) {
        triangle.calculateNormal();
    }
    
    // 如果使用顶点，也计算顶点法向量
    if (!m_vertices.empty()) {
        calculateVertexNormals();
    }
}

void Mesh::calculateVertexNormals() {
    // 重置所有顶点法向量
    for (auto& vertex : m_vertices) {
        vertex.normal = Vector3(0, 0, 0);
    }
    
    // 累加相邻面的法向量
    if (m_useIndices && m_indices.size() % 3 == 0) {
        for (size_t i = 0; i < m_indices.size(); i += 3) {
            int i0 = m_indices[i];
            int i1 = m_indices[i + 1];
            int i2 = m_indices[i + 2];
            
            Vector3 faceNormal = m_triangles[i / 3].faceNormal;
            
            m_vertices[i0].normal = m_vertices[i0].normal + faceNormal;
            m_vertices[i1].normal = m_vertices[i1].normal + faceNormal;
            m_vertices[i2].normal = m_vertices[i2].normal + faceNormal;
        }
    }
    
    // 归一化顶点法向量
    for (auto& vertex : m_vertices) {
        vertex.normal = vertex.normal.normalize();
    }
}

void Mesh::calculateBoundingBox() {
    m_boundingBox.reset();
    
    for (const auto& vertex : m_vertices) {
        m_boundingBox.expand(vertex.position);
    }
    
    // 如果没有顶点，使用三角形顶点
    if (m_vertices.empty()) {
        for (const auto& triangle : m_triangles) {
            for (int i = 0; i < 3; i++) {
                m_boundingBox.expand(triangle.vertices[i].position);
            }
        }
    }
}

void Mesh::transform(const Matrix4& transform) {
    // 变换所有顶点
    for (auto& vertex : m_vertices) {
        vertex.transform(transform);
    }
    
    // 变换所有三角形
    for (auto& triangle : m_triangles) {
        triangle.transform(transform);
    }
    
    // 重新计算包围盒
    calculateBoundingBox();
}

std::vector<Triangle> Mesh::getTransformedTriangles() const {
    std::vector<Triangle> transformedTriangles = m_triangles;
    
    // 应用本地变换矩阵
    for (auto& triangle : transformedTriangles) {
        triangle.transform(m_transform);
    }
    
    return transformedTriangles;
}

void Mesh::clear() {
    m_vertices.clear();
    m_triangles.clear();
    m_indices.clear();
    m_useIndices = false;
    m_boundingBox.reset();
}

void Mesh::rebuildTriangles() {
    m_triangles.clear();
    
    if (m_useIndices && m_indices.size() % 3 == 0) {
        for (size_t i = 0; i < m_indices.size(); i += 3) {
            int i0 = m_indices[i];
            int i1 = m_indices[i + 1];
            int i2 = m_indices[i + 2];
            
            if (i0 < m_vertices.size() && i1 < m_vertices.size() && i2 < m_vertices.size()) {
                Triangle triangle(m_vertices[i0], m_vertices[i1], m_vertices[i2]);
                triangle.material = m_material;
                m_triangles.push_back(triangle);
            }
        }
    }
}

// ==================== 预定义几何体创建 ====================

Mesh* Mesh::createCube(float size) {
    Mesh* mesh = new Mesh();
    float half = size * 0.5f;
    
    // 立方体的8个顶点
    std::vector<Vector3> positions = {
        Vector3(-half, -half, -half), Vector3(half, -half, -half), Vector3(half, half, -half), Vector3(-half, half, -half),  // 后面
        Vector3(-half, -half, half),  Vector3(half, -half, half),  Vector3(half, half, half),  Vector3(-half, half, half)   // 前面
    };
    
    // 立方体的6个面的法向量
    std::vector<Vector3> normals = {
        Vector3(0, 0, -1), Vector3(0, 0, 1),   // 后面、前面
        Vector3(-1, 0, 0), Vector3(1, 0, 0),   // 左面、右面
        Vector3(0, 1, 0),  Vector3(0, -1, 0)   // 上面、下面
    };
    
    // 立方体的6个面（每面4个顶点索引）
    int faceIndices[6][4] = {
        {0, 1, 2, 3}, // 后面
        {4, 7, 6, 5}, // 前面
        {0, 4, 7, 3}, // 左面
        {1, 5, 6, 2}, // 右面
        {3, 2, 6, 7}, // 上面
        {0, 1, 5, 4}  // 下面
    };
    
    // 为每个面创建顶点和三角形
    for (int face = 0; face < 6; face++) {
        Vector3 normal = normals[face];
        Color faceColor = Color(0.8f, 0.8f, 0.8f, 1.0f);
        
        // 每个面的4个顶点
        Vertex vertices[4];
        for (int i = 0; i < 4; i++) {
            vertices[i] = Vertex(positions[faceIndices[face][i]], normal, 
                                Vector2(static_cast<float>(i % 2), static_cast<float>(i / 2)), faceColor);
        }
        
        // 每个面用2个三角形组成
        mesh->addTriangle(vertices[0], vertices[1], vertices[2]);
        mesh->addTriangle(vertices[0], vertices[2], vertices[3]);
    }
    
    mesh->calculateBoundingBox();
    return mesh;
}

Mesh* Mesh::createQuad(float width, float height) {
    Mesh* mesh = new Mesh();
    float halfW = width * 0.5f;
    float halfH = height * 0.5f;
    
    Vector3 normal(0, 0, 1);
    Color color = Color::WHITE;
    
    Vertex v0(Vector3(-halfW, -halfH, 0), normal, Vector2(0, 0), color);
    Vertex v1(Vector3(halfW, -halfH, 0), normal, Vector2(1, 0), color);
    Vertex v2(Vector3(halfW, halfH, 0), normal, Vector2(1, 1), color);
    Vertex v3(Vector3(-halfW, halfH, 0), normal, Vector2(0, 1), color);
    
    mesh->addTriangle(v0, v1, v2);
    mesh->addTriangle(v0, v2, v3);
    
    mesh->calculateBoundingBox();
    return mesh;
}

Mesh* Mesh::createTriangle(float size) {
    Mesh* mesh = new Mesh();
    float half = size * 0.5f;
    
    Vector3 normal(0, 0, 1);
    
    Vertex v0(Vector3(0, half, 0), normal, Vector2(0.5f, 0), Color::RED);
    Vertex v1(Vector3(-half, -half, 0), normal, Vector2(0, 1), Color::GREEN);
    Vertex v2(Vector3(half, -half, 0), normal, Vector2(1, 1), Color::BLUE);
    
    mesh->addTriangle(v0, v1, v2);
    
    mesh->calculateBoundingBox();
    return mesh;
}

Mesh* Mesh::createSphere(float radius, int segments) {
    Mesh* mesh = new Mesh();
    
    // 简化实现：创建一个八面体作为球体近似
    // 在实际项目中，应该使用更复杂的球体生成算法
    
    // 6个顶点
    mesh->addVertex(Vertex(Vector3(0, radius, 0), Vector3(0, 1, 0), Vector2(0.5f, 0), Color::WHITE));    // 顶点
    mesh->addVertex(Vertex(Vector3(0, -radius, 0), Vector3(0, -1, 0), Vector2(0.5f, 1), Color::WHITE));  // 底点
    mesh->addVertex(Vertex(Vector3(radius, 0, 0), Vector3(1, 0, 0), Vector2(1, 0.5f), Color::WHITE));    // 右
    mesh->addVertex(Vertex(Vector3(-radius, 0, 0), Vector3(-1, 0, 0), Vector2(0, 0.5f), Color::WHITE));  // 左
    mesh->addVertex(Vertex(Vector3(0, 0, radius), Vector3(0, 0, 1), Vector2(0.5f, 0.25f), Color::WHITE)); // 前
    mesh->addVertex(Vertex(Vector3(0, 0, -radius), Vector3(0, 0, -1), Vector2(0.5f, 0.75f), Color::WHITE)); // 后
    
    // 8个三角形面
    mesh->addTriangle(0, 2, 4); // 上右前
    mesh->addTriangle(0, 4, 3); // 上前左
    mesh->addTriangle(0, 3, 5); // 上左后
    mesh->addTriangle(0, 5, 2); // 上后右
    mesh->addTriangle(1, 4, 2); // 下前右
    mesh->addTriangle(1, 3, 4); // 下左前
    mesh->addTriangle(1, 5, 3); // 下后左
    mesh->addTriangle(1, 2, 5); // 下右后
    
    mesh->calculateNormals();
    mesh->calculateBoundingBox();
    return mesh;
}

Mesh* Mesh::createPlane(float width, float height, int subdivisions) {
    // 简化实现：创建一个四边形作为平面
    return createQuad(width, height);
}

Mesh* Mesh::loadFromFile(const std::string& filename) {
    // 简单实现：目前不支持文件加载
    // 在实际项目中，这里应该实现OBJ、PLY等格式的加载器
    
    // 返回一个默认立方体
    return createCube(1.0f);
}

} // namespace Scene
