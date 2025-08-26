#ifndef RENDERER_PIPELINE_DEPTH_BUFFER_H
#define RENDERER_PIPELINE_DEPTH_BUFFER_H

namespace Renderer {
namespace Pipeline {

/**
 * @brief 深度缓冲类
 * 
 * 用于3D渲染中的深度测试，解决遮挡关系
 */
class DepthBuffer {
private:
    float* m_buffer;        // 深度缓冲区
    int m_width;            // 缓冲区宽度
    int m_height;           // 缓冲区高度
    bool m_enabled;         // 是否启用深度测试
    
public:
    /**
     * @brief 构造函数
     * @param width 缓冲区宽度
     * @param height 缓冲区高度
     */
    DepthBuffer(int width, int height);
    
    /**
     * @brief 析构函数
     */
    ~DepthBuffer();
    
    // 禁止拷贝构造和赋值
    DepthBuffer(const DepthBuffer&) = delete;
    DepthBuffer& operator=(const DepthBuffer&) = delete;
    
    /**
     * @brief 清空深度缓冲区
     * @param depth 清空深度值 (默认1.0为最远)
     */
    void clear(float depth = 1.0f);
    
    /**
     * @brief 深度测试并设置
     * @param x 像素X坐标
     * @param y 像素Y坐标
     * @param depth 深度值
     * @return 如果通过深度测试返回true
     */
    bool testAndSet(int x, int y, float depth);
    
    /**
     * @brief 只进行深度测试，不设置深度值
     * @param x 像素X坐标
     * @param y 像素Y坐标
     * @param depth 深度值
     * @return 如果通过深度测试返回true
     */
    bool test(int x, int y, float depth) const;
    
    /**
     * @brief 获取指定位置的深度值
     * @param x 像素X坐标
     * @param y 像素Y坐标
     * @return 深度值
     */
    float getDepth(int x, int y) const;
    
    /**
     * @brief 设置指定位置的深度值
     * @param x 像素X坐标
     * @param y 像素Y坐标
     * @param depth 深度值
     */
    void setDepth(int x, int y, float depth);
    
    /**
     * @brief 调整缓冲区大小
     * @param width 新宽度
     * @param height 新高度
     */
    void resize(int width, int height);
    
    /**
     * @brief 启用/禁用深度测试
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    /**
     * @brief 检查深度测试是否启用
     * @return 深度测试状态
     */
    bool isEnabled() const { return m_enabled; }
    
    // Getter方法
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const float* getBuffer() const { return m_buffer; }
    
private:
    /**
     * @brief 检查坐标是否在有效范围内
     * @param x 像素X坐标
     * @param y 像素Y坐标
     * @return 坐标是否有效
     */
    bool isValidCoordinate(int x, int y) const;
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_DEPTH_BUFFER_H
