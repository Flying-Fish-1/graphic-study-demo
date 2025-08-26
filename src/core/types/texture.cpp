#include "texture.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace Core {
namespace Types {

Texture::Texture(int width, int height) 
    : m_width(width), m_height(height), m_ownsData(true) {
    m_pixels = new uint32_t[width * height];
    std::memset(m_pixels, 0, width * height * sizeof(uint32_t));
}

Texture::Texture(uint32_t* pixels, int width, int height, bool takeOwnership)
    : m_pixels(pixels), m_width(width), m_height(height), m_ownsData(takeOwnership) {
}

Texture::~Texture() {
    if (m_ownsData && m_pixels) {
        delete[] m_pixels;
    }
}

Color Texture::sample(float u, float v) const {
    // 默认使用双线性插值
    return sampleBilinear(u, v);
}

Color Texture::sampleBilinear(float u, float v) const {
    // 将UV坐标映射到纹理空间
    u = std::fmod(u, 1.0f);
    v = std::fmod(v, 1.0f);
    if (u < 0) u += 1.0f;
    if (v < 0) v += 1.0f;
    
    float x = u * (m_width - 1);
    float y = v * (m_height - 1);
    
    int x0 = static_cast<int>(x);
    int y0 = static_cast<int>(y);
    int x1 = std::min(x0 + 1, m_width - 1);
    int y1 = std::min(y0 + 1, m_height - 1);
    
    float fx = x - x0;
    float fy = y - y0;
    
    // 获取四个邻近像素的颜色
    Color c00 = getPixel(x0, y0);
    Color c10 = getPixel(x1, y0);
    Color c01 = getPixel(x0, y1);
    Color c11 = getPixel(x1, y1);
    
    // 双线性插值
    Color c0 = c00 * (1.0f - fx) + c10 * fx;
    Color c1 = c01 * (1.0f - fx) + c11 * fx;
    
    return c0 * (1.0f - fy) + c1 * fy;
}

Color Texture::sampleNearest(float u, float v) const {
    // 将UV坐标映射到纹理空间
    u = std::fmod(u, 1.0f);
    v = std::fmod(v, 1.0f);
    if (u < 0) u += 1.0f;
    if (v < 0) v += 1.0f;
    
    int x = static_cast<int>(u * m_width) % m_width;
    int y = static_cast<int>(v * m_height) % m_height;
    
    return getPixel(x, y);
}

void Texture::setPixel(int x, int y, const Color& color) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_pixels[y * m_width + x] = color.toUint32();
    }
}

Color Texture::getPixel(int x, int y) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        return Color::fromUint32(m_pixels[y * m_width + x]);
    }
    return Color::BLACK;
}

void Texture::clear(const Color& color) {
    uint32_t colorValue = color.toUint32();
    for (int i = 0; i < m_width * m_height; i++) {
        m_pixels[i] = colorValue;
    }
}

void Texture::generateCheckerboard(const Color& color1, const Color& color2, int squareSize) {
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            bool isColor1 = ((x / squareSize) + (y / squareSize)) % 2 == 0;
            setPixel(x, y, isColor1 ? color1 : color2);
        }
    }
}

void Texture::generateGradient(const Color& topColor, const Color& bottomColor) {
    for (int y = 0; y < m_height; y++) {
        float t = static_cast<float>(y) / (m_height - 1);
        Color rowColor = topColor * (1.0f - t) + bottomColor * t;
        
        for (int x = 0; x < m_width; x++) {
            setPixel(x, y, rowColor);
        }
    }
}

Texture* Texture::loadFromFile(const std::string& filename) {
    // 简单实现：目前不支持文件加载
    // 在实际项目中，这里应该使用图像加载库如stb_image
    
    // 返回一个默认纹理
    Texture* texture = new Texture(64, 64);
    texture->generateCheckerboard(Color::WHITE, Color(0.5f, 0.5f, 0.5f, 1.0f));
    return texture;
}

Texture* Texture::createSolidColor(const Color& color, int width, int height) {
    Texture* texture = new Texture(width, height);
    texture->clear(color);
    return texture;
}

} // namespace Types
} // namespace Core
