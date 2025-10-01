# 软件光栅化器架构说明（与当前实现同步）

## 模块概览

- `renderer/pipeline/geometry_stage.*`：几何阶段，生成 `GeometryVertex`（裁剪坐标、世界坐标、法线/切线空间、纹理坐标、颜色、1/w、ndcZ）。
- `renderer/pipeline/software_renderer.*`：调度核心，按“几何 → 组装 → 光栅化 → 着色”执行并写入 `RenderTarget`。
- `renderer/pipeline/render_target.*`：输出合并与深度缓冲，支持清屏、深度测试与保存 `PPM`。
- `renderer/lighting/*`：光照接口与点光/方向光实现。
- `scene/*`：场景对象、相机、光源管理。
- `core/types/*`：材质、纹理、顶点、颜色等基础类型。
- `main.cpp`：程序入口，解析命令行参数、搭建场景并触发渲染/预览/保存。

## 渲染流水线

1. 几何阶段（`GeometryVertex::fromVertex`）
   - 应用 `model * view * projection`，计算裁剪坐标与 `1/w`、`ndcZ`。
   - 变换法线/切线/副切线到世界空间。

2. 组装与剔除（`SoftwareRenderer::runPrimitiveAssembly`）
   - 计算三角形面法线并归一化。
   - 背面剔除：`dot(faceNormal, normalize(cameraPos - p0)) <= 0` 时丢弃。

3. 光栅化（`SoftwareRenderer::runRasterStage`）
   - 计算屏幕包围盒并逐像素重心插值；可选择透视正确插值。
   - 生成纹理导数 `dudx/dudy/dvdx/dvdy`（用于纹理采样 LOD/过滤）。
   - 深度测试：`depthTestAndSet(x, y, depth01)`，深度越小越近。

4. 着色（`SoftwareRenderer::runShadingStage`）
   - 取材质基础色与漫反射贴图，叠加法线贴图（TBN 转换）。
   - 按 Blinn-Phong 计算漫反射与高光，加入场景环境光。

5. 输出合并
   - 写入 `RenderTarget` 颜色缓冲；可保存为 `PPM` 或经 SDL 预览显示。

## 坐标与深度约定

- 左手坐标；NDC 深度范围 `[0,1]`。
- 投影矩阵：`Matrix4::perspective`（D3D 风格）。
- 视图矩阵：`Camera::updateViewMatrix` 第三行是 `forward`。
- 视口映射：`translation(w/2,h/2,0) * scale(w/2,-h/2,1)`。

当前代码在光栅阶段对 `ndcZ` 使用 `depth01 = ndcZ * 0.5f + 0.5f` 做了从 `[-1,1]` 到 `[0,1]` 的映射；若全程坚持 D3D 风格（投影已输出 `[0,1]`），可将该步骤等价化为直接使用 `ndcZ`，但需保持全局一致性与测试通过。

## 命令行参数

```
./graphic-study-demo [--width=<像素>] [--height=<像素>] [--output=<文件>] [--save] [--camera-distance=<值>] [--preview|--no-preview]
```

- `--width`/`--height`：渲染尺寸（默认 1280×720）。
- `--output`/`--save`：保存为 PPM 文件。
- `--camera-distance`：设置相机到目标的距离。
- `--preview`/`--no-preview`：开启/关闭 SDL 预览窗口（需 `ENABLE_SDL_PREVIEW`）。

## 性能与正确性要点

- 使用包围盒限制像素范围；跳过退化三角形与非有限导数。
- 透视正确插值对纹理坐标与法线尤为重要。
- 纹理导数用于 MIP 与过滤选择；无导数时可能出现闪烁。
- 深度缓冲初值 1.0，比较逻辑为“小于即通过”。
