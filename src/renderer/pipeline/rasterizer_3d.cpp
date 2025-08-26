#include "rasterizer_3d.h"
#include "../../core/platform/constants.h"
#include "../../core/types/material.h"
#include "../../graphics/graphics.h"
#include <algorithm>
#include <iostream>

namespace Renderer {
namespace Pipeline {

Rasterizer3D::Rasterizer3D(SDLWrapper* platform, DepthBuffer* depthBuffer)
    : m_platform(platform), m_depthBuffer(depthBuffer), 
      m_wireframeMode(false), m_perspectiveCorrection(true) {
}

void Rasterizer3D::rasterizeTriangle(const Triangle& triangle) {
    // 使用现有的Graphics类进行2D光栅化
    Graphics graphics(m_platform);
    
    // 创建透视投影矩阵 - 调整参数使物体可见
    Matrix4 projectionMatrix = Matrix4::perspective(Constants::PI / 3.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    
    // 将3D顶点投影到屏幕坐标
    Vector2 screenVertices[3];
    for (int i = 0; i < 3; i++) {
        // 使用数学库中的投影矩阵进行正确的3D到2D投影
        Vector3 worldPos = triangle.vertices[i].position;
        
        // 应用投影矩阵
        Vector3 projectedPos = projectionMatrix.transformPoint(worldPos);
        
        // 透视除法
        if (projectedPos.z > 0.0f) {
            float x = projectedPos.x / projectedPos.z;
            float y = projectedPos.y / projectedPos.z;
            
            // 转换到屏幕坐标 (假设视口为800x600)
            screenVertices[i] = Vector2(
                (x + 1.0f) * 400.0f,  // 从[-1,1]转换到[0,800]
                (1.0f - y) * 300.0f   // 从[-1,1]转换到[0,600]，Y轴翻转
            );
            
            // 边界检查，防止超出屏幕范围
            screenVertices[i].x = std::max(0.0f, std::min(799.0f, screenVertices[i].x));
            screenVertices[i].y = std::max(0.0f, std::min(599.0f, screenVertices[i].y));
        } else {
            // 如果z <= 0，将顶点放在屏幕外
            screenVertices[i] = Vector2(-1000.0f, -1000.0f);
        }
    }
    
    // 添加调试信息
    static int debugFrame = 0;
    if (debugFrame % 60 == 0) {
        std::cout << "🔍 投影调试:" << std::endl;
        std::cout << "   3D顶点: (" << triangle.vertices[0].position.x << ", " 
                  << triangle.vertices[0].position.y << ", " << triangle.vertices[0].position.z << ")" << std::endl;
        std::cout << "   屏幕坐标: (" << screenVertices[0].x << ", " << screenVertices[0].y << ")" << std::endl;
    }
    debugFrame++;
    
    // 检查是否所有顶点都在屏幕外
    bool allOutside = true;
    for (int i = 0; i < 3; i++) {
        if (screenVertices[i].x >= 0 && screenVertices[i].x < 800 && 
            screenVertices[i].y >= 0 && screenVertices[i].y < 600) {
            allOutside = false;
            break;
        }
    }
    
    if (allOutside) {
        return; // 跳过完全在屏幕外的三角形
    }
    
    // 使用现有的Graphics类绘制三角形
    if (m_wireframeMode) {
        // 线框模式
        graphics.drawLine(
            static_cast<int>(screenVertices[0].x), static_cast<int>(screenVertices[0].y),
            static_cast<int>(screenVertices[1].x), static_cast<int>(screenVertices[1].y),
            triangle.vertices[0].color, triangle.vertices[1].color
        );
        graphics.drawLine(
            static_cast<int>(screenVertices[1].x), static_cast<int>(screenVertices[1].y),
            static_cast<int>(screenVertices[2].x), static_cast<int>(screenVertices[2].y),
            triangle.vertices[1].color, triangle.vertices[2].color
        );
        graphics.drawLine(
            static_cast<int>(screenVertices[2].x), static_cast<int>(screenVertices[2].y),
            static_cast<int>(screenVertices[0].x), static_cast<int>(screenVertices[0].y),
            triangle.vertices[2].color, triangle.vertices[0].color
        );
    } else {
        // 填充模式 - 使用现有的三角形光栅化
        graphics.rasterizeTriangle(
            static_cast<int>(screenVertices[0].x), static_cast<int>(screenVertices[0].y),
            static_cast<int>(screenVertices[1].x), static_cast<int>(screenVertices[1].y),
            static_cast<int>(screenVertices[2].x), static_cast<int>(screenVertices[2].y),
            triangle.vertices[0].color, triangle.vertices[1].color, triangle.vertices[2].color
        );
    }
}

void Rasterizer3D::rasterizeTriangle(const Triangle& triangle, const Vector2 screenVertices[3]) {
    // 这个函数现在调用上面的版本
    rasterizeTriangle(triangle);
}

Vertex Rasterizer3D::interpolateVertex(const Triangle& triangle, float u, float v, float w) const {
    return Vertex::interpolate(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2], u, v, w);
}

Color Rasterizer3D::interpolateColor(const Triangle& triangle, float u, float v, float w) const {
    const Color& c0 = triangle.vertices[0].color;
    const Color& c1 = triangle.vertices[1].color;
    const Color& c2 = triangle.vertices[2].color;
    
    return Color(
        c0.r * w + c1.r * v + c2.r * u,
        c0.g * w + c1.g * v + c2.g * u,
        c0.b * w + c1.b * v + c2.b * u,
        c0.a * w + c1.a * v + c2.a * u
    );
}

Vector3 Rasterizer3D::interpolateNormal(const Triangle& triangle, float u, float v, float w) const {
    const Vector3& n0 = triangle.vertices[0].normal;
    const Vector3& n1 = triangle.vertices[1].normal;
    const Vector3& n2 = triangle.vertices[2].normal;
    
    Vector3 interpolated = n0 * w + n1 * v + n2 * u;
    return interpolated.normalize();
}

void Rasterizer3D::drawWireframe(const Vector2 screenVertices[3], const Color& color) {
    // 创建Graphics实例来绘制线条
    Graphics graphics(m_platform);
    
    // 绘制三角形的三条边
    graphics.drawLine(
        static_cast<int>(screenVertices[0].x), static_cast<int>(screenVertices[0].y),
        static_cast<int>(screenVertices[1].x), static_cast<int>(screenVertices[1].y),
        color, color
    );
    
    graphics.drawLine(
        static_cast<int>(screenVertices[1].x), static_cast<int>(screenVertices[1].y),
        static_cast<int>(screenVertices[2].x), static_cast<int>(screenVertices[2].y),
        color, color
    );
    
    graphics.drawLine(
        static_cast<int>(screenVertices[2].x), static_cast<int>(screenVertices[2].y),
        static_cast<int>(screenVertices[0].x), static_cast<int>(screenVertices[0].y),
        color, color
    );
}

bool Rasterizer3D::getBarycentricCoords(const Vector2& point, const Vector2& v0, const Vector2& v1, const Vector2& v2,
                                       float& u, float& v, float& w) const {
    Vector2 edge0 = v2 - v0;
    Vector2 edge1 = v1 - v0;
    Vector2 edge2 = point - v0;
    
    float dot00 = edge0.dot(edge0);
    float dot01 = edge0.dot(edge1);
    float dot02 = edge0.dot(edge2);
    float dot11 = edge1.dot(edge1);
    float dot12 = edge1.dot(edge2);
    
    // 计算重心坐标
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    w = 1.0f - u - v;
    
    // 检查是否在三角形内
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

void Rasterizer3D::getBoundingBox(const Vector2 screenVertices[3], int& minX, int& minY, int& maxX, int& maxY) const {
    minX = static_cast<int>(std::min({screenVertices[0].x, screenVertices[1].x, screenVertices[2].x}));
    minY = static_cast<int>(std::min({screenVertices[0].y, screenVertices[1].y, screenVertices[2].y}));
    maxX = static_cast<int>(std::max({screenVertices[0].x, screenVertices[1].x, screenVertices[2].x}));
    maxY = static_cast<int>(std::max({screenVertices[0].y, screenVertices[1].y, screenVertices[2].y}));
}

void Rasterizer3D::clampToScreen(int& minX, int& minY, int& maxX, int& maxY) const {
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, SCREEN_WIDTH - 1);
    maxY = std::min(maxY, SCREEN_HEIGHT - 1);
}

} // namespace Pipeline
} // namespace Renderer
