#include <gtest/gtest.h>
#include "core/math/matrix.h"
#include "core/math/vector.h"

using namespace Core::Math;

// 检查模型(单位矩阵)+视图(单位矩阵)+透视投影后，点落于 -1..1 范围
TEST(MVPChainTest, OriginOnNearPlane) {
    float fov = 60.0f;
    float aspect = 1.0f;
    float nearZ = 0.1f;
    float farZ = 100.0f;

    Matrix4 proj = Matrix4::perspective(fov, aspect, nearZ, farZ);
    Matrix4 view = Matrix4::identity();
    Matrix4 model = Matrix4::identity();

    Matrix4 mvp = proj * view * model;

    Vector4 posClip = mvp * Vector4(0.0f, 0.0f, -nearZ, 1.0f);
    Vector3 ndc(posClip.x / posClip.w, posClip.y / posClip.w, posClip.z / posClip.w);

    EXPECT_NEAR(ndc.x, 0.0f, 1e-4f);
    EXPECT_NEAR(ndc.y, 0.0f, 1e-4f);
    EXPECT_NEAR(ndc.z, -1.0f, 1e-4f); // OpenGL NDC 深度近裁剪面=-1
    EXPECT_LE(ndc.x, 1.0f);
    EXPECT_GE(ndc.x, -1.0f);
}