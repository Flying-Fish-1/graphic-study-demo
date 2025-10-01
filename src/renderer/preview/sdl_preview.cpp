#include "sdl_preview.h"

#ifdef ENABLE_SDL_PREVIEW

#include <SDL2/SDL.h>
#include <vector>
#include <algorithm>

namespace Renderer {
namespace Preview {

struct SdlPreview::SDLObjects {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_PixelFormat* pixelFormat = nullptr;
};

SdlPreview::SdlPreview(int width, int height)
    : m_width(width), m_height(height), m_objects(new SDLObjects()) {}

SdlPreview::~SdlPreview() {
    if (!m_objects) {
        return;
    }

    if (m_objects->texture) {
        SDL_DestroyTexture(m_objects->texture);
    }
    if (m_objects->renderer) {
        SDL_DestroyRenderer(m_objects->renderer);
    }
    if (m_objects->window) {
        SDL_DestroyWindow(m_objects->window);
    }
    if (m_objects->pixelFormat) {
        SDL_FreeFormat(m_objects->pixelFormat);
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    delete m_objects;
}

bool SdlPreview::initialize() {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL INIT failed: %s", SDL_GetError());
        return false;
    }

    m_objects->window = SDL_CreateWindow(
        "Software Renderer Preview",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_width,
        m_height,
        SDL_WINDOW_SHOWN);
    if (!m_objects->window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return false;
    }

    m_objects->renderer = SDL_CreateRenderer(m_objects->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_objects->renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        return false;
    }

    m_objects->texture = SDL_CreateTexture(m_objects->renderer,
                                           SDL_PIXELFORMAT_RGBA8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           m_width,
                                           m_height);
    if (!m_objects->texture) {
        SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
        return false;
    }

    m_objects->pixelFormat = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    return m_objects->pixelFormat != nullptr;
}

void SdlPreview::present(const Pipeline::RenderTarget& target, const std::string& windowTitle) {
    if (!m_objects || !m_objects->renderer || !m_objects->texture) {
        return;
    }

    SDL_SetWindowTitle(m_objects->window, windowTitle.c_str());

    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(m_objects->texture, nullptr, &pixels, &pitch) != 0) {
        SDL_Log("SDL_LockTexture failed: %s", SDL_GetError());
        return;
    }

    const auto& colorBuffer = target.getColorBuffer();

    for (int y = 0; y < m_height; ++y) {
        auto* row = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pixels) + y * pitch);
        for (int x = 0; x < m_width; ++x) {
            const auto& color = colorBuffer[y * m_width + x];
            uint8_t r = static_cast<uint8_t>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
            uint8_t g = static_cast<uint8_t>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
            uint8_t b = static_cast<uint8_t>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
            uint8_t a = static_cast<uint8_t>(std::clamp(color.a, 0.0f, 1.0f) * 255.0f);
            row[x] = SDL_MapRGBA(m_objects->pixelFormat, r, g, b, a);
        }
    }

    SDL_UnlockTexture(m_objects->texture);

    SDL_RenderClear(m_objects->renderer);
    SDL_RenderCopy(m_objects->renderer, m_objects->texture, nullptr, nullptr);
    SDL_RenderPresent(m_objects->renderer);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = false;
                break;
            }
        }
        SDL_Delay(16);
    }
}

bool SdlPreview::presentOnce(const Pipeline::RenderTarget& target, const std::string& windowTitle) {
    if (!m_objects || !m_objects->renderer || !m_objects->texture) {
        return false;
    }

    SDL_SetWindowTitle(m_objects->window, windowTitle.c_str());

    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(m_objects->texture, nullptr, &pixels, &pitch) != 0) {
        SDL_Log("SDL_LockTexture failed: %s", SDL_GetError());
        return false;
    }

    const auto& colorBuffer = target.getColorBuffer();
    for (int y = 0; y < m_height; ++y) {
        auto* row = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pixels) + y * pitch);
        for (int x = 0; x < m_width; ++x) {
            const auto& color = colorBuffer[y * m_width + x];
            uint8_t r = static_cast<uint8_t>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
            uint8_t g = static_cast<uint8_t>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
            uint8_t b = static_cast<uint8_t>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
            uint8_t a = static_cast<uint8_t>(std::clamp(color.a, 0.0f, 1.0f) * 255.0f);
            row[x] = SDL_MapRGBA(m_objects->pixelFormat, r, g, b, a);
        }
    }

    SDL_UnlockTexture(m_objects->texture);

    SDL_RenderClear(m_objects->renderer);
    SDL_RenderCopy(m_objects->renderer, m_objects->texture, nullptr, nullptr);
    SDL_RenderPresent(m_objects->renderer);
    return true;
}

bool SdlPreview::pollEvents() {
    if (!m_objects) return false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
            return false;
        }
    }
    return true;
}

} // namespace Preview
} // namespace Renderer

#endif // ENABLE_SDL_PREVIEW
