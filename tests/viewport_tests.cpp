#include <gtest/gtest.h>
#include "core/math/matrix.h"
#include "core/math/vector.h"

using namespace Core::Math;

static Matrix4 buildViewport(int w, int h) {
    return Matrix4::translation(w / 2.0f, h / 2.0f, 0.0f) *
           Matrix4::scale(w / 2.0f, -h / 2.0f, 1.0f);
}

TEST(ViewportMappingTest, MapNDCCornersToScreen) {
    int screenW = 800;
    int screenH = 600;
    Matrix4 viewport = buildViewport(screenW, screenH);

    // 左上角 (-1,1) -> (0,0)
    Vector4 ndcLT(-1.0f, 1.0f, 0.0f, 1.0f);
    Vector4 scrLT = viewport * ndcLT;
    EXPECT_NEAR(scrLT.x, 0.0f, 1e-4f);
    EXPECT_NEAR(scrLT.y, 0.0f, 1e-4f);

    // 右下角 (1,-1) -> (800,600)
    Vector4 ndcRB(1.0f, -1.0f, 0.0f, 1.0f);
    Vector4 scrRB = viewport * ndcRB;
    EXPECT_NEAR(scrRB.x, static_cast<float>(screenW), 1e-4f);
    EXPECT_NEAR(scrRB.y, static_cast<float>(screenH), 1e-4f);

    // 中心 (0,0) -> (400,300)
    Vector4 ndcCenter(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 scrCenter = viewport * ndcCenter;
    EXPECT_NEAR(scrCenter.x, screenW / 2.0f, 1e-4f);
    EXPECT_NEAR(scrCenter.y, screenH / 2.0f, 1e-4f);
}