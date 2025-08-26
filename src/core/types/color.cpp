#include "color.h"

namespace Core {
namespace Types {

Color Color::fromUint32(uint32_t color) {
    return Color(
        ((color >> 24) & 0xFF) / 255.0f,
        ((color >> 16) & 0xFF) / 255.0f,
        ((color >> 8) & 0xFF) / 255.0f,
        (color & 0xFF) / 255.0f
    );
}

uint32_t Color::toUint32() const {
    return ((uint8_t)(r * 255.0f) << 24) |
           ((uint8_t)(g * 255.0f) << 16) |
           ((uint8_t)(b * 255.0f) << 8) |
           ((uint8_t)(a * 255.0f));
}

// 预定义颜色
const Color Color::RED(1.0f, 0.0f, 0.0f, 1.0f);
const Color Color::GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const Color Color::BLUE(0.0f, 0.0f, 1.0f, 1.0f);
const Color Color::WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const Color Color::CYAN(0.0f, 1.0f, 1.0f, 1.0f);
const Color Color::MAGENTA(1.0f, 0.0f, 1.0f, 1.0f);
const Color Color::YELLOW(1.0f, 1.0f, 0.0f, 1.0f);

} // namespace Types
} // namespace Core
