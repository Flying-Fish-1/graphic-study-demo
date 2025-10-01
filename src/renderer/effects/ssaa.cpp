#include "ssaa.h"
#include <algorithm>

namespace Renderer {
namespace Effects {

using Core::Types::Color;
using Renderer::Pipeline::RenderTarget;

void resolveBox(const RenderTarget& highRes, RenderTarget& lowRes, int factor) {
    if (factor <= 1) {
        // 直接拷贝（假设尺寸一致）
        int w = std::min(lowRes.getWidth(), highRes.getWidth());
        int h = std::min(lowRes.getHeight(), highRes.getHeight());
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                lowRes.setPixel(x, y, highRes.getPixel(x, y));
            }
        }
        return;
    }

    int outW = lowRes.getWidth();
    int outH = lowRes.getHeight();
    int inW = highRes.getWidth();
    int inH = highRes.getHeight();

    // 约束：输入应为 out*factor 尺寸
    for (int y = 0; y < outH; ++y) {
        for (int x = 0; x < outW; ++x) {
            int startX = x * factor;
            int startY = y * factor;
            int endX = std::min(startX + factor, inW);
            int endY = std::min(startY + factor, inH);
            Color sum(0, 0, 0, 0);
            int count = 0;
            for (int yy = startY; yy < endY; ++yy) {
                for (int xx = startX; xx < endX; ++xx) {
                    sum = sum + highRes.getPixel(xx, yy);
                    ++count;
                }
            }
            if (count > 0) {
                Color avg = sum * (1.0f / static_cast<float>(count));
                lowRes.setPixel(x, y, avg);
            }
        }
    }
}

} // namespace Effects
} // namespace Renderer


