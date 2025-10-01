#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 优先使用仓库附带的 ffmpeg 可执行文件
if [ -d "$SCRIPT_DIR/external/ffmpeg/bin" ]; then
    export PATH="$SCRIPT_DIR/external/ffmpeg/bin:$PATH"
fi

# 确保可以找到 Homebrew 安装的工具链
export PATH="/opt/homebrew/bin:$PATH"

BUILD_DIR="build"
CMAKE_FLAGS="-DENABLE_SDL_PREVIEW=ON"

echo "==========================================="
echo "     SDL 图形学演示程序启动脚本"
echo "==========================================="

echo "清理旧的构建结果..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR" || exit 1

echo "正在构建程序 (cmake $CMAKE_FLAGS).."
cmake $CMAKE_FLAGS ..
if [ $? -ne 0 ]; then
    echo "CMake 配置失败"
    exit 1
fi

cmake --build .
if [ $? -ne 0 ]; then
    echo "构建失败，请检查错误信息"
    exit 1
fi

echo "启动程序..."
echo "---------------------------------------------"
echo "按 ESC 键或关闭窗口退出程序"
echo "---------------------------------------------"

./graphic-study-demo "$@"

echo "---------------------------------------------"
echo "程序已退出"
echo "---------------------------------------------"
