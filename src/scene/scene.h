#ifndef SCENE_SCENE_H
#define SCENE_SCENE_H

#include "mesh.h"
#include "camera.h"
#include "../core/types/triangle.h"
#include <vector>

// 前向声明避免包含冲突
namespace Renderer { 
    namespace Lighting { class Light; }
    namespace Pipeline { 
        class Rasterizer3D;
        class DepthBuffer;
    }
}

namespace Scene {

using namespace Core::Types;

/**
 * @brief 场景对象
 * 
 * 表示场景中的一个渲染对象，包含网格和变换信息
 */
struct SceneObject {
    Mesh* mesh;                 // 网格数据
    Matrix4 transform;          // 世界变换矩阵
    bool visible;               // 是否可见
    bool castShadows;           // 是否投射阴影
    bool receiveShadows;        // 是否接收阴影
    
    SceneObject(Mesh* m = nullptr) 
        : mesh(m), transform(Matrix4::identity()), 
          visible(true), castShadows(true), receiveShadows(true) {}
};

/**
 * @brief 3D场景类
 * 
 * 管理场景中的所有对象、光源和相机
 */
class Scene {
private:
    std::vector<SceneObject> m_objects;     // 场景对象
    std::vector<Renderer::Lighting::Light*> m_lights;          // 光源列表
    Camera* m_camera;                       // 主相机
    Color m_backgroundColor;                // 背景颜色
    Color m_ambientLight;                   // 全局环境光
    
    // 渲染统计
    mutable int m_totalTriangles;           // 总三角形数
    mutable int m_renderedTriangles;        // 渲染的三角形数
    mutable int m_culledTriangles;          // 被剔除的三角形数
    
public:
    /**
     * @brief 构造函数
     */
    Scene();
    
    /**
     * @brief 析构函数
     */
    ~Scene();
    
    /**
     * @brief 添加网格对象到场景
     * @param mesh 网格指针
     * @param transform 变换矩阵
     * @return 对象索引
     */
    int addObject(Mesh* mesh, const Matrix4& transform = Matrix4::identity());
    
    /**
     * @brief 添加光源到场景
     * @param light 光源指针
     */
    void addLight(Renderer::Lighting::Light* light);
    
    /**
     * @brief 移除对象
     * @param index 对象索引
     */
    void removeObject(int index);
    
    /**
     * @brief 移除光源
     * @param light 光源指针
     */
    void removeLight(Renderer::Lighting::Light* light);
    
    /**
     * @brief 清空场景
     */
    void clear();
    
    /**
     * @brief 渲染场景
     * @param camera 相机
     * @param light 光源
     * @param rasterizer 3D光栅化器
     * @param depthBuffer 深度缓冲
     */
    void render(Camera* camera, Renderer::Lighting::Light* light, 
                Renderer::Pipeline::Rasterizer3D* rasterizer, 
                Renderer::Pipeline::DepthBuffer* depthBuffer);
    
    /**
     * @brief 设置主相机
     * @param camera 相机指针
     */
    void setCamera(Camera* camera) { m_camera = camera; }
    
    /**
     * @brief 收集所有可见的三角形
     * @param viewProjectionMatrix 视图投影矩阵
     * @param performCulling 是否执行视锥体裁剪
     * @return 可见三角形列表
     */
    std::vector<Triangle> collectVisibleTriangles(const Matrix4& viewProjectionMatrix, 
                                                 bool performCulling = true) const;
    
    /**
     * @brief 执行视锥体裁剪
     * @param triangles 三角形列表
     * @param frustum 视锥体
     * @return 可见三角形列表
     */
    std::vector<Triangle> performFrustumCulling(const std::vector<Triangle>& triangles, 
                                               const Frustum& frustum) const;
    
    /**
     * @brief 执行背面剔除
     * @param triangles 三角形列表
     * @param viewDirection 视线方向
     * @return 前向面三角形列表
     */
    std::vector<Triangle> performBackfaceCulling(const std::vector<Triangle>& triangles, 
                                                const Vector3& viewDirection) const;
    
    /**
     * @brief 按深度排序三角形
     * @param triangles 三角形列表
     * @param viewPosition 视点位置
     * @return 排序后的三角形列表
     */
    std::vector<Triangle> sortTrianglesByDepth(const std::vector<Triangle>& triangles, 
                                              const Vector3& viewPosition) const;
    
    /**
     * @brief 更新所有对象的包围盒
     */
    void updateBoundingBoxes();
    
    /**
     * @brief 计算场景的总包围盒
     * @return 场景包围盒
     */
    BoundingBox calculateSceneBounds() const;
    
    /**
     * @brief 查找最近的相交对象
     * @param rayOrigin 射线起点
     * @param rayDirection 射线方向
     * @param maxDistance 最大距离
     * @return 相交的对象索引，-1表示无相交
     */
    int raycast(const Vector3& rayOrigin, const Vector3& rayDirection, float maxDistance = 1000.0f) const;
    
    // Getter方法
    const std::vector<SceneObject>& getObjects() const { return m_objects; }
    const std::vector<Renderer::Lighting::Light*>& getLights() const { return m_lights; }
    Camera* getCamera() const { return m_camera; }
    const Color& getBackgroundColor() const { return m_backgroundColor; }
    const Color& getAmbientLight() const { return m_ambientLight; }
    
    // 渲染统计
    int getTotalTriangles() const { return m_totalTriangles; }
    int getRenderedTriangles() const { return m_renderedTriangles; }
    int getCulledTriangles() const { return m_culledTriangles; }
    
    // Setter方法
    void setBackgroundColor(const Color& color) { m_backgroundColor = color; }
    void setAmbientLight(const Color& color) { m_ambientLight = color; }
    
    /**
     * @brief 获取指定索引的对象
     * @param index 对象索引
     * @return 对象引用
     */
    SceneObject& getObject(int index) { return m_objects[index]; }
    const SceneObject& getObject(int index) const { return m_objects[index]; }
    
    /**
     * @brief 获取对象数量
     */
    size_t getObjectCount() const { return m_objects.size(); }
    
    /**
     * @brief 获取光源数量
     */
    size_t getLightCount() const { return m_lights.size(); }
    
private:
    /**
     * @brief 检查三角形是否在视锥体内
     * @param triangle 三角形
     * @param frustum 视锥体
     * @return 是否可见
     */
    bool isTriangleVisible(const Triangle& triangle, const Frustum& frustum) const;
    
    /**
     * @brief 计算三角形到视点的距离
     * @param triangle 三角形
     * @param viewPosition 视点位置
     * @return 距离
     */
    float getTriangleDistance(const Triangle& triangle, const Vector3& viewPosition) const;
};

} // namespace Scene

#endif // SCENE_SCENE_H
