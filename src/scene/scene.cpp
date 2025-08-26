#include "scene.h"
#include "../renderer/lighting/light.h"
#include "../renderer/pipeline/depth_buffer.h"
#include "../renderer/pipeline/rasterizer_3d.h"
#include <algorithm>
#include <iostream>

namespace Scene {

Scene::Scene() 
    : m_camera(nullptr), 
      m_backgroundColor(0.1f, 0.1f, 0.1f, 1.0f),
      m_ambientLight(0.1f, 0.1f, 0.1f, 1.0f),
      m_totalTriangles(0), m_renderedTriangles(0), m_culledTriangles(0) {
}

Scene::~Scene() {
    // 注意：不删除对象，因为对象可能被多个场景共享
}

int Scene::addObject(Mesh* mesh, const Matrix4& transform) {
    SceneObject object(mesh);
    object.transform = transform;
    m_objects.push_back(object);
    return static_cast<int>(m_objects.size() - 1);
}

void Scene::addLight(Renderer::Lighting::Light* light) {
    m_lights.push_back(light);
}

void Scene::removeObject(int index) {
    if (index >= 0 && index < static_cast<int>(m_objects.size())) {
        m_objects.erase(m_objects.begin() + index);
    }
}

void Scene::removeLight(Renderer::Lighting::Light* light) {
    auto it = std::find(m_lights.begin(), m_lights.end(), light);
    if (it != m_lights.end()) {
        m_lights.erase(it);
    }
}

void Scene::clear() {
    m_objects.clear();
    m_lights.clear();
    m_camera = nullptr;
}

std::vector<Triangle> Scene::collectVisibleTriangles(const Matrix4& viewProjectionMatrix, bool performCulling) const {
    std::vector<Triangle> allTriangles;
    
    // 收集所有对象的三角形
    for (const auto& object : m_objects) {
        if (!object.visible || !object.mesh) continue;
        
        // 获取变换后的三角形
        std::vector<Triangle> objectTriangles = object.mesh->getTransformedTriangles();
        
        // 应用对象变换
        for (auto& triangle : objectTriangles) {
            triangle.transform(object.transform);
        }
        
        allTriangles.insert(allTriangles.end(), objectTriangles.begin(), objectTriangles.end());
    }
    
    m_totalTriangles = static_cast<int>(allTriangles.size());
    
    // 添加调试信息
    static int debugFrame = 0;
    if (debugFrame % 60 == 0) {
        std::cout << "🔍 三角形收集调试:" << std::endl;
        std::cout << "   总三角形数: " << m_totalTriangles << std::endl;
        if (!allTriangles.empty()) {
            std::cout << "   第一个三角形顶点: (" 
                      << allTriangles[0].vertices[0].position.x << ", "
                      << allTriangles[0].vertices[0].position.y << ", "
                      << allTriangles[0].vertices[0].position.z << ")" << std::endl;
        }
    }
    debugFrame++;
    
    if (!performCulling || !m_camera) {
        return allTriangles;
    }
    
    // 执行视锥体裁剪
    const Frustum& frustum = m_camera->getFrustum();
    std::vector<Triangle> visibleTriangles = performFrustumCulling(allTriangles, frustum);
    
    // 执行背面剔除
    Vector3 viewDirection = m_camera->getForward();
    std::vector<Triangle> frontFacingTriangles = performBackfaceCulling(visibleTriangles, viewDirection);
    
    // 按深度排序
    Vector3 viewPosition = m_camera->getPosition();
    std::vector<Triangle> sortedTriangles = sortTrianglesByDepth(frontFacingTriangles, viewPosition);
    
    m_renderedTriangles = static_cast<int>(sortedTriangles.size());
    m_culledTriangles = m_totalTriangles - m_renderedTriangles;
    
    return sortedTriangles;
}

std::vector<Triangle> Scene::performFrustumCulling(const std::vector<Triangle>& triangles, const Frustum& frustum) const {
    std::vector<Triangle> visibleTriangles;
    
    for (const auto& triangle : triangles) {
        if (isTriangleVisible(triangle, frustum)) {
            visibleTriangles.push_back(triangle);
        }
    }
    
    return visibleTriangles;
}

std::vector<Triangle> Scene::performBackfaceCulling(const std::vector<Triangle>& triangles, const Vector3& viewDirection) const {
    std::vector<Triangle> frontFacingTriangles;
    
    for (const auto& triangle : triangles) {
        if (!triangle.isBackface(viewDirection)) {
            frontFacingTriangles.push_back(triangle);
        }
    }
    
    return frontFacingTriangles;
}

std::vector<Triangle> Scene::sortTrianglesByDepth(const std::vector<Triangle>& triangles, const Vector3& viewPosition) const {
    std::vector<Triangle> sortedTriangles = triangles;
    
    // 按深度从远到近排序（画家算法）
    std::sort(sortedTriangles.begin(), sortedTriangles.end(), 
              [this, &viewPosition](const Triangle& a, const Triangle& b) {
                  return getTriangleDistance(a, viewPosition) > getTriangleDistance(b, viewPosition);
              });
    
    return sortedTriangles;
}

void Scene::updateBoundingBoxes() {
    for (auto& object : m_objects) {
        if (object.mesh) {
            object.mesh->calculateBoundingBox();
        }
    }
}

BoundingBox Scene::calculateSceneBounds() const {
    BoundingBox sceneBounds;
    bool hasBounds = false;
    
    for (const auto& object : m_objects) {
        if (object.mesh) {
            BoundingBox objectBounds = object.mesh->getBoundingBox();
            
            // 应用对象变换到包围盒
            Vector3 transformedMin = object.transform.transformPoint(objectBounds.min);
            Vector3 transformedMax = object.transform.transformPoint(objectBounds.max);
            
            if (!hasBounds) {
                sceneBounds = BoundingBox(transformedMin, transformedMax);
                hasBounds = true;
            } else {
                sceneBounds.expand(transformedMin);
                sceneBounds.expand(transformedMax);
            }
        }
    }
    
    return sceneBounds;
}

int Scene::raycast(const Vector3& rayOrigin, const Vector3& rayDirection, float maxDistance) const {
    float closestDistance = maxDistance;
    int closestObject = -1;
    
    for (size_t i = 0; i < m_objects.size(); i++) {
        const auto& object = m_objects[i];
        if (!object.visible || !object.mesh) continue;
        
        // 简化的射线检测：检查包围盒
        BoundingBox bounds = object.mesh->getBoundingBox();
        Vector3 transformedMin = object.transform.transformPoint(bounds.min);
        Vector3 transformedMax = object.transform.transformPoint(bounds.max);
        
        // 这里应该实现完整的射线-包围盒相交测试
        // 简化实现：检查射线是否与包围盒相交
        float t1 = (transformedMin.x - rayOrigin.x) / rayDirection.x;
        float t2 = (transformedMax.x - rayOrigin.x) / rayDirection.x;
        float tmin = std::min(t1, t2);
        float tmax = std::max(t1, t2);
        
        t1 = (transformedMin.y - rayOrigin.y) / rayDirection.y;
        t2 = (transformedMax.y - rayOrigin.y) / rayDirection.y;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
        
        t1 = (transformedMin.z - rayOrigin.z) / rayDirection.z;
        t2 = (transformedMax.z - rayOrigin.z) / rayDirection.z;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
        
        if (tmax >= tmin && tmin >= 0 && tmin < closestDistance) {
            closestDistance = tmin;
            closestObject = static_cast<int>(i);
        }
    }
    
    return closestObject;
}

bool Scene::isTriangleVisible(const Triangle& triangle, const Frustum& frustum) const {
    // 检查三角形的三个顶点是否在视锥体内
    for (int i = 0; i < 3; i++) {
        if (!frustum.isPointInside(triangle.vertices[i].position)) {
            return false;
        }
    }
    return true;
}

float Scene::getTriangleDistance(const Triangle& triangle, const Vector3& viewPosition) const {
    // 计算三角形中心到视点的距离
    Vector3 center = (triangle.vertices[0].position + 
                     triangle.vertices[1].position + 
                     triangle.vertices[2].position) / 3.0f;
    
    return (center - viewPosition).length();
}

void Scene::render(Camera* camera, Renderer::Lighting::Light* light, 
                   Renderer::Pipeline::Rasterizer3D* rasterizer, 
                   Renderer::Pipeline::DepthBuffer* depthBuffer) {
    if (!camera || !rasterizer || !depthBuffer) {
        return;
    }
    
    // 清空深度缓冲
    depthBuffer->clear();
    
    // 获取视图投影矩阵
    Matrix4 viewProjectionMatrix = camera->getViewProjectionMatrix();
    
    // 收集可见三角形 - 暂时禁用视锥体裁剪进行测试
    std::vector<Triangle> visibleTriangles = collectVisibleTriangles(viewProjectionMatrix, false);
    
    // 渲染每个三角形
    for (const auto& triangle : visibleTriangles) {
        rasterizer->rasterizeTriangle(triangle);
    }
}

} // namespace Scene
