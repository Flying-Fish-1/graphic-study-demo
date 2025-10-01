#ifndef CORE_TYPES_TEXTURE_H
#define CORE_TYPES_TEXTURE_H

#include "color.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Core {
namespace Types {

/**
 * @brief Simple mipmapped texture container with bilinear sampling support.
 */
class Texture {
public:
    struct MipLevel {
        int width;
        int height;
        std::vector<uint32_t> pixels;
    };

private:
    std::vector<MipLevel> m_levels;   // mip pyramid (level 0 = base)

public:
    Texture(int width, int height, bool buildMipmaps = true);
    Texture(const std::vector<uint32_t>& pixels, int width, int height, bool buildMipmaps = true);

    // Copy disabled, moves allowed
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) = default;
    Texture& operator=(Texture&&) = default;

    ~Texture() = default;

    Color sample(float u, float v) const;
    Color sample(float u, float v,
                 float dudx, float dudy,
                 float dvdx, float dvdy) const;
    Color sampleLevel(float u, float v, int level) const;

    void setPixel(int x, int y, const Color& color, int level = 0);
    Color getPixel(int x, int y, int level = 0) const;

    void clear(const Color& color);
    void generateCheckerboard(const Color& color1, const Color& color2, int squareSize = 8);
    void generateGradient(const Color& topColor, const Color& bottomColor);

    int getWidth(int level = 0) const;
    int getHeight(int level = 0) const;

    const uint32_t* getPixels(int level = 0) const;

    static Texture* loadFromFile(const std::string& filename);
    static Texture* createSolidColor(const Color& color, int width = 64, int height = 64);

private:
    void allocateLevels(int width, int height);
    void buildMipmaps();
    int pickMipLevel(float dudx, float dudy, float dvdx, float dvdy) const;
    Color sampleBilinear(const MipLevel& level, float u, float v) const;
    static Color readPixel(const MipLevel& level, int x, int y);
};

} // namespace Types
} // namespace Core

#endif // CORE_TYPES_TEXTURE_H
