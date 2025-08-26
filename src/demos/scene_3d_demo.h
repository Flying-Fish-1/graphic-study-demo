#ifndef SCENE_3D_DEMO_H
#define SCENE_3D_DEMO_H

#include "../scene/mesh.h"
#include "../scene/camera.h"

// 前向声明
namespace Scene {
    class Scene;
}
typedef Scene::Scene Scene3D;
#include "../renderer/pipeline/depth_buffer.h"
#include "../renderer/pipeline/rasterizer_3d.h"
#include "../renderer/lighting/light.h"
#include "../core/platform/sdl_wrapper.h"
#include "../core/types/material.h"

using namespace Scene;
using namespace Core::Platform;
using namespace Core::Types;
using namespace Renderer::Pipeline;
using namespace Renderer::Lighting;

/**
 * @brief 3D场景演示类
 * 
 * 展示新的3D渲染架构的完整功能
 */
class Scene3DDemo {
private:
    Scene3D* m_scene;                   // 3D场景
    Camera* m_camera;                   // 相机
    DepthBuffer* m_depthBuffer;         // 深度缓冲
    Rasterizer3D* m_rasterizer;         // 3D光栅化器
    SDLWrapper* m_platform;             // 平台接口
    
    // 场景对象
    Mesh* m_cube;                       // 立方体网格
    Mesh* m_triangle;                   // 三角形网格
    Material* m_redMaterial;            // 红色材质
    Material* m_blueMaterial;           // 蓝色材质
    PointLight* m_pointLight;           // 点光源
    DirectionalLight* m_dirLight;       // 方向光源
    
    // 渲染状态
    bool m_wireframeMode;               // 线框模式
    bool m_rotateObjects;               // 旋转对象
    int m_currentObject;                // 当前显示的对象
    float m_rotationSpeed;              // 旋转速度
    
public:
    /**
     * @brief 构造函数
     * @param platform 平台接口
     */
    Scene3DDemo(SDLWrapper* platform);
    
    /**
     * @brief 析构函数
     */
    ~Scene3DDemo();
    
    /**
     * @brief 初始化演示
     * @return 是否初始化成功
     */
    bool initialize();
    
    /**
     * @brief 更新演示
     * @param deltaTime 时间增量
     */
    void update(float deltaTime);
    
    /**
     * @brief 渲染演示
     * @param time 总时间
     */
    void render(float time);
    
    /**
     * @brief 处理键盘输入
     * @param key 按键代码
     */
    void handleKeyInput(int key);
    
    /**
     * @brief 切换线框模式
     */
    void toggleWireframe();
    
    /**
     * @brief 切换对象旋转
     */
    void toggleRotation();
    
    /**
     * @brief 切换显示对象
     */
    void switchObject();
    
    /**
     * @brief 重置相机位置
     */
    void resetCamera();
    
private:
    /**
     * @brief 创建场景对象
     */
    void createSceneObjects();
    
    /**
     * @brief 创建材质
     */
    void createMaterials();
    
    /**
     * @brief 创建光源
     */
    void createLights();
    
    /**
     * @brief 设置相机
     */
    void setupCamera();
    
    /**
     * @brief 更新对象变换
     * @param time 时间
     */
    void updateTransforms(float time);
    
    /**
     * @brief 渲染场景
     */
    void renderScene();
    
    /**
     * @brief 清理资源
     */
    void cleanup();
};

#endif // SCENE_3D_DEMO_H
