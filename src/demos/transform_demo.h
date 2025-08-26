#ifndef TRANSFORM_DEMO_H
#define TRANSFORM_DEMO_H

#include "../core/math/vector.h"
#include "../core/math/matrix.h"
#include "../core/types/color.h"
#include "../graphics/graphics.h"

using namespace Core::Math;
using namespace Core::Types;

/**
 * @brief 变换演示类
 * 
 * 展示如何使用矩阵进行2D和3D变换
 */
class TransformDemo {
private:
    Graphics* m_graphics;

public:
    TransformDemo(Graphics* graphics);
    
    /**
     * @brief 绘制旋转的三角形
     * @param centerX 中心x坐标
     * @param centerY 中心y坐标
     * @param angle 旋转角度（弧度）
     * @param scale 缩放因子
     * @param color 颜色
     */
    void drawRotatedTriangle(float centerX, float centerY, float angle, float scale, const Color& color);
    
    /**
     * @brief 绘制变换后的正方形
     * @param centerX 中心x坐标
     * @param centerY 中心y坐标
     * @param angle 旋转角度（弧度）
     * @param scaleX x方向缩放因子
     * @param scaleY y方向缩放因子
     * @param color 颜色
     */
    void drawTransformedQuad(float centerX, float centerY, float angle, 
                            float scaleX, float scaleY, const Color& color);
    
    /**
     * @brief 演示3D到2D的投影
     * @param time 时间参数用于动画
     */
    void draw3DProjectionDemo(float time);
    
    /**
     * @brief 演示向量运算
     * @param time 时间参数
     */
    void drawVectorDemo(float time);
};

#endif // TRANSFORM_DEMO_H
