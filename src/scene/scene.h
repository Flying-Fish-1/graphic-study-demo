#ifndef SCENE_SCENE_H
#define SCENE_SCENE_H

#include "mesh.h"
#include "camera.h"
#include "../core/types/color.h"
#include <vector>

namespace Renderer {
namespace Lighting {
class Light;
}
}

namespace Scene {

struct SceneObject {
    Mesh* mesh = nullptr;
    Matrix4 transform = Matrix4::identity();
    Material* materialOverride = nullptr;
    bool visible = true;
};

class Scene {
private:
    std::vector<SceneObject> m_objects;
    std::vector<Renderer::Lighting::Light*> m_lights;
    Camera* m_camera;
    Core::Types::Color m_backgroundColor;
    Core::Types::Color m_ambientLight;

public:
    Scene();

    int addObject(Mesh* mesh, const Matrix4& transform = Matrix4::identity(), Material* materialOverride = nullptr);
    void addLight(Renderer::Lighting::Light* light);

    void removeObject(int index);
    void clear();

    void setCamera(Camera* camera) { m_camera = camera; }
    Camera* getCamera() const { return m_camera; }

    const std::vector<SceneObject>& getObjects() const { return m_objects; }
    const std::vector<Renderer::Lighting::Light*>& getLights() const { return m_lights; }

    const Core::Types::Color& getBackgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const Core::Types::Color& color) { m_backgroundColor = color; }

    const Core::Types::Color& getAmbientLight() const { return m_ambientLight; }
    void setAmbientLight(const Core::Types::Color& color) { m_ambientLight = color; }
};

} // namespace Scene

#endif // SCENE_SCENE_H
