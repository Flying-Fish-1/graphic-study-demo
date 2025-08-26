#ifndef CORE_TYPES_TRIANGLE_H
#define CORE_TYPES_TRIANGLE_H

#include "vertex.h"
#include "../math/vector.h"
#include "../math/matrix.h"

namespace Core {
namespace Types {

// 前向声明
class Material;

/**
 * @brief 3D三角形结构
 * 
 * 表示一个3D三角形面片，包含三个顶点和相关属性
 */
struct Triangle {
    Vertex vertices[3];     // 三角形的三个顶点
    Vector3 faceNormal;     // 面法向量
    float depth;            // 平均深度值
    Material* material;     // 材质指针（可选）
    
    /**
     * @brief 默认构造函数
     */
    Triangle();
    
    /**
     * @brief 使用三个顶点构造三角形
     */
    Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);
    
    /**
     * @brief 计算面法向量
     * 使用叉积计算平面法向量
     */
    void calculateNormal();
    
    /**
     * @brief 计算平均深度
     * 用于深度排序
     */
    void calculateDepth();
    
    /**
     * @brief 检查是否为背面
     * @param viewDir 视线方向
     * @return 如果是背面返回true
     */
    bool isBackface(const Vector3& viewDir) const;
    
    /**
     * @brief 获取三角形的包围盒
     * @param minX 最小X坐标
     * @param minY 最小Y坐标
     * @param maxX 最大X坐标
     * @param maxY 最大Y坐标
     */
    void getBoundingBox(int& minX, int& minY, int& maxX, int& maxY) const;
    
    /**
     * @brief 计算点在三角形内的重心坐标
     * @param point 测试点
     * @param u 重心坐标u
     * @param v 重心坐标v
     * @param w 重心坐标w
     * @return 点是否在三角形内
     */
    bool getBarycentricCoords(const Vector2& point, float& u, float& v, float& w) const;
    
    /**
     * @brief 应用变换矩阵
     * @param transform 变换矩阵
     */
    void transform(const Matrix4& transform);
    
    /**
     * @brief 计算三角形面积
     * @return 三角形面积
     */
    float getArea() const;
};

} // namespace Types
} // namespace Core

#endif // CORE_TYPES_TRIANGLE_H
