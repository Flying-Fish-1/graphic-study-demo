#include "triangle.h"
#include "../platform/constants.h"
#include <algorithm>

namespace Core {
namespace Types {

Triangle::Triangle() : faceNormal(0, 1, 0), depth(0), material(nullptr) {
}

Triangle::Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) 
    : faceNormal(0, 1, 0), depth(0), material(nullptr) {
    vertices[0] = v0;
    vertices[1] = v1;
    vertices[2] = v2;
    
    calculateNormal();
    calculateDepth();
}

void Triangle::calculateNormal() {
    // 使用叉积计算面法向量
    Vector3 edge1 = vertices[1].position - vertices[0].position;
    Vector3 edge2 = vertices[2].position - vertices[0].position;
    
    faceNormal = edge1.cross(edge2).normalize();
}

void Triangle::calculateDepth() {
    // 计算三个顶点的平均深度
    depth = (vertices[0].depth + vertices[1].depth + vertices[2].depth) / 3.0f;
}

bool Triangle::isBackface(const Vector3& viewDir) const {
    // 如果面法向量与视线方向的点积大于0，则为背面
    return faceNormal.dot(viewDir) > 0;
}

void Triangle::getBoundingBox(int& minX, int& minY, int& maxX, int& maxY) const {
    minX = static_cast<int>(std::min({vertices[0].position.x, vertices[1].position.x, vertices[2].position.x}));
    minY = static_cast<int>(std::min({vertices[0].position.y, vertices[1].position.y, vertices[2].position.y}));
    maxX = static_cast<int>(std::max({vertices[0].position.x, vertices[1].position.x, vertices[2].position.x}));
    maxY = static_cast<int>(std::max({vertices[0].position.y, vertices[1].position.y, vertices[2].position.y}));
    
    // 限制在屏幕范围内
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, SCREEN_WIDTH - 1);
    maxY = std::min(maxY, SCREEN_HEIGHT - 1);
}

bool Triangle::getBarycentricCoords(const Vector2& point, float& u, float& v, float& w) const {
    // 使用向量方法计算重心坐标
    Vector2 v0(vertices[0].position.x, vertices[0].position.y);
    Vector2 v1(vertices[1].position.x, vertices[1].position.y);
    Vector2 v2(vertices[2].position.x, vertices[2].position.y);
    
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

void Triangle::transform(const Matrix4& transform) {
    for (int i = 0; i < 3; i++) {
        vertices[i].transform(transform);
    }
    
    calculateNormal();
    calculateDepth();
}

float Triangle::getArea() const {
    Vector3 edge1 = vertices[1].position - vertices[0].position;
    Vector3 edge2 = vertices[2].position - vertices[0].position;
    
    return edge1.cross(edge2).length() * 0.5f;
}

} // namespace Types
} // namespace Core
