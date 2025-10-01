#include <gtest/gtest.h>
#include "../src/core/math/matrix.h"
#include "../src/core/math/vector.h"

using namespace Core::Math;

namespace {
constexpr float EPS = 1e-5f;
}

TEST(VertexNormalTransformTest, RotationZ90Degrees) {
    // 原始法线向量 (1,0,0)
    Vector3 normal(1.0f, 0.0f, 0.0f);
    // 绕 Z 轴旋转 90°，期望法线变为 (0,1,0)
    Matrix4 rotZ = Matrix4::rotationZ(static_cast<float>(M_PI_2));
    Vector3 transformed = rotZ.transformDirection(normal).normalize();
    EXPECT_NEAR(transformed.x, 0.0f, EPS);
    EXPECT_NEAR(transformed.y, 1.0f, EPS);
    EXPECT_NEAR(transformed.z, 0.0f, EPS);
}