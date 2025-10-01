#ifndef RENDERER_PIPELINE_RENDER_QUEUE_H
#define RENDERER_PIPELINE_RENDER_QUEUE_H

#include <vector>

#include "screen_vertex.h"

namespace Core {
namespace Types {
class Material;
}
}

namespace Renderer {
namespace Pipeline {

struct TriangleWorkItem {
    ScreenVertex v0;
    ScreenVertex v1;
    ScreenVertex v2;
    Core::Types::Material* material = nullptr;
    RasterDerivatives derivs{};
    float depthKey = 0.0f;
};

class RenderQueue {
public:
    void clear();

    void addOpaque(const TriangleWorkItem& tri);
    void addTransparent(const TriangleWorkItem& tri);

    void finalize();

    const std::vector<TriangleWorkItem>& getOpaque() const { return m_opaque; }
    const std::vector<TriangleWorkItem>& getTransparent() const { return m_transparent; }

private:
    std::vector<TriangleWorkItem> m_opaque;
    std::vector<TriangleWorkItem> m_transparent;
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_RENDER_QUEUE_H
