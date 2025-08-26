#ifndef CORE_TYPES_VERTEX_H
#define CORE_TYPES_VERTEX_H

#include "../math/vector.h"
#include "../math/matrix.h"
#include "color.h"

namespace Core {
namespace Types {

using namespace Core::Math;

/**
 * @brief 3D顶点结构
 * 
 * 包含3D渲染所需的所有顶点属性
 */
struct Vertex {
    Vector3 position;       // 世界坐标
    Vector3 normal;         // 法向量
    Vector2 texCoord;       // 纹理坐标
    Color color;            // 顶点颜色
    float depth;            // 深度值
    
    /**
     * @brief 默认构造函数
     */
    Vertex();
    
    /**
     * @brief 完整构造函数
     */
    Vertex(const Vector3& pos, const Vector3& norm, const Vector2& uv, const Color& col);
    
    /**
     * @brief 使用重心坐标插值三个顶点
     * @param v0 顶点0
     * @param v1 顶点1
     * @param v2 顶点2
     * @param u 重心坐标u
     * @param v 重心坐标v
     * @param w 重心坐标w (通常 w = 1 - u - v)
     * @return 插值后的顶点
     */
    static Vertex interpolate(const Vertex& v0, const Vertex& v1, const Vertex& v2, 
                             float u, float v, float w);
    
    /**
     * @brief 透视校正插值
     * @param v0 顶点0
     * @param v1 顶点1
     * @param v2 顶点2
     * @param u 重心坐标u
     * @param v 重心坐标v
     * @param w 重心坐标w
     * @return 透视校正后的顶点
     */
    static Vertex interpolatePerspectiveCorrect(const Vertex& v0, const Vertex& v1, const Vertex& v2, 
                                               float u, float v, float w);
    
    /**
     * @brief 应用变换矩阵
     * @param transform 4x4变换矩阵
     */
    void transform(const Matrix4& transform);
    
    /**
     * @brief 投影到屏幕坐标
     * @param projectionMatrix 投影矩阵
     * @param screenWidth 屏幕宽度
     * @param screenHeight 屏幕高度
     */
    Vector2 projectToScreen(const Matrix4& projectionMatrix, int screenWidth, int screenHeight);
};

} // namespace Types
} // namespace Core

#endif // CORE_TYPES_VERTEX_H
