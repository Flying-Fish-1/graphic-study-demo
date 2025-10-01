# 材质与光照

本文档描述材质参数、贴图采样与光照模型，反映当前 `Material`、`Light` 与渲染器着色阶段的行为。

## 材质参数（`Core::Types::Material`）

- 环境色 `ambient`、漫反射 `diffuse`、高光 `specular`、高光指数 `shininess`
- 漫反射贴图 `diffuseMap`（可选）
- 法线贴图 `normalMap`（可选，切线空间）

常用构造：`createDefaultMaterial`、`createRedPlastic`、`createBlueMetal`、`createWhiteDiffuse`。

### 贴图采样

- `sampleAlbedo(texCoord)`：根据 UV 采样漫反射贴图，若无贴图则返回 `diffuse`。
- `sampleAlbedo(texCoord, dudx, dudy, dvdx, dvdy)`：结合屏幕空间导数，便于后续扩展 MIP/过滤策略。
- `sampleNormal(texCoord)`：采样切线空间法线（范围映射到 [-1,1]），在着色阶段用 TBN 矩阵变换到世界空间。

## 光源（`Renderer::Lighting`）

- 抽象基类：`Light`（颜色、强度、环境强度），接口：
  - `getDirection(worldPos)`：片元处的光线方向
  - `getAttenuation(worldPos)`：衰减系数 [0,1]
  - `isVisible(worldPos)`：是否在照射范围内
- 点光：`PointLight(position, color, intensity, range)`
  - 方向：`normalize(position - worldPos)`
  - 衰减：常数/线性/二次项组合，超出 `range` 视为 0
- 方向光：`DirectionalLight(direction, color, intensity)`
  - 恒定方向与强度，无距离衰减

## 着色模型（`SoftwareRenderer::runShadingStage`）

1. 基础色：`base = vertexColor * sampleAlbedo(uv, dUV/dXY)`
2. 法线：若存在法线贴图，`n = normalize(T * nx + B * ny + N * nz)`；否则 `n = normalize(vertexNormal)`
3. 视线方向：`v = normalize(cameraPos - worldPos)`
4. 环境光：`ambient = sceneAmbient * base`
5. 对每个光源：
   - `l = normalize(getDirection(worldPos))`
   - `att = getAttenuation(worldPos)`；若 `att <= 0` 或 `dot(n,l)<=0`，跳过
   - `lightColor = light.color * (light.intensity * att)`
   - 漫反射：`diffuse = base * lightColor * max(dot(n,l),0)`
   - 半程向量：`h = normalize(l + v)`
   - 高光：`spec = specularColor * lightColor * pow(max(dot(n,h),0), shininess)`
6. 最终颜色：`clamp(ambient + Σ(diffuse + spec), 0..1)`，alpha 固定为 1

## 实用建议

- 缩放模型时注意法线归一化；TBN 需与顶点属性一致。
- 贴图坐标越界时可选择重复/钳制；当前实现以纹理类的采样逻辑为准。
- 视角变化明显时建议启用基于导数的采样接口，减少闪烁。



