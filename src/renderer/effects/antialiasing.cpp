#include "antialiasing.h"
#include "../../graphics/graphics.h"
#include <iostream>
#include <algorithm>
#include <cstring>

namespace Renderer {
namespace Effects {

AntiAliasing::AntiAliasing(AntiAliasingMethod method, int ssaaScale) 
    : m_currentMethod(method), m_ssaaScale(ssaaScale) {
}

void AntiAliasing::applyAntiAliasing(Uint32* buffer, int width, int height) {
    // 根据当前选择的抗锯齿方法应用相应的算法
    switch (m_currentMethod) {
        case GAUSSIAN_BLUR:
            applyGaussianBlur(buffer, width, height);
            break;
        case SSAA:
            // SSAA在光栅化阶段已经应用，这里不需要额外处理
            break;
        case SSAA_GAUSSIAN:
            // 先应用高斯模糊
            applyGaussianBlur(buffer, width, height);
            break;
        case NONE:
        default:
            // 不应用任何抗锯齿
            break;
    }
}

void AntiAliasing::applyGaussianBlur(Uint32* buffer, int width, int height) {
    // 可分离的1D高斯模糊：O(n²) 复杂度而非 O(n²×k²)
    applySeparableGaussianBlur(buffer, width, height);
}

void AntiAliasing::applySeparableGaussianBlur(Uint32* buffer, int width, int height) {
    // 1D高斯核 [1, 2, 1] / 4 (对应3x3高斯核的分解)
    const float kernel1D[3] = {0.25f, 0.5f, 0.25f};
    const int radius = 1;
    
    // 临时缓冲区：水平模糊结果
    Uint32* tempBuffer = new Uint32[width * height];
    
    // 第一步：水平方向1D高斯模糊
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0, a = 0;
            
            // 水平方向卷积
            for (int kx = -radius; kx <= radius; kx++) {
                int px = std::max(0, std::min(width - 1, x + kx)); // 边界处理
                
                Uint32 pixel = buffer[y * width + px];
                Color color = Color::fromUint32(pixel);
                
                float weight = kernel1D[kx + radius];
                r += color.r * weight;
                g += color.g * weight;
                b += color.b * weight;
                a += color.a * weight;
            }
            
            Color newColor(r, g, b, a);
            tempBuffer[y * width + x] = newColor.toUint32();
        }
    }
    
    // 第二步：垂直方向1D高斯模糊
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0, a = 0;
            
            // 垂直方向卷积
            for (int ky = -radius; ky <= radius; ky++) {
                int py = std::max(0, std::min(height - 1, y + ky)); // 边界处理
                
                Uint32 pixel = tempBuffer[py * width + x];
                Color color = Color::fromUint32(pixel);
                
                float weight = kernel1D[ky + radius];
                r += color.r * weight;
                g += color.g * weight;
                b += color.b * weight;
                a += color.a * weight;
            }
            
            Color newColor(r, g, b, a);
            buffer[y * width + x] = newColor.toUint32();
        }
    }
    
    // 释放临时缓冲区
    delete[] tempBuffer;
}

void AntiAliasing::applySuperSampling(int x0, int y0, int x1, int y1, int x2, int y2, 
                                     const Color& c0, const Color& c1, const Color& c2, 
                                     Uint32* buffer, int width, int height) {
    // 创建高分辨率缓冲区
    int ssaaWidth = width * m_ssaaScale;
    int ssaaHeight = height * m_ssaaScale;
    Uint32* ssaaBuffer = new Uint32[ssaaWidth * ssaaHeight];
    
    // 清空高分辨率缓冲区
    std::memset(ssaaBuffer, 0, ssaaWidth * ssaaHeight * sizeof(Uint32));
    
    // 在高分辨率下进行光栅化
    // 计算包围盒
    int minX = std::min(std::min(x0, x1), x2) * m_ssaaScale;
    int minY = std::min(std::min(y0, y1), y2) * m_ssaaScale;
    int maxX = std::max(std::max(x0, x1), x2) * m_ssaaScale;
    int maxY = std::max(std::max(y0, y1), y2) * m_ssaaScale;
    
    // 限制在高分辨率屏幕范围内
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, ssaaWidth - 1);
    maxY = std::min(maxY, ssaaHeight - 1);
    
    // 高分辨率下的顶点坐标
    int ssaaX0 = x0 * m_ssaaScale;
    int ssaaY0 = y0 * m_ssaaScale;
    int ssaaX1 = x1 * m_ssaaScale;
    int ssaaY1 = y1 * m_ssaaScale;
    int ssaaX2 = x2 * m_ssaaScale;
    int ssaaY2 = y2 * m_ssaaScale;
    
    // 在高分辨率下进行光栅化
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float sx = x + 0.5f;
            float sy = y + 0.5f;
            
            if (Graphics::checkPointInTriangle(ssaaX0, ssaaY0, ssaaX1, ssaaY1, ssaaX2, ssaaY2, sx, sy)) {
                Color color = Graphics::getPointColorInTriangle(ssaaX0, ssaaY0, ssaaX1, ssaaY1, ssaaX2, ssaaY2, 
                                                                  sx, sy, c0, c1, c2);
                // 设置高分辨率缓冲区的像素
                int index = y * ssaaWidth + x;
                ssaaBuffer[index] = color.toUint32();
            }
        }
    }
    
    // 将高分辨率缓冲区下采样到原始分辨率
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0, a = 0;
            int count = 0;
            
            // 对每个原始像素，采样m_ssaaScale*m_ssaaScale个高分辨率像素
            for (int dy = 0; dy < m_ssaaScale; dy++) {
                for (int dx = 0; dx < m_ssaaScale; dx++) {
                    int ssaaX = x * m_ssaaScale + dx;
                    int ssaaY = y * m_ssaaScale + dy;
                    
                    if (ssaaX < ssaaWidth && ssaaY < ssaaHeight) {
                        int ssaaIndex = ssaaY * ssaaWidth + ssaaX;
                        Color color = Color::fromUint32(ssaaBuffer[ssaaIndex]);
                        
                        r += color.r;
                        g += color.g;
                        b += color.b;
                        a += color.a;
                        count++;
                    }
                }
            }
            
            // 计算平均值
            if (count > 0) {
                r /= count;
                g /= count;
                b /= count;
                a /= count;
                
                // 设置原始分辨率缓冲区的像素
                int index = y * width + x;
                buffer[index] = Color(r, g, b, a).toUint32();
            }
        }
    }
    
    // 释放高分辨率缓冲区
    delete[] ssaaBuffer;
}

void AntiAliasing::switchAntiAliasingMethod() {
    // 循环切换到下一个抗锯齿方案
    m_currentMethod = static_cast<AntiAliasingMethod>((m_currentMethod + 1) % 4);
    
    // 输出当前使用的抗锯齿方案
    const char* methodNames[] = {"无抗锯齿", "高斯模糊抗锯齿", "超采样抗锯齿", "超采样+高斯模糊组合"};
    std::cout << "当前抗锯齿方案: " << methodNames[m_currentMethod] << std::endl;
}

const char* AntiAliasing::getAntiAliasingMethodName() const {
    const char* methodNames[] = {"无抗锯齿", "高斯模糊抗锯齿", "超采样抗锯齿", "超采样+高斯模糊组合"};
    return methodNames[m_currentMethod];
}

void AntiAliasing::increaseSsaaScale() {
    if (m_ssaaScale < MAX_SSAA_SCALE) {
        m_ssaaScale++;
        std::cout << "超采样倍数增加到: " << m_ssaaScale << "x" << std::endl;
    } else {
        std::cout << "已达到最大超采样倍数: " << m_ssaaScale << "x" << std::endl;
    }
}

void AntiAliasing::decreaseSsaaScale() {
    if (m_ssaaScale > MIN_SSAA_SCALE) {
        m_ssaaScale--;
        std::cout << "超采样倍数减少到: " << m_ssaaScale << "x" << std::endl;
    } else {
        std::cout << "已达到最小超采样倍数: " << m_ssaaScale << "x" << std::endl;
    }
}

std::string AntiAliasing::getSsaaScaleInfo() const {
    return std::to_string(m_ssaaScale) + "x超采样";
}

} // namespace Effects
} // namespace Renderer
