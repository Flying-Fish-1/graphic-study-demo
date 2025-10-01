#ifndef RENDERER_PIPELINE_SOFTWARE_RENDERER_H
#define RENDERER_PIPELINE_SOFTWARE_RENDERER_H

#include "render_target.h"
#include "../../scene/scene.h"
#include "../../scene/camera.h"
#include <vector>

namespace Renderer {
namespace Pipeline {

struct SoftwareRendererSettings {
    int width = 800;
    int height = 600;
    bool perspectiveCorrect = true;
    bool backfaceCulling = true;
    int ssaaFactor = 1; // 1表示关闭；2=2xSSAA，3=3xSSAA，4=4xSSAA
    bool fresnelForTranslucent = false; // 对半透明启用菲涅尔
    float fresnelF0 = 0.04f; // 介质默认F0
};

class SoftwareRenderer {
private:
    SoftwareRendererSettings m_settings;
    RenderTarget m_target;

public:
    explicit SoftwareRenderer(const SoftwareRendererSettings& settings = SoftwareRendererSettings());

    void setSettings(const SoftwareRendererSettings& settings);
    const SoftwareRendererSettings& getSettings() const { return m_settings; }

    RenderTarget& getRenderTarget() { return m_target; }
    const RenderTarget& getRenderTarget() const { return m_target; }

    void render(const Scene::Scene& scene);
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_SOFTWARE_RENDERER_H
