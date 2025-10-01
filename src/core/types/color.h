#ifndef CORE_TYPES_COLOR_H
#define CORE_TYPES_COLOR_H

#include <cstdint>

namespace Core {
namespace Types {

/**
 * @brief 颜色类 - RGBA格式
 */
struct Color {
    float r, g, b, a;
    
    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    
    // 颜色运算
    Color operator*(float t) const { return Color(r*t, g*t, b*t, a*t); }
    Color operator*(const Color& other) const { 
        return Color(r * other.r, g * other.g, b * other.b, a * other.a); 
    }
    Color operator+(const Color& other) const { 
        return Color(r + other.r, g + other.g, b + other.b, a + other.a); 
    }
    
    // 格式转换
    static Color fromUint32(uint32_t color);
    uint32_t toUint32() const;
    
    // 预定义颜色
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color WHITE;
    static const Color BLACK;
    static const Color CYAN;
    static const Color MAGENTA;
    static const Color YELLOW;
};

} // namespace Types
} // namespace Core

#endif // CORE_TYPES_COLOR_H
