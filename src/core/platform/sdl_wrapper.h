#ifndef CORE_PLATFORM_SDL_WRAPPER_H
#define CORE_PLATFORM_SDL_WRAPPER_H

#include <SDL2/SDL.h>
#include "../types/color.h"

namespace Core {
namespace Platform {

using namespace Core::Types;

/**
 * @brief SDL平台抽象层
 */
class SDLWrapper {
private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_frameBuffer;
    uint32_t* m_pixelBuffer;
    int m_width, m_height;
    int m_pitch;
    bool m_renderSessionActive;

public:
    SDLWrapper();
    ~SDLWrapper();
    
    // 初始化和清理
    bool initSDL();
    bool createWindow();
    bool createRenderer();
    bool createFrameBuffer();
    void cleanup();
    
    // 渲染操作
    void clearScreen(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
    void setPixel(int x, int y, const Color& color);
    Color getPixel(int x, int y) const;
    void updateScreen();
    
    // 窗口操作
    void setWindowTitle(const char* title);
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    // 访问底层缓冲区和渲染器
    uint32_t* getPixelBuffer() { return m_pixelBuffer; }
    const uint32_t* getPixelBuffer() const { return m_pixelBuffer; }
    SDL_Renderer* getRenderer() { return m_renderer; }
    
    // 渲染会话管理 - 高效的批量像素操作
    bool beginRenderSession();
    void endRenderSession();
    bool isRenderSessionActive() const { return m_renderSessionActive; }
    
    // 私有的纹理锁定管理（仅内部使用）
private:
    bool lockFrameBuffer();
    void unlockFrameBuffer();
};

} // namespace Platform
} // namespace Core

#endif // CORE_PLATFORM_SDL_WRAPPER_H
