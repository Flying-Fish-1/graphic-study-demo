#ifndef SCENE_MESH_H
#define SCENE_MESH_H

#include "../core/types/vertex.h"
#include "../core/types/triangle.h"
#include "../core/types/material.h"
#include "../core/math/vector.h"
#include "../core/math/matrix.h"
#include <vector>
#include <string>

namespace Scene {

using namespace Core::Types;
using namespace Core::Math;

/**
 * @brief 包围盒结构
 */
struct BoundingBox {
    Vector3 min;    // 最小坐标
    Vector3 max;    // 最大坐标
    
    BoundingBox() : min(Vector3(0, 0, 0)), max(Vector3(0, 0, 0)) {}
    BoundingBox(const Vector3& minPoint, const Vector3& maxPoint) : min(minPoint), max(maxPoint) {}
    
    /**
     * @brief 检查点是否在包围盒内
     */
    bool contains(const Vector3& point) const;
    
    /**
     * @brief 检查两个包围盒是否相交
     */
    bool intersects(const BoundingBox& other) const;
    
    /**
     * @brief 获取包围盒中心
     */
    Vector3 getCenter() const { return (min + max) * 0.5f; }
    
    /**
     * @brief 获取包围盒尺寸
     */
    Vector3 getSize() const { return max - min; }
    
    /**
     * @brief 扩展包围盒以包含指定点
     */
    void expand(const Vector3& point);
    
    /**
     * @brief 重置包围盒
     */
    void reset();
};

/**
 * @brief 3D网格类
 * 
 * 表示一个3D模型，包含顶点、三角形和材质信息
 */
class Mesh {
private:
    std::vector<Vertex> m_vertices;         // 顶点数组
    std::vector<Triangle> m_triangles;      // 三角形数组
    std::vector<int> m_indices;             // 索引数组（可选，用于优化）
    Material* m_material;                   // 材质
    BoundingBox m_boundingBox;              // 包围盒
    Matrix4 m_transform;                    // 本地变换矩阵
    bool m_useIndices;                      // 是否使用索引渲染
    
public:
    /**
     * @brief 构造函数
     */
    Mesh();
    
    /**
     * @brief 析构函数
     */
    ~Mesh();
    
    // 禁止拷贝构造和赋值
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    
    /**
     * @brief 添加顶点
     * @param vertex 顶点
     * @return 顶点索引
     */
    int addVertex(const Vertex& vertex);
    
    /**
     * @brief 添加三角形
     * @param v0 顶点0
     * @param v1 顶点1
     * @param v2 顶点2
     */
    void addTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);
    
    /**
     * @brief 使用索引添加三角形
     * @param i0 顶点索引0
     * @param i1 顶点索引1
     * @param i2 顶点索引2
     */
    void addTriangle(int i0, int i1, int i2);
    
    /**
     * @brief 计算所有三角形的法向量
     */
    void calculateNormals();
    
    /**
     * @brief 计算包围盒
     */
    void calculateBoundingBox();
    
    /**
     * @brief 应用变换矩阵
     * @param transform 变换矩阵
     */
    void transform(const Matrix4& transform);
    
    /**
     * @brief 设置本地变换矩阵
     * @param transform 变换矩阵
     */
    void setTransform(const Matrix4& transform) { m_transform = transform; }
    
    /**
     * @brief 获取变换后的三角形列表
     * @return 变换后的三角形
     */
    std::vector<Triangle> getTransformedTriangles() const;
    
    /**
     * @brief 清空网格数据
     */
    void clear();
    
    // Getter方法
    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<Triangle>& getTriangles() const { return m_triangles; }
    const std::vector<int>& getIndices() const { return m_indices; }
    Material* getMaterial() const { return m_material; }
    const BoundingBox& getBoundingBox() const { return m_boundingBox; }
    const Matrix4& getTransform() const { return m_transform; }
    bool usesIndices() const { return m_useIndices; }
    
    // Setter方法
    void setMaterial(Material* material) { m_material = material; }
    void setUseIndices(bool useIndices) { m_useIndices = useIndices; }
    
    /**
     * @brief 创建预定义几何体
     */
    static Mesh* createCube(float size = 1.0f);
    static Mesh* createQuad(float width = 1.0f, float height = 1.0f);
    static Mesh* createTriangle(float size = 1.0f);
    static Mesh* createSphere(float radius = 1.0f, int segments = 16);
    static Mesh* createPlane(float width = 1.0f, float height = 1.0f, int subdivisions = 1);
    
    /**
     * @brief 从文件加载网格
     * @param filename 文件名
     * @return 网格指针，失败返回nullptr
     */
    static Mesh* loadFromFile(const std::string& filename);
    
private:
    /**
     * @brief 重建三角形数组（从顶点和索引）
     */
    void rebuildTriangles();
    
    /**
     * @brief 计算顶点法向量（平均相邻面的法向量）
     */
    void calculateVertexNormals();
};

} // namespace Scene

#endif // SCENE_MESH_H
