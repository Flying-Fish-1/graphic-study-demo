#include "texture.h"
#include <algorithm>
#include <cmath>

namespace Core {
namespace Types {

Texture::Texture(int width, int height, bool shouldBuildMipmaps) {
    allocateLevels(width, height);
    if (shouldBuildMipmaps) {
        buildMipmaps();
    }
}

Texture::Texture(const std::vector<uint32_t>& pixels, int width, int height, bool shouldBuildMipmaps) {
    allocateLevels(width, height);
    if (!pixels.empty()) {
        MipLevel& base = m_levels[0];
        std::copy(pixels.begin(), pixels.begin() + std::min<size_t>(pixels.size(), base.pixels.size()), base.pixels.begin());
    }
    if (shouldBuildMipmaps) {
        buildMipmaps();
    }
}

Color Texture::sample(float u, float v) const {
    return sampleLevel(u, v, 0);
}

Color Texture::sample(float u, float v,
                      float dudx, float dudy,
                      float dvdx, float dvdy) const {
    int level = pickMipLevel(dudx, dudy, dvdx, dvdy);
    return sampleLevel(u, v, level);
}

Color Texture::sampleLevel(float u, float v, int level) const {
    if (m_levels.empty()) {
        return Color::BLACK;
    }
    level = std::clamp(level, 0, static_cast<int>(m_levels.size()) - 1);
    return sampleBilinear(m_levels[level], u, v);
}

void Texture::setPixel(int x, int y, const Color& color, int level) {
    if (m_levels.empty()) return;
    level = std::clamp(level, 0, static_cast<int>(m_levels.size()) - 1);
    MipLevel& target = m_levels[level];
    if (x >= 0 && x < target.width && y >= 0 && y < target.height) {
        target.pixels[y * target.width + x] = color.toUint32();
        if (level == 0 && m_levels.size() > 1) {
            buildMipmaps();
        }
    }
}

Color Texture::getPixel(int x, int y, int level) const {
    if (m_levels.empty()) return Color::BLACK;
    level = std::clamp(level, 0, static_cast<int>(m_levels.size()) - 1);
    return readPixel(m_levels[level], x, y);
}

void Texture::clear(const Color& color) {
    if (m_levels.empty()) return;
    uint32_t packed = color.toUint32();
    for (auto& level : m_levels) {
        std::fill(level.pixels.begin(), level.pixels.end(), packed);
        if (&level != &m_levels[0]) break; // fill base, rebuild below
    }
    if (m_levels.size() > 1) {
        buildMipmaps();
    }
}

void Texture::generateCheckerboard(const Color& color1, const Color& color2, int squareSize) {
    if (m_levels.empty()) return;
    MipLevel& base = m_levels[0];
    for (int y = 0; y < base.height; ++y) {
        for (int x = 0; x < base.width; ++x) {
            bool useFirst = ((x / squareSize) + (y / squareSize)) % 2 == 0;
            base.pixels[y * base.width + x] = (useFirst ? color1 : color2).toUint32();
        }
    }
    if (m_levels.size() > 1) {
        buildMipmaps();
    }
}

void Texture::generateGradient(const Color& topColor, const Color& bottomColor) {
    if (m_levels.empty()) return;
    MipLevel& base = m_levels[0];
    for (int y = 0; y < base.height; ++y) {
        float t = base.height > 1 ? static_cast<float>(y) / static_cast<float>(base.height - 1) : 0.0f;
        Color rowColor = topColor * (1.0f - t) + bottomColor * t;
        uint32_t packed = rowColor.toUint32();
        for (int x = 0; x < base.width; ++x) {
            base.pixels[y * base.width + x] = packed;
        }
    }
    if (m_levels.size() > 1) {
        buildMipmaps();
    }
}

int Texture::getWidth(int level) const {
    if (m_levels.empty()) return 0;
    level = std::clamp(level, 0, static_cast<int>(m_levels.size()) - 1);
    return m_levels[level].width;
}

int Texture::getHeight(int level) const {
    if (m_levels.empty()) return 0;
    level = std::clamp(level, 0, static_cast<int>(m_levels.size()) - 1);
    return m_levels[level].height;
}

const uint32_t* Texture::getPixels(int level) const {
    if (m_levels.empty()) return nullptr;
    level = std::clamp(level, 0, static_cast<int>(m_levels.size()) - 1);
    return m_levels[level].pixels.data();
}

Texture* Texture::loadFromFile(const std::string& /*filename*/) {
    // Placeholder loader: create a checkerboard texture for now.
    Texture* texture = new Texture(128, 128, false);
    texture->generateCheckerboard(Color::WHITE, Color(0.5f, 0.5f, 0.5f, 1.0f), 8);
    texture->buildMipmaps();
    return texture;
}

Texture* Texture::createSolidColor(const Color& color, int width, int height) {
    Texture* texture = new Texture(width, height, false);
    texture->clear(color);
    texture->buildMipmaps();
    return texture;
}

void Texture::allocateLevels(int width, int height) {
    m_levels.clear();
    width = std::max(1, width);
    height = std::max(1, height);

    int levelWidth = width;
    int levelHeight = height;
    while (true) {
        MipLevel level;
        level.width = levelWidth;
        level.height = levelHeight;
        level.pixels.resize(static_cast<size_t>(levelWidth * levelHeight), Color::BLACK.toUint32());
        m_levels.push_back(std::move(level));

        if (levelWidth == 1 && levelHeight == 1) break;
        levelWidth = std::max(1, levelWidth / 2);
        levelHeight = std::max(1, levelHeight / 2);
    }
}

void Texture::buildMipmaps() {
    if (m_levels.size() <= 1) return;
    for (std::size_t i = 1; i < m_levels.size(); ++i) {
        MipLevel& prev = m_levels[i - 1];
        MipLevel& current = m_levels[i];
        for (int y = 0; y < current.height; ++y) {
            for (int x = 0; x < current.width; ++x) {
                int srcX = x * 2;
                int srcY = y * 2;
                Color c00 = readPixel(prev, srcX, srcY);
                Color c10 = readPixel(prev, srcX + 1, srcY);
                Color c01 = readPixel(prev, srcX, srcY + 1);
                Color c11 = readPixel(prev, srcX + 1, srcY + 1);
                Color avg = (c00 + c10 + c01 + c11) * 0.25f;
                current.pixels[y * current.width + x] = avg.toUint32();
            }
        }
    }
}

int Texture::pickMipLevel(float dudx, float dudy, float dvdx, float dvdy) const {
    if (m_levels.size() <= 1) return 0;
    float dudxAbs = std::fabs(dudx);
    float dudyAbs = std::fabs(dudy);
    float dvdxAbs = std::fabs(dvdx);
    float dvdyAbs = std::fabs(dvdy);

    float rho = std::max({dudxAbs, dudyAbs, dvdxAbs, dvdyAbs});
    if (rho < 1e-8f) return 0;

    float baseWidth = static_cast<float>(m_levels[0].width);
    float baseHeight = static_cast<float>(m_levels[0].height);
    float lambda = std::log2(std::max(rho * std::max(baseWidth, baseHeight), 1.0f));
    int level = static_cast<int>(std::floor(lambda));
    return std::clamp(level, 0, static_cast<int>(m_levels.size()) - 1);
}

Color Texture::sampleBilinear(const MipLevel& level, float u, float v) const {
    if (level.width == 0 || level.height == 0) {
        return Color::BLACK;
    }

    u = u - std::floor(u);
    v = v - std::floor(v);

    float x = u * (level.width - 1);
    float y = v * (level.height - 1);

    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    int x1 = std::min(x0 + 1, level.width - 1);
    int y1 = std::min(y0 + 1, level.height - 1);

    float fx = x - static_cast<float>(x0);
    float fy = y - static_cast<float>(y0);

    Color c00 = readPixel(level, x0, y0);
    Color c10 = readPixel(level, x1, y0);
    Color c01 = readPixel(level, x0, y1);
    Color c11 = readPixel(level, x1, y1);

    Color c0 = c00 * (1.0f - fx) + c10 * fx;
    Color c1 = c01 * (1.0f - fx) + c11 * fx;
    return c0 * (1.0f - fy) + c1 * fy;
}

Color Texture::readPixel(const MipLevel& level, int x, int y) {
    x = std::clamp(x, 0, level.width - 1);
    y = std::clamp(y, 0, level.height - 1);
    return Color::fromUint32(level.pixels[y * level.width + x]);
}

} // namespace Types
} // namespace Core
