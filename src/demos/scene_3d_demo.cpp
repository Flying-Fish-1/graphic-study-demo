#include "scene_3d_demo.h"
#include "../scene/scene.h"
#include <iostream>

Scene3DDemo::Scene3DDemo(SDLWrapper* platform) 
    : m_scene(nullptr), m_camera(nullptr), m_depthBuffer(nullptr), m_rasterizer(nullptr),
      m_platform(platform), m_cube(nullptr), m_triangle(nullptr), m_redMaterial(nullptr),
      m_blueMaterial(nullptr), m_pointLight(nullptr), m_dirLight(nullptr),
      m_wireframeMode(false), m_rotateObjects(true), m_currentObject(0), m_rotationSpeed(1.0f) {
}

Scene3DDemo::~Scene3DDemo() {
    cleanup();
}

bool Scene3DDemo::initialize() {
    std::cout << "🔧 初始化3D场景演示..." << std::endl;
    
    try {
        // 创建深度缓冲
        m_depthBuffer = new DepthBuffer(m_platform->getWidth(), m_platform->getHeight());
        std::cout << "✓ 深度缓冲创建成功" << std::endl;
        
        // 创建3D光栅化器
        m_rasterizer = new Rasterizer3D(m_platform, m_depthBuffer);
        std::cout << "✓ 3D光栅化器创建成功" << std::endl;
        
        // 创建场景对象
        createSceneObjects();
        std::cout << "✓ 场景对象创建成功" << std::endl;
        
        // 创建材质
        createMaterials();
        std::cout << "✓ 材质创建成功" << std::endl;
        
        // 创建光源
        createLights();
        std::cout << "✓ 光源创建成功" << std::endl;
        
        // 设置相机
        setupCamera();
        std::cout << "✓ 相机设置成功" << std::endl;
        
        // 创建场景
        m_scene = new Scene3D();
        m_scene->setCamera(m_camera);
        m_scene->addObject(m_cube, Matrix4::identity());
        m_scene->addLight(m_pointLight);
        std::cout << "✓ 场景创建成功" << std::endl;
        
        std::cout << "✅ 3D场景演示初始化完成" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 3D场景演示初始化失败: " << e.what() << std::endl;
        return false;
    }
}

void Scene3DDemo::update(float deltaTime) {
    if (m_rotateObjects) {
        updateTransforms(deltaTime);
    }
}

void Scene3DDemo::render(float time) {
    if (!m_scene || !m_camera || !m_rasterizer || !m_depthBuffer) {
        std::cerr << "⚠️  3D场景演示未完全初始化" << std::endl;
        return;
    }
    
    // 渲染场景
    if (m_platform->beginRenderSession()) {
        // 清空屏幕
        m_platform->clearScreen(0, 0, 0, 255);
        
        // 添加调试信息
        static int debugFrame = 0;
        if (debugFrame % 60 == 0) {
            std::cout << "🔍 调试信息:" << std::endl;
            std::cout << "   相机位置: " << m_camera->getPosition().x << ", " 
                      << m_camera->getPosition().y << ", " << m_camera->getPosition().z << std::endl;
            std::cout << "   场景对象数: " << m_scene->getObjects().size() << std::endl;
            std::cout << "   光源数: " << m_scene->getLights().size() << std::endl;
        }
        debugFrame++;
        
        // 渲染3D场景
        m_scene->render(m_camera, m_pointLight, m_rasterizer, m_depthBuffer);
        
        m_platform->endRenderSession();
    }
    
    // 显示渲染统计
    static int frameCount = 0;
    if (frameCount % 60 == 0) {
        std::cout << "📊 渲染统计 - 总三角形: " << m_scene->getTotalTriangles() 
                  << ", 渲染: " << m_scene->getRenderedTriangles() 
                  << ", 剔除: " << m_scene->getCulledTriangles() << std::endl;
    }
    frameCount++;
}

void Scene3DDemo::handleKeyInput(int key) {
    switch (key) {
        case 'W':
            toggleWireframe();
            break;
        case 'R':
            toggleRotation();
            break;
        case 'O':
            switchObject();
            break;
        case 'C':
            resetCamera();
            break;
    }
}

void Scene3DDemo::toggleWireframe() {
    m_wireframeMode = !m_wireframeMode;
    if (m_rasterizer) {
        m_rasterizer->setWireframeMode(m_wireframeMode);
    }
    std::cout << "🎨 线框模式: " << (m_wireframeMode ? "开启" : "关闭") << std::endl;
}

void Scene3DDemo::toggleRotation() {
    m_rotateObjects = !m_rotateObjects;
    std::cout << "🔄 对象旋转: " << (m_rotateObjects ? "开启" : "关闭") << std::endl;
}

void Scene3DDemo::switchObject() {
    m_currentObject = (m_currentObject + 1) % 2;
    std::cout << "🎯 切换对象: " << (m_currentObject == 0 ? "立方体" : "三角形") << std::endl;
}

void Scene3DDemo::resetCamera() {
    if (m_camera) {
        m_camera->setPosition(Vector3(0.0f, 0.0f, -8.0f));
        m_camera->lookAt(Vector3(0.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    }
    std::cout << "📷 相机位置已重置" << std::endl;
}

void Scene3DDemo::createSceneObjects() {
    // 创建立方体
    m_cube = Mesh::createCube(2.0f);
    
    // 创建三角形
    m_triangle = Mesh::createTriangle(2.0f);
}

void Scene3DDemo::createMaterials() {
    // 创建红色材质
    m_redMaterial = new Material();
    m_redMaterial->setAmbient(Color(0.2f, 0.0f, 0.0f, 1.0f));
    m_redMaterial->setDiffuse(Color(1.0f, 0.0f, 0.0f, 1.0f));
    m_redMaterial->setSpecular(Color(1.0f, 1.0f, 1.0f, 1.0f));
    m_redMaterial->setShininess(32.0f);
    
    // 创建蓝色材质
    m_blueMaterial = new Material();
    m_blueMaterial->setAmbient(Color(0.0f, 0.0f, 0.2f, 1.0f));
    m_blueMaterial->setDiffuse(Color(0.0f, 0.0f, 1.0f, 1.0f));
    m_blueMaterial->setSpecular(Color(1.0f, 1.0f, 1.0f, 1.0f));
    m_blueMaterial->setShininess(32.0f);
    
    // 设置材质
    if (m_cube) m_cube->setMaterial(m_redMaterial);
    if (m_triangle) m_triangle->setMaterial(m_blueMaterial);
}

void Scene3DDemo::createLights() {
    // 创建点光源
    m_pointLight = new PointLight(Vector3(3.0f, 3.0f, 3.0f), Color(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);
    
    // 创建方向光源
    m_dirLight = new DirectionalLight(Vector3(0.0f, -1.0f, 0.0f), Color(0.5f, 0.5f, 0.5f, 1.0f), 0.5f);
}

void Scene3DDemo::setupCamera() {
    m_camera = new Camera();
    // 调整相机位置，让物体更容易看到
    m_camera->setPosition(Vector3(0.0f, 0.0f, -8.0f));  // 从-5改为-8，离物体更远
    m_camera->setPerspective(Constants::PI / 4.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    m_camera->lookAt(Vector3(0.0f, 0.0f, -8.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
}

void Scene3DDemo::updateTransforms(float time) {
    if (!m_scene) return;
    
    // 更新立方体旋转
    Matrix4 rotationMatrix = Matrix4::rotationY(time * m_rotationSpeed) * Matrix4::rotationX(time * m_rotationSpeed * 0.7f);
    m_scene->getObject(0).transform = rotationMatrix;
}

void Scene3DDemo::renderScene() {
    if (!m_scene || !m_camera || !m_rasterizer || !m_depthBuffer) return;
    
    // 渲染3D场景
    m_scene->render(m_camera, m_pointLight, m_rasterizer, m_depthBuffer);
}

void Scene3DDemo::cleanup() {
    std::cout << "🧹 清理3D场景演示资源..." << std::endl;
    
    delete m_scene;
    delete m_camera;
    delete m_depthBuffer;
    delete m_rasterizer;
    delete m_cube;
    delete m_triangle;
    delete m_redMaterial;
    delete m_blueMaterial;
    delete m_pointLight;
    delete m_dirLight;
    
    m_scene = nullptr;
    m_camera = nullptr;
    m_depthBuffer = nullptr;
    m_rasterizer = nullptr;
    m_cube = nullptr;
    m_triangle = nullptr;
    m_redMaterial = nullptr;
    m_blueMaterial = nullptr;
    m_pointLight = nullptr;
    m_dirLight = nullptr;
    
    std::cout << "✅ 3D场景演示资源清理完成" << std::endl;
}
