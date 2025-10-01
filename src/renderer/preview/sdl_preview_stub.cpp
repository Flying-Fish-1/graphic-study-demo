#include "sdl_preview.h"
#include <iostream>

namespace Renderer {
namespace Preview {

SdlPreview::SdlPreview(int width, int height) : m_width(width), m_height(height) {}

SdlPreview::~SdlPreview() = default;

bool SdlPreview::initialize() {
    return false;
}

void SdlPreview::present(const Pipeline::RenderTarget&, const std::string& windowTitle) {
    presentFallbackMessage(windowTitle);
}

void SdlPreview::presentFallbackMessage(const std::string& windowTitle) const {
    std::cerr << "SDL 预览功能未启用，无法显示窗口: " << windowTitle << std::endl;
}

} // namespace Preview
} // namespace Renderer
