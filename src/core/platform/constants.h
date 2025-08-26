#ifndef CONSTANTS_H
#define CONSTANTS_H

// 屏幕常量
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const char* const WINDOW_TITLE = "SDL Graphics Demo";

// 抗锯齿方案枚举
enum AntiAliasingMethod {
    NONE,           // 无抗锯齿
    GAUSSIAN_BLUR,  // 高斯模糊抗锯齿
    SSAA,           // 超采样抗锯齿
    SSAA_GAUSSIAN,  // 超采样+高斯模糊组合
};

// 超采样倍数常量
const int MIN_SSAA_SCALE = 1;
const int MAX_SSAA_SCALE = 8;  // 最大8x超采样

#endif // CONSTANTS_H
