#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <memory>

// 新架构模块
#include "core/math/vector.h"
#include "core/math/matrix.h"
#include "core/types/color.h"
#include "core/platform/sdl_wrapper.h"
#include "graphics/graphics.h"
#include "renderer/effects/antialiasing.h"
#include "demos/transform_demo.h"
#include "demos/scene_3d_demo.h"
#include "core/platform/constants.h"

using namespace Core::Math;
using namespace Core::Types;
using namespace Core::Platform;
using namespace Renderer::Effects;

/**
 * @brief 完全使用新架构的图形学演示应用
 */
class GraphicsDemo {
public:
    enum DemoMode {
        ORIGINAL_TRIANGLE,
        ROTATING_TRIANGLES,
        TRANSFORM_DEMO,
        VECTOR_DEMO,
        PROJECTION_3D,
        SCENE_3D_DEMO,  // 新增3D场景演示
        DEMO_MODE_COUNT
    };

private:
    std::unique_ptr<SDLWrapper> m_platform;
    std::unique_ptr<Graphics> m_graphics;
    std::unique_ptr<AntiAliasing> m_antiAliasing;
    std::unique_ptr<TransformDemo> m_transformDemo;
    std::unique_ptr<Scene3DDemo> m_scene3DDemo;  // 新增3D演示
    
    bool m_isRunning;
    int m_frameCount;
    DemoMode m_currentDemo;
    
public:
    GraphicsDemo() : m_isRunning(false), m_frameCount(0), m_currentDemo(ORIGINAL_TRIANGLE) {}
    
    bool initialize();
    void run();
    void shutdown();
    
private:
    void handleEvents();
    void update(float deltaTime);
    void render(float time);
    void updateWindowTitle();
    
    void renderOriginalTriangle();
    void renderRotatingTriangles(float time);
    void renderTransformDemo(float time);
    void renderVectorDemo(float time);
    void renderProjection3D(float time);
    void renderScene3DDemo(float time);  // 新增3D场景渲染
};

bool GraphicsDemo::initialize() {
    std::cout << "🔧 开始初始化..." << std::endl;
    
    m_platform = std::make_unique<SDLWrapper>();
    std::cout << "✓ 创建SDL包装器" << std::endl;
    
    if (!m_platform->initSDL()) {
        std::cerr << "❌ SDL初始化失败" << std::endl;
        return false;
    }
    std::cout << "✓ SDL初始化成功" << std::endl;
    
    if (!m_platform->createWindow()) {
        std::cerr << "❌ 窗口创建失败" << std::endl;
        return false;
    }
    std::cout << "✓ 窗口创建成功" << std::endl;
    
    if (!m_platform->createRenderer()) {
        std::cerr << "❌ 渲染器创建失败" << std::endl;
        return false;
    }
    std::cout << "✓ 渲染器创建成功" << std::endl;
    
    if (!m_platform->createFrameBuffer()) {
        std::cerr << "❌ 帧缓冲创建失败" << std::endl;
        return false;
    }
    std::cout << "✓ 帧缓冲创建成功" << std::endl;
    
    m_graphics = std::make_unique<Graphics>(m_platform.get());
    std::cout << "✓ 图形模块创建成功" << std::endl;
    
    m_antiAliasing = std::make_unique<AntiAliasing>();
    std::cout << "✓ 抗锯齿模块创建成功" << std::endl;
    
    m_transformDemo = std::make_unique<TransformDemo>(m_graphics.get());
    std::cout << "✓ 变换演示模块创建成功" << std::endl;
    
    // 初始化3D场景演示
    m_scene3DDemo = std::make_unique<Scene3DDemo>(m_platform.get());
    if (!m_scene3DDemo->initialize()) {
        std::cerr << "❌ 3D场景演示初始化失败" << std::endl;
        return false;
    }
    std::cout << "✓ 3D场景演示模块创建成功" << std::endl;
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "   🏗️ 新架构图形学演示程序启动成功" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "📋 控制说明:" << std::endl;
    std::cout << "   ESC键或关闭窗口 - 退出程序" << std::endl;
    std::cout << "   A键             - 切换抗锯齿方案" << std::endl;
    std::cout << "   上/下箭头键      - 调整超采样倍数" << std::endl;
    std::cout << "   空格键           - 切换演示模式" << std::endl;
    std::cout << "   W键             - 切换线框模式 (3D模式)" << std::endl;
    std::cout << "   R键             - 切换对象旋转 (3D模式)" << std::endl;
    std::cout << "   O键             - 切换显示对象 (3D模式)" << std::endl;
    std::cout << "   C键             - 重置相机位置 (3D模式)" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "🎨 当前配置:" << std::endl;
    std::cout << "   抗锯齿方案: " << m_antiAliasing->getAntiAliasingMethodName() << std::endl;
    std::cout << "   超采样倍数: " << m_antiAliasing->getSsaaScale() << "x" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    updateWindowTitle();
    m_isRunning = true;
    std::cout << "🚀 准备进入主循环..." << std::endl;
    return true;
}

void GraphicsDemo::run() {
    while (m_isRunning) {
        handleEvents();
        
        float time = m_frameCount * 0.01f;
        update(time);
        render(time);
        
        SDL_Delay(16); // 60 FPS
        m_frameCount++;
    }
}

void GraphicsDemo::shutdown() {
    m_scene3DDemo.reset();
    m_transformDemo.reset();
    m_antiAliasing.reset();
    m_graphics.reset();
    
    if (m_platform) {
        m_platform->cleanup();
        m_platform.reset();
    }
    
    std::cout << "✅ 新架构程序已安全退出" << std::endl;
}

void GraphicsDemo::handleEvents() {
    SDL_Event e;
    
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
            m_isRunning = false;
            } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    m_isRunning = false;
                    break;
                    
                case SDLK_a:
                    m_antiAliasing->switchAntiAliasingMethod();
                    updateWindowTitle();
                    std::cout << "🔧 抗锯齿方案: " << m_antiAliasing->getAntiAliasingMethodName() << std::endl;
                    break;
                    
                case SDLK_UP:
                    m_antiAliasing->increaseSsaaScale();
                    updateWindowTitle();
                    std::cout << "📈 超采样倍数: " << m_antiAliasing->getSsaaScale() << "x" << std::endl;
                    break;
                    
                case SDLK_DOWN:
                    m_antiAliasing->decreaseSsaaScale();
                    updateWindowTitle();
                    std::cout << "📉 超采样倍数: " << m_antiAliasing->getSsaaScale() << "x" << std::endl;
                    break;
                    
                case SDLK_SPACE: {
                    m_currentDemo = static_cast<DemoMode>((m_currentDemo + 1) % DEMO_MODE_COUNT);
                    const char* demoNames[] = {
                        "🔺 原始三角形", "🌟 旋转三角形", "🔄 变换演示", 
                        "➡️ 向量演示", "🎯 3D投影", "🎮 3D场景"
                    };
                    std::cout << "🎮 切换到演示模式: " << demoNames[m_currentDemo] << std::endl;
                    break;
                }
                
                // 3D场景演示控制
                case SDLK_w:
                    if (m_currentDemo == SCENE_3D_DEMO && m_scene3DDemo) {
                        m_scene3DDemo->toggleWireframe();
                    }
                    break;
                    
                case SDLK_r:
                    if (m_currentDemo == SCENE_3D_DEMO && m_scene3DDemo) {
                        m_scene3DDemo->toggleRotation();
                    }
                    break;
                    
                case SDLK_o:
                    if (m_currentDemo == SCENE_3D_DEMO && m_scene3DDemo) {
                        m_scene3DDemo->switchObject();
                    }
                    break;
                    
                case SDLK_c:
                    if (m_currentDemo == SCENE_3D_DEMO && m_scene3DDemo) {
                        m_scene3DDemo->resetCamera();
                    }
                    break;
                
                default:
                    std::cout << "⌨️  按下了键: " << SDL_GetKeyName(e.key.keysym.sym) << std::endl;
            break;
        }
        }
    }
}

void GraphicsDemo::update(float deltaTime) {
    // 更新3D场景演示
    if (m_currentDemo == SCENE_3D_DEMO && m_scene3DDemo) {
        m_scene3DDemo->update(deltaTime);
    }
}

void GraphicsDemo::render(float time) {
    m_platform->clearScreen(0, 0, 0, 255);
    
    switch (m_currentDemo) {
        case ORIGINAL_TRIANGLE:
            renderOriginalTriangle();
            break;
            
        case ROTATING_TRIANGLES:
            renderRotatingTriangles(time);
            break;
            
        case TRANSFORM_DEMO:
            renderTransformDemo(time);
            break;
            
        case VECTOR_DEMO:
            renderVectorDemo(time);
            break;
            
        case PROJECTION_3D:
            renderProjection3D(time);
            break;
            
        case SCENE_3D_DEMO:
            renderScene3DDemo(time);
            break;
            
        case DEMO_MODE_COUNT:
            break;
    }
    
    // 抗锯齿后处理 - 在渲染会话外进行
    AntiAliasingMethod currentMethod = m_antiAliasing->getCurrentMethod();
    if (currentMethod != NONE) {
        if (m_platform->beginRenderSession()) {
            m_antiAliasing->applyAntiAliasing(m_platform->getPixelBuffer(), 
                                             m_platform->getWidth(), m_platform->getHeight());
            m_platform->endRenderSession();
        }
    }
    
    m_platform->updateScreen();
}

void GraphicsDemo::renderOriginalTriangle() {
    // 使用新的渲染会话机制进行高效封装
    if (m_platform->beginRenderSession()) {
        // 现在可以高效地使用setPixel，无需担心性能问题
        
        // 绘制测试像素
        m_platform->setPixel(100, 100, Color::RED);
        m_platform->setPixel(101, 100, Color::GREEN);
        m_platform->setPixel(102, 100, Color::BLUE);
        m_platform->setPixel(103, 100, Color::WHITE);
        m_platform->setPixel(104, 100, Color::YELLOW);
        
        // 绘制一个小正方形
        for (int y = 150; y < 160; y++) {
            for (int x = 200; x < 210; x++) {
                m_platform->setPixel(x, y, Color::CYAN);
            }
        }
        
        // 现在可以恢复三角形绘制！
        Color red = Color::RED;
        Color green = Color::GREEN;
        Color blue = Color::BLUE;

        int x0 = 50, y0 = 100;
        int x1 = 170, y1 = 350;
        int x2 = 400, y2 = 300;

        // 使用基础光栅化绘制三角形
        m_graphics->rasterizeTriangle(x0, y0, x1, y1, x2, y2, red, green, blue);
        
        m_platform->endRenderSession();
    }
    
    static int frame = 0;
    if (frame % 60 == 0) {
        std::cout << "渲染帧 " << frame << " - 高效封装渲染" << std::endl;
    }
    frame++;
}

void GraphicsDemo::renderRotatingTriangles(float time) {
    if (m_platform->beginRenderSession()) {
        Color colors[] = {
            Color::RED, Color::GREEN, Color::BLUE, Color::YELLOW, Color::MAGENTA
        };
        
        for (int i = 0; i < 5; i++) {
            float angle = time + i * Constants::TAU / 5.0f;
            float scale = 0.8f + 0.3f * std::sin(time * 2.0f + i);
            float centerX = 400 + 150 * std::cos(angle);
            float centerY = 300 + 100 * std::sin(angle);
            
            m_transformDemo->drawRotatedTriangle(centerX, centerY, angle * 2.0f, scale, colors[i]);
        }
        
        m_platform->endRenderSession();
    }
}

void GraphicsDemo::renderTransformDemo(float time) {
    if (m_platform->beginRenderSession()) {
        Color cyan = Color::CYAN;
        Color magenta = Color::MAGENTA;
        
        m_transformDemo->drawTransformedQuad(200, 150, time, 1.0f, 1.0f, cyan);
        
        float scaleX = 1.0f + 0.5f * std::sin(time * 2.0f);
        float scaleY = 1.0f + 0.5f * std::cos(time * 1.5f);
        m_transformDemo->drawTransformedQuad(600, 150, time * 0.7f, scaleX, scaleY, magenta);
        
        m_transformDemo->drawTransformedQuad(400, 400, -time * 1.2f, 
                                          0.8f + 0.4f * std::cos(time * 3.0f), 
                                          0.8f + 0.4f * std::sin(time * 2.5f), 
                                          Color::WHITE);
        
        m_platform->endRenderSession();
    }
}

void GraphicsDemo::renderVectorDemo(float time) {
    if (m_platform->beginRenderSession()) {
        m_transformDemo->drawVectorDemo(time);
        m_platform->endRenderSession();
    }
}

void GraphicsDemo::renderProjection3D(float time) {
    if (m_platform->beginRenderSession()) {
        m_transformDemo->draw3DProjectionDemo(time);
        m_platform->endRenderSession();
    }
}

void GraphicsDemo::renderScene3DDemo(float time) {
    if (m_scene3DDemo) {
        m_scene3DDemo->render(time);
    }
}

void GraphicsDemo::updateWindowTitle() {
    std::string title = "Graphics Demo [New Architecture] - " + 
                       std::string(m_antiAliasing->getAntiAliasingMethodName()) + " - " + 
                       m_antiAliasing->getSsaaScaleInfo();
    m_platform->setWindowTitle(title.c_str());
}

int main(int argc, char* argv[]) {
    try {
        GraphicsDemo app;
        
        if (!app.initialize()) {
            std::cerr << "❌ 应用初始化失败" << std::endl;
            return -1;
        }
        
        app.run();
        app.shutdown();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "💥 程序异常: " << e.what() << std::endl;
        return -1;
    }
}