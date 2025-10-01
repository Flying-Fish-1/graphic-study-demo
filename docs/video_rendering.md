# 渲染模式与输出

程序通过 `--mode=<preview|png|video>` 选择互斥的三种输出方式：

1. **preview**（默认）
   - `./start.sh --mode=preview`
   - 仅打开 SDL2 实时预览窗口，场景随时间旋转，不写入任何文件。

2. **png**
   - `./build/graphic-study-demo --mode=png --output=result.png`
   - 渲染单帧，先写入临时 `PPM`，随后调用 ffmpeg 转换成 PNG。未指定 `--output` 时默认写到 `output.png`。

3. **video**
   - `./build/graphic-study-demo --mode=video --output=demo.mp4 --duration=10 --fps=30`
   - 按给定时长与帧率生成 `frame_xxxx.ppm` 序列，全部渲染完成后使用 ffmpeg 合成 MP4，最后清理临时帧目录。

## 集成 ffmpeg

若不想依赖系统安装，可将可执行文件放置在仓库内：
```bash
mkdir -p external/ffmpeg/bin
cp <ffmpeg 可执行文件> external/ffmpeg/bin/ffmpeg
chmod +x external/ffmpeg/bin/ffmpeg
```
脚本与可执行程序都会优先从 `external/ffmpeg/bin` 中寻找 `ffmpeg`。
