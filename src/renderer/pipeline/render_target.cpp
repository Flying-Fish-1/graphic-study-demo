#include "render_target.h"
#include <algorithm>
#include <fstream>

namespace Renderer {
namespace Pipeline {

using Core::Types::Color;

RenderTarget::RenderTarget(int width, int height) : m_width(width), m_height(height) {
    resize(width, height);
}

void RenderTarget::resize(int width, int height) {
    m_width = std::max(0, width);
    m_height = std::max(0, height);
    m_colorBuffer.resize(static_cast<std::size_t>(m_width * m_height), Color::BLACK);
    m_depthBuffer.resize(static_cast<std::size_t>(m_width * m_height), 1.0f);
}

void RenderTarget::clearColor(const Color& color) {
    std::fill(m_colorBuffer.begin(), m_colorBuffer.end(), color);
}

void RenderTarget::clearDepth(float depth) {
    std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), depth);
}

void RenderTarget::clear(const Color& color, float depth) {
    clearColor(color);
    clearDepth(depth);
}

bool RenderTarget::depthTestAndSet(int x, int y, float depth) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return false;
    }
    std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x);
    if (depth < m_depthBuffer[index]) {
        m_depthBuffer[index] = depth;
        return true;
    }
    return false;
}

bool RenderTarget::depthPasses(int x, int y, float depth) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return false;
    }
    std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x);
    return depth < m_depthBuffer[index];
}

void RenderTarget::setDepth(int x, int y, float depth) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return;
    }
    std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x);
    m_depthBuffer[index] = depth;
}

float RenderTarget::getDepth(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return 1.0f;
    }
    std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x);
    return m_depthBuffer[index];
}

void RenderTarget::setPixel(int x, int y, const Color& color) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return;
    }
    std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x);
    m_colorBuffer[index] = color;
}

Color RenderTarget::getPixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return Color::BLACK;
    }
    std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x);
    return m_colorBuffer[index];
}

bool RenderTarget::savePPM(const std::string& filename) const {
    if (m_width == 0 || m_height == 0) {
        return false;
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file << "P6\n" << m_width << " " << m_height << "\n255\n";
    for (const auto& color : m_colorBuffer) {
        unsigned char r = static_cast<unsigned char>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
        unsigned char g = static_cast<unsigned char>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
        unsigned char b = static_cast<unsigned char>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
        file.write(reinterpret_cast<const char*>(&r), 1);
        file.write(reinterpret_cast<const char*>(&g), 1);
        file.write(reinterpret_cast<const char*>(&b), 1);
    }
    return true;
}

} // namespace Pipeline
} // namespace Renderer
