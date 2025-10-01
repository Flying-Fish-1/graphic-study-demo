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
    bool enableFresnel = false; // 启用基于Schlick近似的菲涅尔反射
    float fresnelF0 = 0.04f; // 无材质高光时的默认法线入射反射率
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
