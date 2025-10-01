# 构建与运行

本文档说明依赖、构建、运行与常用参数，覆盖带/不带 SDL 预览两种模式。

## 依赖

- C++17 编译器（Clang/GCC/MSVC 均可）
- CMake >= 3.10
- 可选：SDL2（用于实时预览窗口），本仓库已在 `external/SDL2` 提供静态/动态库与头文件
- 可选：GTest（`external/googletest` 已内置，用于单元测试）

## 一键脚本（推荐）

```bash
./start.sh --preview            # 默认带预览
./start.sh --no-preview --save --output=output.ppm
```

脚本会：清理/生成 `build/`、配置 CMake（默认 `-DENABLE_SDL_PREVIEW=ON`）、编译并运行。

## 手动构建

```bash
mkdir -p build && cd build
cmake -DENABLE_SDL_PREVIEW=ON ..      # 如无需窗口：-DENABLE_SDL_PREVIEW=OFF
cmake --build .
./graphic-study-demo --save --output=out.ppm
```

若开启预览，CMake 会在 `external/SDL2` 下查找 SDL2；也可将该目录替换为系统安装路径，并更新 CMake 变量。

## 运行参数

```text
--width=<像素>              渲染宽度（默认 1280）
--height=<像素>             渲染高度（默认 720）
--camera-distance=<值>      相机距离（默认 6.0）
--preview / --no-preview     强制开/关 SDL 预览
--save                       保存 PPM 输出
--output=<文件名>           指定输出文件名
```

示例：

```bash
./graphic-study-demo --width=1024 --height=768 --preview
./graphic-study-demo --no-preview --save --output=render.ppm
```

## 常见问题

- 找不到 SDL2：确认 `external/SDL2/lib` 下存在库文件，并由 CMake 检测到；或关闭预览。
- 仅保存图片：加入 `--save` 或 `--output` 参数。
- 输出为黑图：检查相机位置/目标、光照强度、材质贴图路径与分辨率参数。



