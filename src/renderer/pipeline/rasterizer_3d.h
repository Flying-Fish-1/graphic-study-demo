#ifndef RENDERER_PIPELINE_RASTERIZER_3D_H
#define RENDERER_PIPELINE_RASTERIZER_3D_H

#include "depth_buffer.h"
#include "../../core/types/triangle.h"
#include "../../core/types/vertex.h"
#include "../../core/platform/sdl_wrapper.h"

namespace Renderer {
namespace Pipeline {

using namespace Core::Types;
using namespace Core::Platform;

/**
 * @brief 3D光栅化器
 * 
 * 负责将3D三角形光栅化到屏幕上，支持深度测试和透视校正插值
 */
class Rasterizer3D {
private:
    DepthBuffer* m_depthBuffer;     // 深度缓冲区
    SDLWrapper* m_platform;        // 平台接口
    bool m_wireframeMode;          // 线框模式
    bool m_perspectiveCorrection;  // 透视校正开关
    
public:
    /**
     * @brief 构造函数
     * @param platform 平台接口
     * @param depthBuffer 深度缓冲区
     */
    Rasterizer3D(SDLWrapper* platform, DepthBuffer* depthBuffer);
    
    /**
     * @brief 析构函数
     */
    ~Rasterizer3D() = default;
    
    /**
     * @brief 光栅化3D三角形
     * @param triangle 要光栅化的三角形
     */
    void rasterizeTriangle(const Triangle& triangle);
    
    /**
     * @brief 光栅化3D三角形（提供变换后的屏幕坐标）
     * @param triangle 3D三角形数据
     * @param screenVertices 变换后的屏幕坐标
     */
    void rasterizeTriangle(const Triangle& triangle, const Vector2 screenVertices[3]);
    
    /**
     * @brief 插值顶点属性
     * @param triangle 三角形
     * @param u 重心坐标u
     * @param v 重心坐标v
     * @param w 重心坐标w
     * @return 插值后的顶点
     */
    Vertex interpolateVertex(const Triangle& triangle, float u, float v, float w) const;
    
    /**
     * @brief 插值颜色
     * @param triangle 三角形
     * @param u 重心坐标u
     * @param v 重心坐标v
     * @param w 重心坐标w
     * @return 插值后的颜色
     */
    Color interpolateColor(const Triangle& triangle, float u, float v, float w) const;
    
    /**
     * @brief 插值法向量
     * @param triangle 三角形
     * @param u 重心坐标u
     * @param v 重心坐标v
     * @param w 重心坐标w
     * @return 插值后的法向量
     */
    Vector3 interpolateNormal(const Triangle& triangle, float u, float v, float w) const;
    
    /**
     * @brief 绘制三角形线框
     * @param screenVertices 屏幕坐标顶点
     * @param color 线框颜色
     */
    void drawWireframe(const Vector2 screenVertices[3], const Color& color);
    
    /**
     * @brief 计算屏幕空间的重心坐标
     * @param point 屏幕上的点
     * @param v0 顶点0的屏幕坐标
     * @param v1 顶点1的屏幕坐标
     * @param v2 顶点2的屏幕坐标
     * @param u 输出重心坐标u
     * @param v 输出重心坐标v
     * @param w 输出重心坐标w
     * @return 点是否在三角形内
     */
    bool getBarycentricCoords(const Vector2& point, const Vector2& v0, const Vector2& v1, const Vector2& v2,
                             float& u, float& v, float& w) const;
    
    // 渲染状态设置
    void setWireframeMode(bool enabled) { m_wireframeMode = enabled; }
    void setPerspectiveCorrection(bool enabled) { m_perspectiveCorrection = enabled; }
    
    // 状态查询
    bool isWireframeMode() const { return m_wireframeMode; }
    bool isPerspectiveCorrectionEnabled() const { return m_perspectiveCorrection; }
    
private:
    /**
     * @brief 获取三角形的2D包围盒
     * @param screenVertices 屏幕坐标顶点
     * @param minX 最小X坐标
     * @param minY 最小Y坐标
     * @param maxX 最大X坐标
     * @param maxY 最大Y坐标
     */
    void getBoundingBox(const Vector2 screenVertices[3], int& minX, int& minY, int& maxX, int& maxY) const;
    
    /**
     * @brief 限制包围盒到屏幕范围内
     * @param minX 最小X坐标
     * @param minY 最小Y坐标
     * @param maxX 最大X坐标
     * @param maxY 最大Y坐标
     */
    void clampToScreen(int& minX, int& minY, int& maxX, int& maxY) const;
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_RASTERIZER_3D_H
