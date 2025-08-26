#ifndef CORE_TYPES_TEXTURE_H
#define CORE_TYPES_TEXTURE_H

#include "color.h"
#include <cstdint>
#include <string>

namespace Core {
namespace Types {

/**
 * @brief 纹理类
 * 
 * 用于存储和采样2D纹理数据
 */
class Texture {
private:
    uint32_t* m_pixels;     // 纹理像素数据
    int m_width;            // 纹理宽度
    int m_height;           // 纹理高度
    bool m_ownsData;        // 是否拥有数据的所有权
    
public:
    /**
     * @brief 构造函数
     * @param width 纹理宽度
     * @param height 纹理高度
     */
    Texture(int width, int height);
    
    /**
     * @brief 从现有数据构造纹理
     * @param pixels 像素数据
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param takeOwnership 是否取得数据所有权
     */
    Texture(uint32_t* pixels, int width, int height, bool takeOwnership = false);
    
    /**
     * @brief 析构函数
     */
    ~Texture();
    
    // 禁止拷贝构造和赋值
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    
    /**
     * @brief 在指定UV坐标处采样颜色
     * @param u 纹理坐标U (0-1)
     * @param v 纹理坐标V (0-1)
     * @return 采样的颜色
     */
    Color sample(float u, float v) const;
    
    /**
     * @brief 双线性插值采样
     * @param u 纹理坐标U (0-1)
     * @param v 纹理坐标V (0-1)
     * @return 插值后的颜色
     */
    Color sampleBilinear(float u, float v) const;
    
    /**
     * @brief 最近邻采样
     * @param u 纹理坐标U (0-1)
     * @param v 纹理坐标V (0-1)
     * @return 采样的颜色
     */
    Color sampleNearest(float u, float v) const;
    
    /**
     * @brief 设置指定像素的颜色
     * @param x 像素X坐标
     * @param y 像素Y坐标
     * @param color 颜色
     */
    void setPixel(int x, int y, const Color& color);
    
    /**
     * @brief 获取指定像素的颜色
     * @param x 像素X坐标
     * @param y 像素Y坐标
     * @return 像素颜色
     */
    Color getPixel(int x, int y) const;
    
    /**
     * @brief 清空纹理为指定颜色
     * @param color 填充颜色
     */
    void clear(const Color& color);
    
    /**
     * @brief 生成棋盘格纹理
     * @param color1 颜色1
     * @param color2 颜色2
     * @param squareSize 格子大小
     */
    void generateCheckerboard(const Color& color1, const Color& color2, int squareSize = 8);
    
    /**
     * @brief 生成渐变纹理
     * @param topColor 顶部颜色
     * @param bottomColor 底部颜色
     */
    void generateGradient(const Color& topColor, const Color& bottomColor);
    
    // Getter方法
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const uint32_t* getPixels() const { return m_pixels; }
    
    /**
     * @brief 从文件加载纹理
     * @param filename 文件名
     * @return 纹理指针，失败返回nullptr
     */
    static Texture* loadFromFile(const std::string& filename);
    
    /**
     * @brief 创建单色纹理
     * @param color 纹理颜色
     * @param width 纹理宽度
     * @param height 纹理高度
     * @return 纹理指针
     */
    static Texture* createSolidColor(const Color& color, int width = 64, int height = 64);
};

} // namespace Types
} // namespace Core

#endif // CORE_TYPES_TEXTURE_H
