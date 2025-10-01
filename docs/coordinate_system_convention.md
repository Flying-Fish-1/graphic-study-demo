# 坐标系与变换约定（项目实现版）

本文档与当前代码实现严格一致，采用 D3D/Vulkan 风格的左手坐标与 NDC 深度范围约定。

## 1. 坐标系定义

### 1.1 世界/相机坐标系（左手）
- **类型**: 左手坐标系
- **X轴**: 向右为正
- **Y轴**: 向上为正
- **Z轴**: 指向屏幕内部为正（远离相机为更大正值）

### 1.2 屏幕坐标系（像素栅格）
- **类型**: 左手坐标系
- **X轴**: 向右为正
- **Y轴**: 向下为正
- **Z轴/深度**: 近=0，远=1（与深度缓冲一致）

## 2. 矩阵与变换约定

### 2.1 视图矩阵 (View Matrix)
实现位于 `Scene::Camera::updateViewMatrix`，为左手坐标视图矩阵：
```cpp
m_viewMatrix = Matrix4({
    right.x,   right.y,   right.z,   -right.dot(m_position),
    up.x,      up.y,      up.z,      -up.dot(m_position),
    forward.x, forward.y, forward.z, -forward.dot(m_position),
    0, 0, 0, 1
});
```

### 2.2 透视投影矩阵 (Perspective)
实现位于 `Matrix4::perspective`，为左手/D3D 风格且 NDC 深度范围为 `[0,1]`：
```cpp
float f = 1.0f / std::tan(fov * 0.5f);
float range = far - near;
Matrix4{
    f / aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / range, -near * far / range,
    0, 0, 1, 0
};
```

### 2.3 视口矩阵 (Viewport)
将 NDC 映射到屏幕坐标（Y 轴翻转）：
```cpp
Matrix4::translation(screenW / 2.0f, screenH / 2.0f, 0.0f) *
Matrix4::scale(screenW / 2.0f, -screenH / 2.0f, 1.0f);
```

## 3. 几何体与法线约定

- 法向量统一为“指向面外为正”，背面剔除依赖面法线与视线方向点积。
- 立方体等几何的 Z 正方向朝“屏幕内”；请根据左手坐标系生成顶点/法线。

## 4. 变换顺序与乘法约定

1. 模型 → 世界（Model）
2. 世界 → 相机（View）
3. 相机 → 裁剪（Projection）
4. 透视除法：裁剪 → NDC
5. 视口：NDC → 屏幕

矩阵应用顺序（从右到左）：
```cpp
final = viewport * projection * view * model;
```

## 5. 深度测试约定

- NDC 深度范围：`[0,1]`（近=0，远=1）。
- 深度缓冲初始化为 `1.0`（最远）。
- 通过判定：`newDepth < storedDepth`（更小更近）。

实现细节提示（与代码保持一致）：
```cpp
// 当前光栅阶段使用：depth01 = ndcZ * 0.5f + 0.5f
// 该式用于将 [-1,1] → [0,1]。若全程采用 D3D 风格投影（NDC 已为 [0,1]），
// 可直接使用 depth01 = ndcZ 以避免重复映射。
```

## 6. 相机设置示例

```cpp
camera->setPerspective(Constants::PI / 3.0f, width / (float)height, 0.1f, 100.0f);
camera->lookAt(
    Vector3(3.0f, 3.0f, +6.0f),   // 位于 Z 正方向
    Vector3(0.0f, 0.0f, 0.0f),
    Vector3(0.0f, 1.0f, 0.0f)
);
```

## 7. 背面剔除约定

- 面法线 `n` 与视线方向 `v = normalize(cameraPos - p0)` 满足 `n·v <= 0` 时剔除（与实现一致）。

## 8. 调试建议

1. 检查视图矩阵第三行是否为 `forward`（左手坐标）。
2. 验证投影矩阵与 NDC 深度 `[0,1]` 的一致性。
3. 深度缓冲初值应为 `1.0`，并确保“更小更近”的比较逻辑。
4. 视口变换中的 Y 翻转是否正确映射到屏幕坐标。
