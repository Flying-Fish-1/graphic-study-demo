#include "sdl_wrapper.h"
#include <iostream>
#include <cstring>

// 从旧常量中获取屏幕参数
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define WINDOW_TITLE "Graphics Demo - New Architecture"

namespace Core {
namespace Platform {

SDLWrapper::SDLWrapper() 
    : m_window(nullptr), m_renderer(nullptr), m_frameBuffer(nullptr), 
      m_pixelBuffer(nullptr), m_pitch(0), m_width(SCREEN_WIDTH), m_height(SCREEN_HEIGHT),
      m_renderSessionActive(false) {
}

SDLWrapper::~SDLWrapper() {
    cleanup();
}

bool SDLWrapper::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL 初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool SDLWrapper::createWindow() {
    m_window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        m_width, m_height,
        SDL_WINDOW_SHOWN
    );

    if (!m_window) {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_RaiseWindow(m_window);
    return true;
}

bool SDLWrapper::createRenderer() {
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool SDLWrapper::createFrameBuffer() {
    m_frameBuffer = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        m_width, m_height
    );

    if (!m_frameBuffer) {
        std::cerr << "帧缓冲创建失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 不在这里锁定纹理，而是在需要时锁定
    m_pixelBuffer = nullptr;
    return true;
}

void SDLWrapper::cleanup() {
    if (m_frameBuffer) {
        SDL_DestroyTexture(m_frameBuffer);
        m_frameBuffer = nullptr;
    }

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}

void SDLWrapper::clearScreen(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    void* pixels;
    int pitch;
    
    if (SDL_LockTexture(m_frameBuffer, nullptr, &pixels, &pitch) == 0) {
        uint32_t* pixelBuffer = static_cast<uint32_t*>(pixels);
        uint32_t color = (r << 24) | (g << 16) | (b << 8) | a;
        
        for (int i = 0; i < m_width * m_height; i++) {
            pixelBuffer[i] = color;
        }
        
        SDL_UnlockTexture(m_frameBuffer);
    }
}

void SDLWrapper::setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        if (m_renderSessionActive && m_pixelBuffer) {
            // 渲染会话期间：高效的直接缓冲区访问
            uint32_t color = (r << 24) | (g << 16) | (b << 8) | 0xFF;
            m_pixelBuffer[y * m_width + x] = color;
        } else {
            // 非渲染会话期间：传统的锁定/解锁方式（兼容性）
            void* pixels;
            int pitch;
            
            if (SDL_LockTexture(m_frameBuffer, nullptr, &pixels, &pitch) == 0) {
                uint32_t* pixelBuffer = static_cast<uint32_t*>(pixels);
                uint32_t color = (r << 24) | (g << 16) | (b << 8) | 0xFF;
                pixelBuffer[y * m_width + x] = color;
                SDL_UnlockTexture(m_frameBuffer);
            }
        }
    }
}

void SDLWrapper::setPixel(int x, int y, const Color& color) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        if (m_renderSessionActive && m_pixelBuffer) {
            // 渲染会话期间：高效的直接缓冲区访问
            m_pixelBuffer[y * m_width + x] = color.toUint32();
        } else {
            // 非渲染会话期间：传统的锁定/解锁方式（兼容性）
            void* pixels;
            int pitch;
            
            if (SDL_LockTexture(m_frameBuffer, nullptr, &pixels, &pitch) == 0) {
                uint32_t* pixelBuffer = static_cast<uint32_t*>(pixels);
                pixelBuffer[y * m_width + x] = color.toUint32();
                SDL_UnlockTexture(m_frameBuffer);
            }
        }
    }
}

Color SDLWrapper::getPixel(int x, int y) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        void* pixels;
        int pitch;
        
        if (SDL_LockTexture(m_frameBuffer, nullptr, &pixels, &pitch) == 0) {
            uint32_t* pixelBuffer = static_cast<uint32_t*>(pixels);
            uint32_t color = pixelBuffer[y * m_width + x];
            SDL_UnlockTexture(m_frameBuffer);
            return Color::fromUint32(color);
        }
    }
    return Color::BLACK;
}

void SDLWrapper::updateScreen() {
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_frameBuffer, nullptr, nullptr);
    SDL_RenderPresent(m_renderer);
}

void SDLWrapper::setWindowTitle(const char* title) {
    if (m_window) {
        SDL_SetWindowTitle(m_window, title);
    }
}

bool SDLWrapper::lockFrameBuffer() {
    void* pixels;
    if (SDL_LockTexture(m_frameBuffer, nullptr, &pixels, &m_pitch) == 0) {
        m_pixelBuffer = static_cast<uint32_t*>(pixels);
        return true;
    }
    return false;
}

void SDLWrapper::unlockFrameBuffer() {
    SDL_UnlockTexture(m_frameBuffer);
    m_pixelBuffer = nullptr;
}

bool SDLWrapper::beginRenderSession() {
    if (m_renderSessionActive) {
        std::cerr << "渲染会话已经激活" << std::endl;
        return false;
    }
    
    if (lockFrameBuffer()) {
        m_renderSessionActive = true;
        return true;
    }
    return false;
}

void SDLWrapper::endRenderSession() {
    if (m_renderSessionActive) {
        unlockFrameBuffer();
        m_renderSessionActive = false;
    }
}

} // namespace Platform
} // namespace Core
