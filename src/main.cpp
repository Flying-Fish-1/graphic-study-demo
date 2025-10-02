#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

#include "scene/scene.h"
#include "scene/camera.h"
#include "scene/mesh.h"
#include "core/types/material.h"
#include "core/types/texture.h"
#include "renderer/pipeline/software_renderer.h"
#include "renderer/lighting/light.h"
#include "renderer/preview/sdl_preview.h"
#include "util/ffmpeg_utils.h"
#ifdef ENABLE_SDL_PREVIEW
#include <SDL2/SDL.h>
#endif

namespace {
enum class OutputMode {
    Preview,
    Png,
    Video
};

struct RenderOptions {
    int width = 1280;
    int height = 720;
    float cameraDistance = 6.0f;
    OutputMode mode = OutputMode::Preview;
    std::string outputPath;
    float durationSeconds = 5.0f;
    int fps = 30;
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
            opts.outputPath = arg.substr(std::string("--output=").size());
        } else if (auto value = assignFloat("--camera-distance=")) {
            opts.cameraDistance = std::max(0.1f, *value);
        } else if (arg.rfind("--mode=", 0) == 0) {
            const std::string value = arg.substr(std::string("--mode=").size());
            if (value == "preview") {
                opts.mode = OutputMode::Preview;
            } else if (value == "png") {
                opts.mode = OutputMode::Png;
            } else if (value == "video") {
                opts.mode = OutputMode::Video;
            } else {
                std::cerr << "未知的模式: " << value << std::endl;
                std::exit(1);
            }
        } else if (auto value = assignFloat("--duration=")) {
            opts.durationSeconds = std::max(0.0f, *value);
        } else if (auto value = assignInt("--fps=")) {
            opts.fps = std::max(1, *value);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "用法: " << argv[0]
                      << " --mode=<preview|png|video>"
                      << " [--width=<像素>] [--height=<像素>]"
                      << " [--output=<文件>] [--camera-distance=<值>]"
                      << " [--duration=<秒>] [--fps=<帧率>]" << std::endl;
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

    namespace fs = std::filesystem;

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
    material->setDiffuse(Core::Types::Color(1.0f, 1.0f, 1.0f, 0.2f)); // 50% 透明
    material->setSpecular(Core::Types::Color(1.0f, 1.0f, 1.0f, 0.8f));
    material->setShininess(84.0f);
    mesh->setMaterial(material.get());
    // 为球体使用高光更明显的材质
    auto sphereMat = std::unique_ptr<Core::Types::Material>(Core::Types::Material::createWhiteDiffuse());
    sphereMat->setSpecular(Core::Types::Color(0.2f, 0.2f, 0.2f, 1.0f));
    sphereMat->setShininess(132.0f);
    gradSphere->setMaterial(sphereMat.get());

    int cubeIndex = scene.addObject(mesh.get());
    // 将球体放到立方体更远的位置，给点光源留下穿过空腔的空间
    Core::Math::Matrix4 sphereX = Core::Math::Matrix4::translation(0.0f, 0.0f, -10.0f);
    scene.addObject(gradSphere.get(), sphereX);

    auto pointLight = std::make_unique<Renderer::Lighting::PointLight>(Core::Math::Vector3(0.0f, 0.0f, -5.0f), Core::Types::Color::WHITE, 4.0f, 20.0f);
    auto dirLight = std::make_unique<Renderer::Lighting::DirectionalLight>(Core::Math::Vector3(-1.0f, -1.0f, -1.0f), Core::Types::Color(1.0f, 0.95f, 0.85f, 1.0f), 0.4f);
    scene.addLight(pointLight.get());
    scene.addLight(dirLight.get());

    Renderer::Pipeline::SoftwareRendererSettings settings;
    settings.width = options.width;
    settings.height = options.height;
    settings.ssaaFactor = 2;

    Renderer::Pipeline::SoftwareRenderer renderer(settings);

    auto animateScene = [&](float time) {
        Core::Math::Matrix4 rotY = Core::Math::Matrix4::rotationY(time);
        Core::Math::Matrix4 rotX = Core::Math::Matrix4::rotationX(time * 0.5f);
        scene.setObjectTransform(cubeIndex, rotY * rotX);
    };

    switch (options.mode) {
    case OutputMode::Preview: {
#ifdef ENABLE_SDL_PREVIEW
        Renderer::Preview::SdlPreview preview(settings.width, settings.height);
        if (!preview.initialize()) {
            std::cerr << "SDL 预览初始化失败" << std::endl;
            return 1;
        }

        bool running = true;
        uint32_t lastTicks = SDL_GetTicks();
        float time = 0.0f;
        while (running) {
            running = preview.pollEvents();
            uint32_t now = SDL_GetTicks();
            float dt = (now - lastTicks) * 0.001f;
            lastTicks = now;

            time += dt;
            animateScene(time);

            renderer.render(scene);
            preview.presentOnce(renderer.getRenderTarget(), "软件渲染预览 - 旋转立方体");

            SDL_Delay(16);
        }
        return 0;
#else
        std::cerr << "程序未启用 SDL 预览，无法运行 preview 模式" << std::endl;
        return 1;
#endif
    }
    case OutputMode::Png: {
        fs::path output = options.outputPath.empty() ? fs::path("output.png") : fs::path(options.outputPath);
        if (output.extension().empty()) {
            output.replace_extension(".png");
        }

        animateScene(0.0f);
        renderer.render(scene);

        fs::path ppmPath = output;
        ppmPath.replace_extension(".ppm");
        if (!renderer.getRenderTarget().savePPM(ppmPath.string())) {
            std::cerr << "写入临时 PPM 失败: " << ppmPath << std::endl;
            return 1;
        }

        std::string ffmpegPath;
        if (!Util::Ffmpeg::locateFfmpeg(ffmpegPath)) {
            std::cerr << "未找到 ffmpeg，可将其放在 external/ffmpeg/bin/ 目录" << std::endl;
            return 1;
        }

        std::string errorMessage;
        if (!Util::Ffmpeg::convertImage(ffmpegPath, ppmPath.string(), output.string(), errorMessage)) {
            std::cerr << errorMessage << std::endl;
            return 1;
        }

        std::error_code removeEc;
        fs::remove(ppmPath, removeEc);
        std::cout << "PNG 已输出: " << output << std::endl;
        return 0;
    }
    case OutputMode::Video: {
        fs::path output = options.outputPath.empty() ? fs::path("output.mp4") : fs::path(options.outputPath);
        if (output.extension().empty()) {
            output.replace_extension(".mp4");
        }

        const int fps = std::max(1, options.fps);
        const float duration = std::max(0.0f, options.durationSeconds);
        const int frameCount = std::max(1, static_cast<int>(std::round(duration * fps)));

        fs::path framesDir = output.parent_path().empty()
            ? fs::path(output.stem().string() + "_frames")
            : output.parent_path() / (output.stem().string() + "_frames");
        std::error_code ec;
        fs::create_directories(framesDir, ec);
        if (ec) {
            std::cerr << "无法创建帧目录: " << framesDir << " 错误: " << ec.message() << std::endl;
            return 1;
        }

        for (int frame = 0; frame < frameCount; ++frame) {
            float time = static_cast<float>(frame) / static_cast<float>(fps);
            animateScene(time);
            renderer.render(scene);

            std::ostringstream fileName;
            fileName << "frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
            fs::path framePath = framesDir / fileName.str();
            if (!renderer.getRenderTarget().savePPM(framePath.string())) {
                std::cerr << "写入帧失败: " << framePath << std::endl;
                return 1;
            }
        }

        std::string ffmpegPath;
        if (!Util::Ffmpeg::locateFfmpeg(ffmpegPath)) {
            std::cerr << "未找到 ffmpeg，可将其放在 external/ffmpeg/bin/ 目录" << std::endl;
            return 1;
        }

        fs::path patternPath = framesDir / "frame_%04d.ppm";
        std::string errorMessage;
        if (!Util::Ffmpeg::encodeVideo(ffmpegPath, patternPath.string(), output.string(), fps, errorMessage)) {
            std::cerr << errorMessage << std::endl;
            return 1;
        }

        fs::remove_all(framesDir, ec);
        if (ec) {
            std::cerr << "警告: 无法删除临时帧目录 " << framesDir << " : " << ec.message() << std::endl;
        }

        std::cout << "视频已输出: " << output << std::endl;
        return 0;
    }
    }

    return 0;
}
