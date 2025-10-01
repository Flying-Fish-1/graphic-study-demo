#!/bin/bash

echo "🧪 开始测试修复效果..."
echo "=================================="

# 编译程序
echo "🔧 编译程序..."
cd build
make clean && make

if [ $? -eq 0 ]; then
    echo "✅ 编译成功"
else
    echo "❌ 编译失败"
    exit 1
fi

echo ""
echo "🎯 测试深度测试修复..."
echo "预期改进："
echo "  - 深度测试使用NDC空间z值(0-1范围)而不是视图空间z值"
echo "  - 深度测试应该更准确"

echo ""
echo "🎨 测试材质和光照修复..."
echo "预期改进："
echo "  - 5种不同材质：红色高反射、蓝色金属、绿色塑料、金色金属、灰色塑料"
echo "  - 3种光源：点光源(暖色)、方向光(太阳光)、聚光灯(冷色)"
echo "  - 颜色值使用正确的0-1范围格式"
echo "  - 更丰富的光照效果"

echo ""
echo "🎮 控制说明："
echo "  - 空格键：切换到3D场景演示"
echo "  - M键：切换材质类型"
echo "  - W键：切换线框模式"
echo "  - R键：切换对象旋转"
echo "  - O键：切换显示对象"
echo "  - C键：重置相机位置"
echo "  - ESC键：退出程序"

echo ""
echo "🚀 启动程序进行测试..."
echo "请观察："
echo "  1. 深度测试是否正常工作"
echo "  2. 不同材质的光照效果"
echo "  3. 多种光源的叠加效果"
echo "  4. 材质切换时的视觉变化"

# 运行程序
./graphic-study-demo
