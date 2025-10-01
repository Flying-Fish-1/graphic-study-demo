#include "render_queue.h"

#include <algorithm>

namespace Renderer {
namespace Pipeline {

void RenderQueue::clear() {
    m_opaque.clear();
    m_transparent.clear();
}

void RenderQueue::addOpaque(const TriangleWorkItem& tri) {
    m_opaque.push_back(tri);
}

void RenderQueue::addTransparent(const TriangleWorkItem& tri) {
    m_transparent.push_back(tri);
}

void RenderQueue::finalize() {
    std::sort(m_opaque.begin(), m_opaque.end(), [](const TriangleWorkItem& a, const TriangleWorkItem& b) {
        return a.depthKey < b.depthKey;
    });

    std::sort(m_transparent.begin(), m_transparent.end(), [](const TriangleWorkItem& a, const TriangleWorkItem& b) {
        return a.depthKey > b.depthKey;
    });
}

} // namespace Pipeline
} // namespace Renderer
