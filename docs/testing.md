# 测试说明

本项目使用 GoogleTest 进行单元测试，覆盖矩阵/向量、裁剪/视口、深度缓冲、法线变换与材质光照等关键逻辑。

## 构建与运行

默认启用测试构建：

```bash
mkdir -p build && cd build
cmake -DBUILD_TESTING=ON ..
cmake --build .
ctest --output-on-failure
```

或直接运行可执行文件：

```bash
./tests/graphic-study-demo_tests
```

## 覆盖点概览（对应 `tests/*.cpp`）

- `unit_tests.cpp`
  - 透视矩阵基本性质（透视后 w 与 z 的关系）。

- `transform_tests.cpp`
  - MVP 链路与 NDC 映射断言（示例基于 OpenGL 约定的注释对照）。

- `clip_tests.cpp`
  - 超出远平面时的 NDC.z 行为（D3D 深度范围 [0,1] 预期）。

- `viewport_tests.cpp`
  - 视口矩阵将 NDC 角点映射到屏幕像素的正确性（含 Y 翻转）。

- `depth_stencil_tests.cpp`
  - 深度缓冲清空、比较与写入逻辑：`newDepth < storedDepth` 通过。

- `vertex_normals_tests.cpp`
  - 法线方向变换的正确性（以 Z 轴旋转 90° 为例）。

- `material_lighting_tests.cpp`
  - Blinn-Phong 模型的边界情形：正向入射（有漫反+高光）、背向（仅环境）。

## 注意事项

- 测试工程直接链接核心实现源文件，不依赖 SDL 预览。
- 若修改坐标与深度约定，请同步更新断言逻辑与相关文档。
- 在 CI/本地环境差异较大时，浮点断言使用 `EXPECT_NEAR`，确保误差容忍。



