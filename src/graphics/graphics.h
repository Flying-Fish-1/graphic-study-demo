#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../core/types/color.h"
#include "../core/platform/sdl_wrapper.h"

using namespace Core::Types;
using namespace Core::Platform;

/**
 * @brief 图形绘制类
 * 
 * 提供各种图形绘制功能，包括直线、圆形和三角形光栅化
 */
class Graphics {
private:
    SDLWrapper* m_sdlWrapper;

public:
    /**
     * @brief 构造函数
     * @param sdlWrapper SDL包装器指针
     */
    Graphics(SDLWrapper* sdlWrapper);
    
    /**
     * @brief 使用Bresenham算法绘制直线
     * @param x0 起点x坐标
     * @param y0 起点y坐标
     * @param x1 终点x坐标
     * @param y1 终点y坐标
     * @param color0 起点颜色
     * @param color1 终点颜色
     */
    void drawLine(int x0, int y0, int x1, int y1, const Color& color0, const Color& color1);
    
    /**
     * @brief 使用Bresenham算法绘制圆形
     * @param xc 圆心x坐标
     * @param yc 圆心y坐标
     * @param radius 半径
     * @param color 颜色
     */
    void drawCircle(int xc, int yc, int radius, const Color& color);
    
    /**
     * @brief 三角形光栅化
     * @param x0 顶点1 x坐标
     * @param y0 顶点1 y坐标
     * @param x1 顶点2 x坐标
     * @param y1 顶点2 y坐标
     * @param x2 顶点3 x坐标
     * @param y2 顶点3 y坐标
     * @param c0 顶点1颜色
     * @param c1 顶点2颜色
     * @param c2 顶点3颜色
     */
        void rasterizeTriangle(int x0, int y0, int x1, int y1, int x2, int y2,
                          const Color& c0, const Color& c1, const Color& c2);
    
    /**
     * @brief 检查点是否在三角形内
     * @param x0 顶点1 x坐标
     * @param y0 顶点1 y坐标
     * @param x1 顶点2 x坐标
     * @param y1 顶点2 y坐标
     * @param x2 顶点3 x坐标
     * @param y2 顶点3 y坐标
     * @param px 测试点x坐标
     * @param py 测试点y坐标
     * @return 点在三角形内返回true，否则返回false
     */
    static bool checkPointInTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int px, int py);
    
    /**
     * @brief 获取三角形内某点的插值颜色
     * @param x0 顶点1 x坐标
     * @param y0 顶点1 y坐标
     * @param x1 顶点2 x坐标
     * @param y1 顶点2 y坐标
     * @param x2 顶点3 x坐标
     * @param y2 顶点3 y坐标
     * @param px 测试点x坐标
     * @param py 测试点y坐标
     * @param c0 顶点1颜色
     * @param c1 顶点2颜色
     * @param c2 顶点3颜色
     * @return 插值颜色
     */
    static Color getPointColorInTriangle(int x0, int y0, int x1, int y1, int x2, int y2, 
                                           int px, int py, const Color& c0, const Color& c1, const Color& c2);

private:
    /**
     * @brief 利用对称性绘制圆形的8个点
     * @param xc 圆心x坐标
     * @param yc 圆心y坐标
     * @param x 相对坐标x
     * @param y 相对坐标y
     * @param color 颜色
     */
    void plotCirclePoints(int xc, int yc, int x, int y, const Color& color);
};

#endif // GRAPHICS_H
