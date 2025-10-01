#include <gtest/gtest.h>
#include "core/math/matrix.h"
#include "core/math/vector.h"

using namespace Core::Math;

TEST(MatrixPerspectiveTest, WSignIsNegativeZ) {
    float fov = 60.0f;             // degree
    float aspect = 1.0f;           // square viewport
    float nearZ = 0.1f;
    float farZ = 100.0f;

    Matrix4 proj = Matrix4::perspective(fov, aspect, nearZ, farZ);

    // 选择位于 -1 到 -far 范围内的 z
    Vector3 p(0.0f, 0.0f, 1.0f); // D3D-style, near plane z>0
    Vector4 hp(p, 1.0f);
    Vector4 tp = proj * hp; // 列主序矩阵乘运算

    EXPECT_NEAR(p.z, tp.w, 1e-4f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
