#ifndef RENDERER_PREVIEW_SDL_PREVIEW_H
#define RENDERER_PREVIEW_SDL_PREVIEW_H

#include <string>

#include "../pipeline/render_target.h"

namespace Renderer {
namespace Preview {

class SdlPreview {
public:
    SdlPreview(int width, int height);
    ~SdlPreview();

    bool initialize();
    void present(const Pipeline::RenderTarget& target, const std::string& windowTitle);

    // 非阻塞：上传并呈现一帧，不进入内部事件循环
    bool presentOnce(const Pipeline::RenderTarget& target, const std::string& windowTitle);
    // 轮询事件：返回false表示收到退出请求
    bool pollEvents();

private:
    int m_width;
    int m_height;

#ifdef ENABLE_SDL_PREVIEW
    struct SDLObjects;
    SDLObjects* m_objects;
#else
    void presentFallbackMessage(const std::string& windowTitle) const;
#endif
};

} // namespace Preview
} // namespace Renderer

#endif // RENDERER_PREVIEW_SDL_PREVIEW_H
