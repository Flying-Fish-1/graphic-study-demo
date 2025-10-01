#include <gtest/gtest.h>

#include "core/types/material.h"
#include "core/math/vector.h"

using namespace Core::Types;
using namespace Core::Math;

TEST(MaterialLightingTest, FacingLightProducesExpectedColor) {
    Material material;

    Vector3 normal(0, 0, 1);
    Vector3 lightDir(0, 0, 1);
    Vector3 viewDir(0, 0, 1);
    Vector3 worldPos(0, 0, 0);

    Color result = material.calculateLighting(normal, lightDir, viewDir, worldPos);

    float ambient = material.getAmbient().r * 0.1f;
    float diffuse = material.getDiffuse().r * 1.0f;
    float specular = material.getSpecular().r * 1.0f;
    float expected = ambient + diffuse + specular;

    const float kEpsilon = 1e-4f;
    EXPECT_NEAR(result.r, expected, kEpsilon);
    EXPECT_NEAR(result.g, expected, kEpsilon);
    EXPECT_NEAR(result.b, expected, kEpsilon);
}

TEST(MaterialLightingTest, LightBehindSurfaceProducesAmbientOnly) {
    Material material;

    Vector3 normal(0, 0, 1);
    Vector3 lightDir(0, 0, -1);
    Vector3 viewDir(0, 0, 1);
    Vector3 worldPos(0, 0, 0);

    Color result = material.calculateLighting(normal, lightDir, viewDir, worldPos);

    float ambient = material.getAmbient().r * 0.1f;
    const float kEpsilon = 1e-4f;
    EXPECT_NEAR(result.r, ambient, kEpsilon);
    EXPECT_NEAR(result.g, ambient, kEpsilon);
    EXPECT_NEAR(result.b, ambient, kEpsilon);
}
