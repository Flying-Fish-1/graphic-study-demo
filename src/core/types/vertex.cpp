#include "vertex.h"
#include "../math/matrix.h"

namespace Core {
namespace Types {

Vertex::Vertex() 
    : position(0, 0, 0), normal(0, 1, 0), texCoord(0, 0), color(Color::WHITE), depth(0) {
}

Vertex::Vertex(const Vector3& pos, const Vector3& norm, const Vector2& uv, const Color& col)
    : position(pos), normal(norm), texCoord(uv), color(col), depth(pos.z) {
}

Vertex Vertex::interpolate(const Vertex& v0, const Vertex& v1, const Vertex& v2, 
                          float u, float v, float w) {
    Vertex result;
    
    // 插值位置
    result.position = v0.position * w + v1.position * v + v2.position * u;
    
    // 插值法向量并归一化
    result.normal = (v0.normal * w + v1.normal * v + v2.normal * u).normalize();
    
    // 插值纹理坐标
    result.texCoord = v0.texCoord * w + v1.texCoord * v + v2.texCoord * u;
    
    // 插值颜色
    result.color = Color(
        v0.color.r * w + v1.color.r * v + v2.color.r * u,
        v0.color.g * w + v1.color.g * v + v2.color.g * u,
        v0.color.b * w + v1.color.b * v + v2.color.b * u,
        v0.color.a * w + v1.color.a * v + v2.color.a * u
    );
    
    // 插值深度
    result.depth = v0.depth * w + v1.depth * v + v2.depth * u;
    
    return result;
}

Vertex Vertex::interpolatePerspectiveCorrect(const Vertex& v0, const Vertex& v1, const Vertex& v2, 
                                            float u, float v, float w) {
    // 透视校正插值公式：
    // attribute = (a0/w0 * u + a1/w1 * v + a2/w2 * w) / (1/w0 * u + 1/w1 * v + 1/w2 * w)
    
    float w0 = v0.position.z; // 使用z作为w分量
    float w1 = v1.position.z;
    float w2 = v2.position.z;
    
    // 防止除零
    if (w0 == 0) w0 = 0.001f;
    if (w1 == 0) w1 = 0.001f;
    if (w2 == 0) w2 = 0.001f;
    
    float invW0 = 1.0f / w0;
    float invW1 = 1.0f / w1;
    float invW2 = 1.0f / w2;
    
    float denominator = invW0 * u + invW1 * v + invW2 * w;
    
    if (denominator == 0) {
        return interpolate(v0, v1, v2, u, v, w); // 回退到线性插值
    }
    
    Vertex result;
    
    // 透视校正插值
    result.texCoord = (v0.texCoord * invW0 * u + v1.texCoord * invW1 * v + v2.texCoord * invW2 * w) / denominator;
    
    // 颜色插值
    Color c0 = v0.color * invW0 * u;
    Color c1 = v1.color * invW1 * v;
    Color c2 = v2.color * invW2 * w;
    result.color = Color(
        (c0.r + c1.r + c2.r) / denominator,
        (c0.g + c1.g + c2.g) / denominator,
        (c0.b + c1.b + c2.b) / denominator,
        (c0.a + c1.a + c2.a) / denominator
    );
    
    // 位置和法向量仍使用线性插值
    result.position = v0.position * w + v1.position * v + v2.position * u;
    result.normal = (v0.normal * w + v1.normal * v + v2.normal * u).normalize();
    result.depth = v0.depth * w + v1.depth * v + v2.depth * u;
    
    return result;
}

void Vertex::transform(const Matrix4& transform) {
    position = transform.transformPoint(position);
    normal = transform.transformDirection(normal).normalize();
    depth = position.z;
}

Vector2 Vertex::projectToScreen(const Matrix4& projectionMatrix, int screenWidth, int screenHeight) {
    Vector3 projected = projectionMatrix.transformPoint(position);
    
    // NDC坐标 [-1, 1] 转换到屏幕坐标 [0, screenSize]
    float x = (projected.x + 1.0f) * 0.5f * screenWidth;
    float y = (1.0f - projected.y) * 0.5f * screenHeight; // Y轴翻转
    
    return Vector2(x, y);
}

} // namespace Types
} // namespace Core
