#include "ssaa.h"
#include <algorithm>
#include <vector>

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

    std::vector<Color> horizontal(static_cast<std::size_t>(outW * inH), Color(0, 0, 0, 0));
    for (int y = 0; y < inH; ++y) {
        for (int x = 0; x < outW; ++x) {
            int startX = x * factor;
            int endX = std::min(startX + factor, inW);
            Color sum(0, 0, 0, 0);
            int count = 0;
            for (int xx = startX; xx < endX; ++xx) {
                sum = sum + highRes.getPixel(xx, y);
                ++count;
            }
            if (count > 0) {
                horizontal[static_cast<std::size_t>(y * outW + x)] = sum * (1.0f / static_cast<float>(count));
            }
        }
    }

    for (int y = 0; y < outH; ++y) {
        int startY = y * factor;
        int endY = std::min(startY + factor, inH);
        for (int x = 0; x < outW; ++x) {
            Color sum(0, 0, 0, 0);
            int count = 0;
            for (int yy = startY; yy < endY; ++yy) {
                sum = sum + horizontal[static_cast<std::size_t>(yy * outW + x)];
                ++count;
            }
            if (count > 0) {
                lowRes.setPixel(x, y, sum * (1.0f / static_cast<float>(count)));
            }
        }
    }
}

} // namespace Effects
} // namespace Renderer

