#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "scene/scene.h"
#include "scene/camera.h"
#include "scene/mesh.h"
#include "core/types/material.h"
#include "core/types/texture.h"
#include "renderer/pipeline/software_renderer.h"
#include "renderer/lighting/light.h"
#include "renderer/preview/sdl_preview.h"
#ifdef ENABLE_SDL_PREVIEW
#include <SDL2/SDL.h>
#endif

namespace {
struct RenderOptions {
    int width = 1280;
    int height = 720;
    float cameraDistance = 6.0f;
    bool previewWindow =
#ifdef ENABLE_SDL_PREVIEW
        true;
#else
        false;
#endif
    bool saveImage = false;
    std::string outputFile = "output.ppm";
};

RenderOptions parseOptions(int argc, char** argv) {
    RenderOptions opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        auto assignInt = [&arg](const std::string& prefix) -> std::optional<int> {
            if (arg.rfind(prefix, 0) == 0) {
                return std::stoi(arg.substr(prefix.size()));
            }
            return {};
        };
        auto assignFloat = [&arg](const std::string& prefix) -> std::optional<float> {
            if (arg.rfind(prefix, 0) == 0) {
                return std::stof(arg.substr(prefix.size()));
            }
            return {};
        };

        if (auto value = assignInt("--width=")) {
            opts.width = std::max(1, *value);
        } else if (auto value = assignInt("--height=")) {
            opts.height = std::max(1, *value);
        } else if (arg.rfind("--output=", 0) == 0) {
            opts.outputFile = arg.substr(std::string("--output=").size());
            opts.saveImage = true;
        } else if (auto value = assignFloat("--camera-distance=")) {
            opts.cameraDistance = std::max(0.1f, *value);
        } else if (arg == "--preview") {
            opts.previewWindow = true;
        } else if (arg == "--no-preview") {
            opts.previewWindow = false;
        } else if (arg == "--save") {
            opts.saveImage = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "用法: " << argv[0]
                      << " [--width=<像素>] [--height=<像素>] [--output=<文件>] [--save] [--camera-distance=<值>] [--preview|--no-preview]" << std::endl;
            std::exit(0);
        } else {
            std::cerr << "未知参数: " << arg << std::endl;
            std::exit(1);
        }
    }
    return opts;
}
}

int main(int argc, char** argv) {
    const RenderOptions options = parseOptions(argc, argv);

    Scene::Scene scene;

    auto camera = std::make_unique<Scene::Camera>();
    camera->setPerspective(Core::Math::Constants::PI / 3.0f, static_cast<float>(options.width) / static_cast<float>(options.height), 0.1f, 100.0f);
    camera->lookAt(Core::Math::Vector3(3.0f, 3.0f, options.cameraDistance),
                   Core::Math::Vector3(0.0f, 0.0f, 0.0f),
                   Core::Math::Vector3(0.0f, 1.0f, 0.0f));
    scene.setCamera(camera.get());
    scene.setAmbientLight(Core::Types::Color(0.5f, 0.5f, 0.5f, 1.0f));

    auto mesh = std::unique_ptr<Scene::Mesh>(Scene::Mesh::createHollowCube(3.0f, 2.2f));
    // 额外添加一个颜色渐变的三角形球体
    auto gradSphere = std::unique_ptr<Scene::Mesh>(Scene::Mesh::createGradientSphere(1.8f, 64,
        Core::Types::Color(1.0f, 0.0f, 0.0f, 1.0f),
        Core::Types::Color(0.0f, 0.5f, 1.0f, 1.0f)));

    auto material = std::unique_ptr<Core::Types::Material>(Core::Types::Material::createRedPlastic());
    //auto texture = std::unique_ptr<Core::Types::Texture>(Core::Types::Texture::createSolidColor(Core::Types::Color::RED, 256, 256));
    //material->setDiffuseMap(texture.get());
    material->setDiffuse(Core::Types::Color(1.0f, 1.0f, 1.0f, 0.3f)); // 50% 透明
    material->setSpecular(Core::Types::Color(1.0f, 1.0f, 1.0f, 0.8f));
    material->setShininess(84.0f);
    mesh->setMaterial(material.get());
    // 为球体使用高光更明显的材质
    auto sphereMat = std::unique_ptr<Core::Types::Material>(Core::Types::Material::createWhiteDiffuse());
    sphereMat->setSpecular(Core::Types::Color(0.2f, 0.2f, 0.2f, 1.0f));
    sphereMat->setShininess(32.0f);
    gradSphere->setMaterial(sphereMat.get());

    int cubeIndex = scene.addObject(mesh.get());
    // 将球体放到立方体旁边
    Scene::SceneObject rightObj;
    Core::Math::Matrix4 sphereX = Core::Math::Matrix4::translation(3.5f, 0.0f, 0.0f);
    int sphereIndex = scene.addObject(gradSphere.get(), sphereX);

    auto pointLight = std::make_unique<Renderer::Lighting::PointLight>(Core::Math::Vector3(2.5f, 2.5f, options.cameraDistance - 0.5f), Core::Types::Color::WHITE, 5.0f, 18.0f);
    auto dirLight = std::make_unique<Renderer::Lighting::DirectionalLight>(Core::Math::Vector3(-1.0f, -1.0f, -1.0f), Core::Types::Color(1.0f, 0.95f, 0.85f, 1.0f), 0.4f);
    scene.addLight(pointLight.get());
    scene.addLight(dirLight.get());

    Renderer::Pipeline::SoftwareRendererSettings settings;
    settings.width = options.width;
    settings.height = options.height;
    settings.ssaaFactor = 1;
    //settings.fresnelForTranslucent = true;
    //settings.fresnelF0 = 0.04f;

    Renderer::Pipeline::SoftwareRenderer renderer(settings);

#ifdef ENABLE_SDL_PREVIEW
    if (options.previewWindow) {
        Renderer::Preview::SdlPreview preview(settings.width, settings.height);
        if (preview.initialize()) {
            bool running = true;
            uint32_t lastTicks = SDL_GetTicks();
            float angle = 0.0f;
            while (running) {
                running = preview.pollEvents();
                uint32_t now = SDL_GetTicks();
                float dt = (now - lastTicks) * 0.001f;
                lastTicks = now;

                angle += dt;
                Core::Math::Matrix4 rotY = Core::Math::Matrix4::rotationY(angle);
                Core::Math::Matrix4 rotX = Core::Math::Matrix4::rotationX(angle * 0.5f);
                scene.setObjectTransform(cubeIndex, rotY * rotX);

                renderer.render(scene);
                preview.presentOnce(renderer.getRenderTarget(), "软件渲染预览 - 旋转立方体");

                SDL_Delay(16);
            }
        } else {
            std::cerr << "SDL 预览初始化失败，跳过窗口显示" << std::endl;
        }
    } else {
        renderer.render(scene);
    }
#else
    renderer.render(scene);
#endif


    if (options.saveImage) {
        if (renderer.getRenderTarget().savePPM(options.outputFile)) {
            std::cout << "渲染完成，输出文件: " << options.outputFile << std::endl;
        } else {
            std::cerr << "写入输出文件失败: " << options.outputFile << std::endl;
            return 1;
        }
    }

#ifdef ENABLE_SDL_PREVIEW
    if (options.previewWindow) {
        Renderer::Preview::SdlPreview preview(settings.width, settings.height);
        if (preview.initialize()) {
            preview.present(renderer.getRenderTarget(), "软件渲染预览");
        } else {
            std::cerr << "SDL 预览初始化失败，跳过窗口显示" << std::endl;
        }
    }
#else
    if (options.previewWindow) {
        Renderer::Preview::SdlPreview preview(settings.width, settings.height);
        preview.present(renderer.getRenderTarget(), "软件渲染预览");
    }
#endif

    return 0;
}
