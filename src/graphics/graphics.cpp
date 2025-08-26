#include "graphics.h"
#include "../core/platform/constants.h"
#include <algorithm>
#include <cmath>
#include <iostream>

Graphics::Graphics(SDLWrapper* sdlWrapper) : m_sdlWrapper(sdlWrapper) {
}

void Graphics::drawLine(int x0, int y0, int x1, int y1, const Color& color0, const Color& color1) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    int x = x0;
    int y = y0;
    
    while (true) {
        float t = (float)(x - x0) / (float)(x1 - x0);
        Color color = color0 * (1.0f - t) + color1 * t;
        m_sdlWrapper->setPixel(x, y, color);
        
        if (x == x1 && y == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void Graphics::drawCircle(int xc, int yc, int r, const Color& color) {
    int x = 0;
    int y = r;
    int p = 1 - r;  // initial decision parameter
    
    // initial plot
    plotCirclePoints(xc, yc, x, y, color);
    
    while (x < y) {
        x++;
        
        // update decision parameter
        if (p < 0) {
            p += 2 * x + 1;
        } 
        else {
            y--;
            p += 2 * (x - y) + 1;
        }
        
        // plot eight points
        plotCirclePoints(xc, yc, x, y, color);
    }
}

void Graphics::plotCirclePoints(int xc, int yc, int x, int y, const Color& color) {
    m_sdlWrapper->setPixel(xc + x, yc + y, color);
    m_sdlWrapper->setPixel(xc - x, yc + y, color);
    m_sdlWrapper->setPixel(xc + x, yc - y, color);
    m_sdlWrapper->setPixel(xc - x, yc - y, color);
    m_sdlWrapper->setPixel(xc + y, yc + x, color);
    m_sdlWrapper->setPixel(xc - y, yc + x, color);
    m_sdlWrapper->setPixel(xc + y, yc - x, color);
    m_sdlWrapper->setPixel(xc - y, yc - x, color);
}

void Graphics::rasterizeTriangle(int x0, int y0, int x1, int y1, int x2, int y2, 
                                const Color& c0, const Color& c1, const Color& c2) {
    // 计算包围盒
    int minX = std::min(std::min(x0, x1), x2);
    int minY = std::min(std::min(y0, y1), y2);
    int maxX = std::max(std::max(x0, x1), x2);
    int maxY = std::max(std::max(y0, y1), y2);
    
    // 限制在屏幕范围内
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, SCREEN_WIDTH - 1);
    maxY = std::min(maxY, SCREEN_HEIGHT - 1);
    
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float sx = x + 0.5f;
            float sy = y + 0.5f;
            
            if (checkPointInTriangle(x0, y0, x1, y1, x2, y2, sx, sy)) {
                Color color = getPointColorInTriangle(x0, y0, x1, y1, x2, y2, sx, sy, c0, c1, c2);
                m_sdlWrapper->setPixel(x, y, color);
            }
        }
    }
}

bool Graphics::checkPointInTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int px, int py) {
    // 计算向量
    int v0x = x2 - x0, v0y = y2 - y0;
    int v1x = x1 - x0, v1y = y1 - y0;
    int v2x = px - x0, v2y = py - y0;
    
    // 计算点积
    float dot00 = v0x * v0x + v0y * v0y;
    float dot01 = v0x * v1x + v0y * v1y;
    float dot02 = v0x * v2x + v0y * v2y;
    float dot11 = v1x * v1x + v1y * v1y;
    float dot12 = v1x * v2x + v1y * v2y;
    
    // 计算重心坐标
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    // 检查是否在三角形内
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

Color Graphics::getPointColorInTriangle(int x0, int y0, int x1, int y1, int x2, int y2, 
                                          int px, int py, const Color& c0, const Color& c1, const Color& c2) {
    // 使用向量方法计算重心坐标
    int v0x = x2 - x0, v0y = y2 - y0;
    int v1x = x1 - x0, v1y = y1 - y0;
    int v2x = px - x0, v2y = py - y0;
    
    float dot00 = v0x * v0x + v0y * v0y;
    float dot01 = v0x * v1x + v0y * v1y;
    float dot02 = v0x * v2x + v0y * v2y;
    float dot11 = v1x * v1x + v1y * v1y;
    float dot12 = v1x * v2x + v1y * v2y;
    
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    float w = 1.0f - u - v;
    
    // 使用重心坐标插值颜色
    return c0 * w + c1 * v + c2 * u;
}
