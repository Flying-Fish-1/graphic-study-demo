# 架构总览

本项目实现了一个可测试的、模块化的软件光栅化渲染器。本文概览模块职责、关键数据结构与数据流。

## 目录结构（核心）

```
src/
├─ core/
│  ├─ math/            数学库（向量/矩阵/常量）
│  └─ types/           基础类型（颜色/纹理/材质/顶点/三角形）
├─ scene/              场景系统（相机/网格/场景对象/光源注册）
├─ renderer/
│  ├─ lighting/        光照抽象与实现（点光/方向光）
│  └─ pipeline/        渲染管线（几何/光栅/着色/RenderTarget）
└─ main.cpp            程序入口（参数解析、场景搭建、渲染/保存/预览）
```

## 关键数据结构

- `Core::Types::Vertex`：位置、法线、切线/副切线、纹理坐标、顶点色。
- `Renderer::Pipeline::GeometryVertex`：裁剪坐标、世界坐标、法线/切线空间、纹理坐标、顶点色、`reciprocalW`、`ndcZ`。
- `Renderer::Pipeline::RenderTarget`：颜色缓冲与深度缓冲，`depthTestAndSet` 实现“小于即通过”。
- `Core::Types::Material`：漫反射/高光参数、漫反射/法线贴图采样，提供 `sampleAlbedo` 与 `sampleNormal`。
- `Renderer::Lighting::Light`：光源接口；`PointLight`、`DirectionalLight` 提供方向、衰减与可见性判断。

## 数据流（逐对象逐三角形）

1. `GeometryStage`：`fromVertex` 生成 `GeometryVertex`，计算 `clipPosition`、`1/w`、`ndcZ`，并变换 TBN。
2. `PrimitiveAssembly`：根据索引组装三角形，计算面法线并执行背面剔除。
3. `RasterStage`：包围盒扫描 → 重心插值（可透视校正）→ 深度测试 → 生成纹理导数。
4. `ShadingStage`：采样材质与法线，按 Blinn-Phong 叠加场景环境光与各光源贡献。
5. `Output`：写入 `RenderTarget`，支持保存 PPM 与 SDL 预览。

## 坐标系与深度

- 左手坐标；NDC 深度 `[0,1]`；视口 Y 轴向下。
- 详见 `docs/coordinate_system_convention.md`。

## 可测试性

- 通过 `tests/` 下的单元测试覆盖：投影/裁剪、视口映射、深度/模板语义（深度）、法线变换、材质光照。
- 测试工程链接核心实现源文件，独立于 SDL 预览。

## 扩展点

- 新光源类型（聚光）、阴影映射占位、MSAA/SSAA、纹理过滤与 MIP 策略完善、Gamma 矫正与色彩管理。



