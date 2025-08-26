#ifndef ANTIALIASING_H
#define ANTIALIASING_H

#include <SDL2/SDL.h>
#include <string>
#include "../../core/platform/constants.h"
#include "../../core/types/color.h"

namespace Renderer {
namespace Effects {

using namespace Core::Types;

/**
 * @brief 抗锯齿管理类
 * 
 * 提供各种抗锯齿算法的实现和管理
 */
class AntiAliasing {
private:
    AntiAliasingMethod m_currentMethod;
    int m_ssaaScale;

public:
    /**
     * @brief 构造函数
     * @param method 默认抗锯齿方法
     * @param ssaaScale 默认超采样倍数
     */
    AntiAliasing(AntiAliasingMethod method = SSAA, int ssaaScale = 2);
    
    /**
     * @brief 应用选定的抗锯齿方法
     * @param buffer 像素缓冲区
     * @param width 缓冲区宽度
     * @param height 缓冲区高度
     */
    void applyAntiAliasing(Uint32* buffer, int width, int height);
    
    /**
     * @brief 高斯模糊抗锯齿算法（使用可分离的1D卷积优化）
     * @param buffer 像素缓冲区
     * @param width 缓冲区宽度
     * @param height 缓冲区高度
     */
    void applyGaussianBlur(Uint32* buffer, int width, int height);
    
private:
    /**
     * @brief 可分离的1D高斯模糊实现
     * @param buffer 像素缓冲区
     * @param width 缓冲区宽度
     * @param height 缓冲区高度
     */
    void applySeparableGaussianBlur(Uint32* buffer, int width, int height);
    
public:
    
    /**
     * @brief 超采样抗锯齿算法
     * @param x0 顶点1 x坐标
     * @param y0 顶点1 y坐标
     * @param x1 顶点2 x坐标
     * @param y1 顶点2 y坐标
     * @param x2 顶点3 x坐标
     * @param y2 顶点3 y坐标
     * @param c0 顶点1颜色
     * @param c1 顶点2颜色
     * @param c2 顶点3颜色
     * @param buffer 像素缓冲区
     * @param width 缓冲区宽度
     * @param height 缓冲区高度
     */
    void applySuperSampling(int x0, int y0, int x1, int y1, int x2, int y2, 
                           const Color& c0, const Color& c1, const Color& c2, 
                           Uint32* buffer, int width, int height);
    
    /**
     * @brief 切换抗锯齿方案
     */
    void switchAntiAliasingMethod();
    
    /**
     * @brief 获取当前抗锯齿方案名称
     * @return 抗锯齿方案名称
     */
    const char* getAntiAliasingMethodName() const;
    
    /**
     * @brief 增加超采样倍数
     */
    void increaseSsaaScale();
    
    /**
     * @brief 减少超采样倍数
     */
    void decreaseSsaaScale();
    
    /**
     * @brief 获取超采样倍数信息
     * @return 超采样倍数信息字符串
     */
    std::string getSsaaScaleInfo() const;
    
    /**
     * @brief 获取当前抗锯齿方法
     * @return 当前抗锯齿方法
     */
    AntiAliasingMethod getCurrentMethod() const { return m_currentMethod; }
    
    /**
     * @brief 获取当前超采样倍数
     * @return 超采样倍数
     */
    int getSsaaScale() const { return m_ssaaScale; }
};

} // namespace Effects
} // namespace Renderer

#endif // ANTIALIASING_H
