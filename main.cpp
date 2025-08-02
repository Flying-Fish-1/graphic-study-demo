#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

// ColorRGBA结构体定义
struct ColorRGBA {
    float r, g, b, a;
    
    ColorRGBA(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}
    
    ColorRGBA operator*(float t) const { 
        return ColorRGBA(r*t, g*t, b*t, a*t); 
    }
    
    ColorRGBA operator+(const ColorRGBA& other) const {
        return ColorRGBA(r + other.r, g + other.g, b + other.b, a + other.a);
    }

    // 从Uint32转换为ColorRGBA
    static ColorRGBA fromUint32(Uint32 color) {
        return ColorRGBA(
            ((color >> 24) & 0xFF) / 255.0f,
            ((color >> 16) & 0xFF) / 255.0f,
            ((color >> 8) & 0xFF) / 255.0f,
            (color & 0xFF) / 255.0f
        );
    }

    // 转换为Uint32
    Uint32 toUint32() const {
        return ((Uint8)(r * 255.0f) << 24) |
               ((Uint8)(g * 255.0f) << 16) |
               ((Uint8)(b * 255.0f) << 8) |
               ((Uint8)(a * 255.0f));
    }
};

// const variables
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const char* WINDOW_TITLE = "SDL Graphics Demo";

// 抗锯齿方案枚举
enum AntiAliasingMethod {
    NONE,           // 无抗锯齿
    GAUSSIAN_BLUR,  // 高斯模糊抗锯齿
    SSAA,           // 超采样抗锯齿
    SSAA_GAUSSIAN,  // 超采样+高斯模糊组合
};

// 当前使用的抗锯齿方案
AntiAliasingMethod g_currentAAMethod = SSAA;  // 默认使用超采样抗锯齿

// 超采样倍数
int g_ssaaScale = 2;  // 默认2x超采样
const int MIN_SSAA_SCALE = 1;
const int MAX_SSAA_SCALE = 8;  // 最大8x超采样

// global variables
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
SDL_Texture* g_frameBuffer = nullptr;
Uint32* g_pixelBuffer = nullptr;
int g_pitch = 0; // number of bytes per row

// function prototypes
bool initSDL();
bool createWindow();
bool createRenderer();
bool createFrameBuffer();
void updateScreen();
void clearScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void cleanup();

void setPixel(int x, int y, ColorRGBA color);
Uint32 getPixel(int x, int y);
void draw(float time);

void drawLine(int x0, int y0, int x1, int y1, ColorRGBA color0, ColorRGBA color1);
void drawCircle(int xc, int yc, int radius, ColorRGBA color);
void plotCirclePoints(int xc, int yc, int x, int y, ColorRGBA color);

void applyAntiAliasing(Uint32* buffer, int width, int height);
void applyGaussianBlur(Uint32* buffer, int width, int height);
void applySuperSampling(int x0, int y0, int x1, int y1, int x2, int y2, 
                        ColorRGBA c0, ColorRGBA c1, ColorRGBA c2, 
                        Uint32* buffer, int width, int height);
bool isTriangleEdge(int x0, int y0, int x1, int y1, int x2, int y2, int minX, int minY, int maxX, int maxY);
void switchAntiAliasingMethod();
const char* getAntiAliasingMethodName();
void increaseSsaaScale();
void decreaseSsaaScale();
std::string getSsaaScaleInfo();

bool checkPointInTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int px, int py);
ColorRGBA getPointColorInTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int px, int py, 
                                ColorRGBA c0, ColorRGBA c1, ColorRGBA c2);

// 光栅化
void rasterization(int x0, int y0, int x1, int y1, int x2, int y2, ColorRGBA c0, ColorRGBA c1, ColorRGBA c2) {
    // 计算包围盒
    int minX = std::min(std::min(x0, x1), x2);
    int minY = std::min(std::min(y0, y1), y2);
    int maxX = std::max(std::max(x0, x1), x2);
    int maxY = std::max(std::max(y0, y1), y2);
    
    // 限制在屏幕范围内
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, SCREEN_WIDTH - 1);
    maxY = std::min(maxY, SCREEN_HEIGHT - 1);
    
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float sx = x + 0.5f;
            float sy = y + 0.5f;
            
            if (checkPointInTriangle(x0, y0, x1, y1, x2, y2, sx, sy)) {
                ColorRGBA color = getPointColorInTriangle(x0, y0, x1, y1, x2, y2, sx, sy, c0, c1, c2);
                setPixel(x, y, color);
            }
        }
    }
}

// initialize SDL
bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL 初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

// create window
bool createWindow() {
    g_window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!g_window) {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_RaiseWindow(g_window);
    return true;
}

// create renderer
bool createRenderer() {
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer) {
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

// create frame buffer
bool createFrameBuffer() {
    g_frameBuffer = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT
    );
    
    if (!g_frameBuffer) {
        std::cerr << "无法创建帧缓冲区: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // create pixel buffer
    g_pixelBuffer = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT];
    if (!g_pixelBuffer) {
        std::cerr << "无法分配像素缓冲区" << std::endl;
        return false;
    }
    
    // clear pixel buffer
    memset(g_pixelBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Uint32));
    g_pitch = SCREEN_WIDTH * sizeof(Uint32);
    
    return true;
}


void updateScreen() {
    SDL_UpdateTexture(g_frameBuffer, NULL, g_pixelBuffer, g_pitch);
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_frameBuffer, NULL, NULL);
    SDL_RenderPresent(g_renderer);
}

// clear screen
void clearScreen(Uint8 r = 0, Uint8 g = 0, Uint8 b = 0, Uint8 a = 255) {
    Uint32 color = (r << 24) | (g << 16) | (b << 8) | a;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        g_pixelBuffer[i] = color;
    }
}

// close SDL
void cleanup() {
    // release pixel buffer
    if (g_pixelBuffer) {
        delete[] g_pixelBuffer;
        g_pixelBuffer = nullptr;
    }
    
    // release frame buffer
    if (g_frameBuffer) {
        SDL_DestroyTexture(g_frameBuffer);
        g_frameBuffer = nullptr;
    }
    
    // release renderer
    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = nullptr;
    }
    
    // release window
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }
    
    // quit SDL
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    // initialize SDL
    if (!initSDL() || !createWindow() || !createRenderer() || !createFrameBuffer()) {
        cleanup();
        return 1;
    }

    std::cout << "窗口已创建。按ESC键或关闭窗口退出。" << std::endl;
    std::cout << "按A键切换抗锯齿方案。" << std::endl;
    std::cout << "按上箭头键增加超采样倍数，按下箭头键减少超采样倍数。" << std::endl;
    std::cout << "当前抗锯齿方案: " << getAntiAliasingMethodName() << std::endl;
    std::cout << "当前超采样倍数: " << g_ssaaScale << "x" << std::endl;

    // 更新窗口标题，显示当前抗锯齿方案和超采样倍数
    std::string windowTitle = std::string(WINDOW_TITLE) + " - " + getAntiAliasingMethodName() + " - " + getSsaaScaleInfo();
    SDL_SetWindowTitle(g_window, windowTitle.c_str());

    // main loop flag
    bool quit = false;
    SDL_Event e;
    
    // frame count
    int frameCount = 0;
    
    // main loop
    while (!quit) {
        // handle events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (e.key.keysym.sym == SDLK_a) {
                    // 切换抗锯齿方案
                    switchAntiAliasingMethod();
                    
                    // 更新窗口标题
                    std::string windowTitle = std::string(WINDOW_TITLE) + " - " + getAntiAliasingMethodName() + " - " + getSsaaScaleInfo();
                    SDL_SetWindowTitle(g_window, windowTitle.c_str());
                } else if (e.key.keysym.sym == SDLK_UP) {
                    // 增加超采样倍数
                    increaseSsaaScale();
                    
                    // 更新窗口标题
                    std::string windowTitle = std::string(WINDOW_TITLE) + " - " + getAntiAliasingMethodName() + " - " + getSsaaScaleInfo();
                    SDL_SetWindowTitle(g_window, windowTitle.c_str());
                } else if (e.key.keysym.sym == SDLK_DOWN) {
                    // 减少超采样倍数
                    decreaseSsaaScale();
                    
                    // 更新窗口标题
                    std::string windowTitle = std::string(WINDOW_TITLE) + " - " + getAntiAliasingMethodName() + " - " + getSsaaScaleInfo();
                    SDL_SetWindowTitle(g_window, windowTitle.c_str());
                }
                // output key name
                std::cout << "按下了键: " << SDL_GetKeyName(e.key.keysym.sym) << std::endl;
            }
        }
        // update time per frame
        float t = frameCount * 0.01f;

        // clear screen
        clearScreen(0, 0, 0, 255); // set background color to black

        // draw according to time
        draw(t);

        updateScreen();
        
        SDL_Delay(16); // 60 FPS
        
        frameCount++;
    }

    // clean up
    cleanup();

    return 0;
}

// set pixel color
void setPixel(int x, int y, ColorRGBA color) {
    // check if pixel is out of screen
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }
    int index = y * (g_pitch / 4) + x;
    g_pixelBuffer[index] = ((Uint8)(color.r * 255.0f) << 24) | 
                          ((Uint8)(color.g * 255.0f) << 16) | 
                          ((Uint8)(color.b * 255.0f) << 8) | 
                          (Uint8)(color.a * 255.0f);
}

// 读取像素
Uint32 getPixel(int x, int y) {
    // 检查像素是否超出屏幕范围
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return 0; // 返回默认值（黑色，透明）
    }
    int index = y * (g_pitch / 4) + x;
    return g_pixelBuffer[index];
}

// Breseham's line drawing algorithm
void drawLine(int x0, int y0, int x1, int y1, ColorRGBA color0, ColorRGBA color1) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    int x = x0;
    int y = y0;
    
    while (true) {
        float t = (float)(x - x0) / (float)(x1 - x0);
        ColorRGBA color = color0 * (1.0f - t) + color1 * t;
        setPixel(x, y, color);
        
        if (x == x1 && y == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

// Bresenham's circle drawing algorithm
void drawCircle(int xc, int yc, int r, ColorRGBA color) {
    int x = 0;
    int y = r;
    int p = 1 - r;  // initial decision parameter
    
    // initial plot
    plotCirclePoints(xc, yc, x, y, color);
    
    while (x < y) {
        x++;
        
        // update decision parameter
        if (p < 0) {
            p += 2 * x + 1;
        } 
        else {
            y--;
            p += 2 * (x - y) + 1;
        }
        
        // plot eight points
        plotCirclePoints(xc, yc, x, y, color);
    }
}

// plot circle points using symmetry
void plotCirclePoints(int xc, int yc, int x, int y, ColorRGBA color) {
    setPixel(xc + x, yc + y, color);
    setPixel(xc - x, yc + y, color);
    setPixel(xc + x, yc - y, color);
    setPixel(xc - x, yc - y, color);
    setPixel(xc + y, yc + x, color);
    setPixel(xc - y, yc + x, color);
    setPixel(xc + y, yc - x, color);
    setPixel(xc - y, yc - x, color);
}

// 检查点是否在三角形内
bool checkPointInTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int px, int py) {
    // 计算向量
    int v0x = x2 - x0, v0y = y2 - y0;
    int v1x = x1 - x0, v1y = y1 - y0;
    int v2x = px - x0, v2y = py - y0;
    
    // 计算点积
    float dot00 = v0x * v0x + v0y * v0y;
    float dot01 = v0x * v1x + v0y * v1y;
    float dot02 = v0x * v2x + v0y * v2y;
    float dot11 = v1x * v1x + v1y * v1y;
    float dot12 = v1x * v2x + v1y * v2y;
    
    // 计算重心坐标
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    // 检查是否在三角形内
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

ColorRGBA getPointColorInTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int px, int py, 
                                ColorRGBA c0, ColorRGBA c1, ColorRGBA c2) {
    // 使用向量方法计算重心坐标
    int v0x = x2 - x0, v0y = y2 - y0;
    int v1x = x1 - x0, v1y = y1 - y0;
    int v2x = px - x0, v2y = py - y0;
    
    float dot00 = v0x * v0x + v0y * v0y;
    float dot01 = v0x * v1x + v0y * v1y;
    float dot02 = v0x * v2x + v0y * v2y;
    float dot11 = v1x * v1x + v1y * v1y;
    float dot12 = v1x * v2x + v1y * v2y;
    
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    float w = 1.0f - u - v;
    
    // 使用重心坐标插值颜色
    return c0 * w + c1 * v + c2 * u;
}

// 实现抗锯齿算法
// 高斯模糊抗锯齿算法
void applyGaussianBlur(Uint32* buffer, int width, int height) {
    // 创建临时缓冲区来存储处理后的像素
    Uint32* tempBuffer = new Uint32[width * height];
    memcpy(tempBuffer, buffer, width * height * sizeof(Uint32));
    
    // 定义卷积核 (3x3 高斯模糊)
    const float kernel[3][3] = {
        {1.0f/16, 2.0f/16, 1.0f/16},
        {2.0f/16, 4.0f/16, 2.0f/16},
        {1.0f/16, 2.0f/16, 1.0f/16}
    };
    
    // 应用卷积
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            float r = 0, g = 0, b = 0, a = 0;
            
            // 应用卷积核
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = x + kx;
                    int py = y + ky;
                    
                    // 获取像素颜色
                    Uint32 pixel = buffer[py * width + px];
                    ColorRGBA color = ColorRGBA::fromUint32(pixel);
                    
                    // 应用权重
                    float weight = kernel[ky+1][kx+1];
                    r += color.r * weight;
                    g += color.g * weight;
                    b += color.b * weight;
                    a += color.a * weight;
                }
            }
            
            // 设置新的像素颜色
            ColorRGBA newColor(r, g, b, a);
            tempBuffer[y * width + x] = newColor.toUint32();
        }
    }
    
    // 将处理后的像素复制回原始缓冲区
    memcpy(buffer, tempBuffer, width * height * sizeof(Uint32));
    
    // 释放临时缓冲区
    delete[] tempBuffer;
}

// 超采样抗锯齿算法 (SSAA - SuperSampling Anti-Aliasing)
void applySuperSampling(int x0, int y0, int x1, int y1, int x2, int y2, 
                       ColorRGBA c0, ColorRGBA c1, ColorRGBA c2, 
                       Uint32* buffer, int width, int height) {
    // 创建高分辨率缓冲区
    int ssaaWidth = width * g_ssaaScale;
    int ssaaHeight = height * g_ssaaScale;
    Uint32* ssaaBuffer = new Uint32[ssaaWidth * ssaaHeight];
    
    // 清空高分辨率缓冲区
    memset(ssaaBuffer, 0, ssaaWidth * ssaaHeight * sizeof(Uint32));
    
    // 在高分辨率下进行光栅化
    // 计算包围盒
    int minX = std::min(std::min(x0, x1), x2) * g_ssaaScale;
    int minY = std::min(std::min(y0, y1), y2) * g_ssaaScale;
    int maxX = std::max(std::max(x0, x1), x2) * g_ssaaScale;
    int maxY = std::max(std::max(y0, y1), y2) * g_ssaaScale;
    
    // 限制在高分辨率屏幕范围内
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, ssaaWidth - 1);
    maxY = std::min(maxY, ssaaHeight - 1);
    
    // 高分辨率下的顶点坐标
    int ssaaX0 = x0 * g_ssaaScale;
    int ssaaY0 = y0 * g_ssaaScale;
    int ssaaX1 = x1 * g_ssaaScale;
    int ssaaY1 = y1 * g_ssaaScale;
    int ssaaX2 = x2 * g_ssaaScale;
    int ssaaY2 = y2 * g_ssaaScale;
    
    // 在高分辨率下进行光栅化
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float sx = x + 0.5f;
            float sy = y + 0.5f;
            
            if (checkPointInTriangle(ssaaX0, ssaaY0, ssaaX1, ssaaY1, ssaaX2, ssaaY2, sx, sy)) {
                ColorRGBA color = getPointColorInTriangle(ssaaX0, ssaaY0, ssaaX1, ssaaY1, ssaaX2, ssaaY2, 
                                                        sx, sy, c0, c1, c2);
                // 设置高分辨率缓冲区的像素
                int index = y * ssaaWidth + x;
                ssaaBuffer[index] = color.toUint32();
            }
        }
    }
    
    // 将高分辨率缓冲区下采样到原始分辨率
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0, a = 0;
            int count = 0;
            
            // 对每个原始像素，采样g_ssaaScale*g_ssaaScale个高分辨率像素
            for (int dy = 0; dy < g_ssaaScale; dy++) {
                for (int dx = 0; dx < g_ssaaScale; dx++) {
                    int ssaaX = x * g_ssaaScale + dx;
                    int ssaaY = y * g_ssaaScale + dy;
                    
                    if (ssaaX < ssaaWidth && ssaaY < ssaaHeight) {
                        int ssaaIndex = ssaaY * ssaaWidth + ssaaX;
                        ColorRGBA color = ColorRGBA::fromUint32(ssaaBuffer[ssaaIndex]);
                        
                        r += color.r;
                        g += color.g;
                        b += color.b;
                        a += color.a;
                        count++;
                    }
                }
            }
            
            // 计算平均值
            if (count > 0) {
                r /= count;
                g /= count;
                b /= count;
                a /= count;
                
                // 设置原始分辨率缓冲区的像素
                int index = y * width + x;
                buffer[index] = ColorRGBA(r, g, b, a).toUint32();
            }
        }
    }
    
    // 释放高分辨率缓冲区
    delete[] ssaaBuffer;
}

// 应用选定的抗锯齿方法
void applyAntiAliasing(Uint32* buffer, int width, int height) {
    // 根据当前选择的抗锯齿方法应用相应的算法
    switch (g_currentAAMethod) {
        case GAUSSIAN_BLUR:
            applyGaussianBlur(buffer, width, height);
            break;
        case SSAA:
            // SSAA在光栅化阶段已经应用，这里不需要额外处理
            break;
        case SSAA_GAUSSIAN:
            // 先应用高斯模糊
            applyGaussianBlur(buffer, width, height);
            break;
        case NONE:
        default:
            // 不应用任何抗锯齿
            break;
    }
}

// 切换抗锯齿方案
void switchAntiAliasingMethod() {
    // 循环切换到下一个抗锯齿方案
    g_currentAAMethod = static_cast<AntiAliasingMethod>((g_currentAAMethod + 1) % 4);
    
    // 输出当前使用的抗锯齿方案
    const char* methodNames[] = {"无抗锯齿", "高斯模糊抗锯齿", "超采样抗锯齿", "超采样+高斯模糊组合"};
    std::cout << "当前抗锯齿方案: " << methodNames[g_currentAAMethod] << std::endl;
}

// 获取抗锯齿方案名称
const char* getAntiAliasingMethodName() {
    const char* methodNames[] = {"无抗锯齿", "高斯模糊抗锯齿", "超采样抗锯齿", "超采样+高斯模糊组合"};
    return methodNames[g_currentAAMethod];
}

// 增加超采样倍数
void increaseSsaaScale() {
    if (g_ssaaScale < MAX_SSAA_SCALE) {
        g_ssaaScale++;
        std::cout << "超采样倍数增加到: " << g_ssaaScale << "x" << std::endl;
    } else {
        std::cout << "已达到最大超采样倍数: " << g_ssaaScale << "x" << std::endl;
    }
}

// 减少超采样倍数
void decreaseSsaaScale() {
    if (g_ssaaScale > MIN_SSAA_SCALE) {
        g_ssaaScale--;
        std::cout << "超采样倍数减少到: " << g_ssaaScale << "x" << std::endl;
    } else {
        std::cout << "已达到最小超采样倍数: " << g_ssaaScale << "x" << std::endl;
    }
}

// 获取超采样倍数信息
std::string getSsaaScaleInfo() {
    return std::to_string(g_ssaaScale) + "x超采样";
}

// draw函数
void draw(float time) {
    ColorRGBA red(1.0f, 0.0f, 0.0f, 1.0f);
    ColorRGBA green(0.0f, 1.0f, 0.0f, 1.0f);
    ColorRGBA blue(0.0f, 0.0f, 1.0f, 1.0f);

    int x0 = 50, y0 = 100;
    int x1 = 170, y1 = 350;
    int x2 = 400, y2 = 300;

    // 根据当前抗锯齿方案选择渲染方式
    if (g_currentAAMethod == SSAA || g_currentAAMethod == SSAA_GAUSSIAN) {
        // 使用超采样方法直接渲染到缓冲区
        applySuperSampling(x0, y0, x1, y1, x2, y2, red, green, blue, g_pixelBuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
    } 
    else {
        // 使用普通光栅化
        rasterization(x0, y0, x1, y1, x2, y2, red, green, blue);
    }
    
    // 应用其他抗锯齿处理（如高斯模糊）
    applyAntiAliasing(g_pixelBuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
}
