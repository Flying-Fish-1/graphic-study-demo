#ifndef RENDERER_PIPELINE_SOFTWARE_RENDERER_H
#define RENDERER_PIPELINE_SOFTWARE_RENDERER_H

#include "render_target.h"
#include "geometry_stage.h"
#include "../../scene/scene.h"
#include "../../scene/camera.h"
#include "../../core/types/material.h"
#include "../../renderer/lighting/light.h"
#include <vector>

namespace Renderer {
namespace Pipeline {

using Core::Types::Material;

struct SoftwareRendererSettings {
    int width = 800;
    int height = 600;
    bool perspectiveCorrect = true;
    bool backfaceCulling = true;
    int ssaaFactor = 1; // 1表示关闭；2=2xSSAA，3=3xSSAA，4=4xSSAA
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

private:
    struct ScreenVertex {
        GeometryVertex attributes;
        float screenX;
        float screenY;
        float ndcZ;
        bool valid = false;
    };

    struct RasterDerivatives {
        float dudx;
        float dudy;
        float dvdx;
        float dvdy;
    };

    std::vector<ScreenVertex> runGeometryStage(const Scene::SceneObject& object,
                                               const Core::Math::Matrix4& viewMatrix,
                                               const Core::Math::Matrix4& projectionMatrix) const;

    bool runPrimitiveAssembly(const ScreenVertex& v0,
                              const ScreenVertex& v1,
                              const ScreenVertex& v2,
                              const Core::Math::Vector3& cameraPos,
                              Core::Math::Vector3& faceNormal,
                              bool applyCulling = true) const;

    RasterDerivatives computeRasterDerivatives(const ScreenVertex& v0,
                                               const ScreenVertex& v1,
                                               const ScreenVertex& v2) const;

    void runRasterStage(const ScreenVertex& v0,
                        const ScreenVertex& v1,
                        const ScreenVertex& v2,
                        Material* material,
                        const std::vector<Renderer::Lighting::Light*>& lights,
                        const Core::Math::Vector3& cameraPos,
                        const Core::Types::Color& ambientLight,
                        const RasterDerivatives& derivs);

    Core::Types::Color runShadingStage(const GeometryVertex& interpolated,
                                       Material* material,
                                       const std::vector<Renderer::Lighting::Light*>& lights,
                                       const Core::Math::Vector3& viewPos,
                                       const Core::Types::Color& sceneAmbient,
                                       const RasterDerivatives& derivs) const;
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_SOFTWARE_RENDERER_H
