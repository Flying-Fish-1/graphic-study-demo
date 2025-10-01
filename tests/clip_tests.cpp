#include <gtest/gtest.h>
#include "core/math/matrix.h"
#include "core/math/vector.h"

using namespace Core::Math;

// 验证位于远裁剪面之外的顶点经投影后 NDC.z 超出 [-1,1]
TEST(ClipStageTest, VertexOutsideFarPlaneIsClipped) {
    float fov = 60.0f;
    float aspect = 1.0f;
    float nearZ = 0.1f;
    float farZ = 100.0f;

    Matrix4 proj = Matrix4::perspective(fov, aspect, nearZ, farZ);

    // 构造位于远裁剪面外的顶点 (z > farZ)
    Vector4 posWorld(0.0f, 0.0f, 150.0f, 1.0f);

    Vector4 clipPos = proj * posWorld;
    Vector3 ndc(clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w);

    // 远裁剪面外应使 ndc.z > 1 (D3D 深度范围 [0,1])
    EXPECT_GT(ndc.z, 1.0f);
}
