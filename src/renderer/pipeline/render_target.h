#ifndef RENDERER_PIPELINE_RENDER_TARGET_H
#define RENDERER_PIPELINE_RENDER_TARGET_H

#include "../../core/types/color.h"
#include <string>
#include <vector>

namespace Renderer {
namespace Pipeline {

class RenderTarget {
private:
    int m_width;
    int m_height;
    std::vector<Core::Types::Color> m_colorBuffer;
    std::vector<float> m_depthBuffer;

public:
    RenderTarget(int width = 0, int height = 0);

    void resize(int width, int height);

    void clearColor(const Core::Types::Color& color);
    void clearDepth(float depth);
    void clear(const Core::Types::Color& color, float depth);

    bool depthTestAndSet(int x, int y, float depth);
    bool depthPasses(int x, int y, float depth) const;
    void setDepth(int x, int y, float depth);
    float getDepth(int x, int y) const;

    void setPixel(int x, int y, const Core::Types::Color& color);
    Core::Types::Color getPixel(int x, int y) const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const std::vector<Core::Types::Color>& getColorBuffer() const { return m_colorBuffer; }

    bool savePPM(const std::string& filename) const;
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_RENDER_TARGET_H
