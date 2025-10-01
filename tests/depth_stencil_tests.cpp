#include <gtest/gtest.h>
#include "renderer/pipeline/render_target.h"

using namespace Renderer::Pipeline;

namespace {
constexpr int W = 4;
constexpr int H = 3;
constexpr float FAR_DEPTH = 1.0f;
constexpr float NEAR_DEPTH = 0.3f;
}

TEST(RenderTargetDepthTest, ClearAndGetDepth) {
    RenderTarget target(W, H);
    target.clear(Core::Types::Color::BLACK, FAR_DEPTH);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            EXPECT_FLOAT_EQ(target.getDepth(x, y), FAR_DEPTH);
        }
    }
}

TEST(RenderTargetDepthTest, DepthTestAndSetPassesForCloserFragment) {
    RenderTarget target(W, H);
    target.clear(Core::Types::Color::BLACK, FAR_DEPTH);
    EXPECT_TRUE(target.depthTestAndSet(1, 1, NEAR_DEPTH));
    EXPECT_FLOAT_EQ(target.getDepth(1, 1), NEAR_DEPTH);
}

TEST(RenderTargetDepthTest, DepthTestAndSetRejectsFartherFragment) {
    RenderTarget target(W, H);
    target.clear(Core::Types::Color::BLACK, NEAR_DEPTH);
    EXPECT_FALSE(target.depthTestAndSet(2, 2, FAR_DEPTH));
    EXPECT_FLOAT_EQ(target.getDepth(2, 2), NEAR_DEPTH);
}

TEST(RenderTargetDepthTest, DepthPassQueryWorks) {
    RenderTarget target(W, H);
    target.clear(Core::Types::Color::BLACK, FAR_DEPTH);
    target.setDepth(0, 0, NEAR_DEPTH);
    EXPECT_TRUE(target.depthPasses(0, 0, 0.2f));
    EXPECT_FALSE(target.depthPasses(0, 0, 0.8f));
}
