#include "scene.h"

namespace Scene {

Scene::Scene()
    : m_camera(nullptr),
      m_backgroundColor(0.1f, 0.1f, 0.1f, 1.0f),
      m_ambientLight(0.1f, 0.1f, 0.1f, 1.0f) {}

int Scene::addObject(Mesh* mesh, const Matrix4& transform, Material* materialOverride) {
    SceneObject object;
    object.mesh = mesh;
    object.transform = transform;
    object.materialOverride = materialOverride;
    m_objects.push_back(object);
    return static_cast<int>(m_objects.size() - 1);
}

void Scene::addLight(Renderer::Lighting::Light* light) {
    if (light) {
        m_lights.push_back(light);
    }
}

void Scene::removeObject(int index) {
    if (index >= 0 && index < static_cast<int>(m_objects.size())) {
        m_objects.erase(m_objects.begin() + index);
    }
}

void Scene::clear() {
    m_objects.clear();
    m_lights.clear();
    m_camera = nullptr;
}

} // namespace Scene
