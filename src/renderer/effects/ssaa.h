#ifndef RENDERER_EFFECTS_SSAA_H
#define RENDERER_EFFECTS_SSAA_H

#include "renderer/pipeline/render_target.h"

namespace Renderer {
namespace Effects {

// 简单的盒滤 SSAA resolve：把高分辨率颜色缓冲按 factor×factor 做平均，下采样到目标
void resolveBox(const Pipeline::RenderTarget& highRes,
                Pipeline::RenderTarget& lowRes,
                int factor);

} // namespace Effects
} // namespace Renderer

#endif // RENDERER_EFFECTS_SSAA_H


