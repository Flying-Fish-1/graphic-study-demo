#include "logger.h"
#include <cstdarg>
#include <cstring>
#include <iostream>

namespace Core {
namespace Platform {

// 静态成员初始化
std::unique_ptr<Logger> Logger::s_instance = nullptr;

Logger::Logger() : m_initialized(false) {
}

Logger::~Logger() {
    // 析构函数实现
}

Logger* Logger::getInstance() {
    if (!s_instance) {
        s_instance = std::unique_ptr<Logger>(new Logger());
    }
    return s_instance.get();
}

void Logger::initialize(LogLevel defaultLevel) {
    if (m_initialized) {
        return;
    }
    
    // 设置SDL日志输出函数
    SDL_LogSetOutputFunction([](void* userdata, int category, SDL_LogPriority priority, const char* message) {
        // 根据优先级添加不同的前缀
        const char* levelPrefix = "";
        switch (priority) {
            case SDL_LOG_PRIORITY_VERBOSE:
                levelPrefix = " [VERBOSE] ";
                break;
            case SDL_LOG_PRIORITY_DEBUG:
                levelPrefix = " [DEBUG] ";
                break;
            case SDL_LOG_PRIORITY_INFO:
                levelPrefix = " [INFO] ";
                break;
            case SDL_LOG_PRIORITY_WARN:
                levelPrefix = " [WARN] ";
                break;
            case SDL_LOG_PRIORITY_ERROR:
                levelPrefix = " [ERROR] ";
                break;
            case SDL_LOG_PRIORITY_CRITICAL:
                levelPrefix = " [CRITICAL] ";
                break;
            default:
                levelPrefix = " [LOG] ";
                break;
        }
        
        // 根据类别添加不同的前缀
        const char* categoryPrefix = "";
        switch (category) {
            case SDL_LOG_CATEGORY_APPLICATION:
                categoryPrefix = "[APP] ";
                break;
            case SDL_LOG_CATEGORY_ERROR:
                categoryPrefix = "[ERR] ";
                break;
            case SDL_LOG_CATEGORY_SYSTEM:
                categoryPrefix = "[SYS] ";
                break;
            case SDL_LOG_CATEGORY_AUDIO:
                categoryPrefix = "[AUD] ";
                break;
            case SDL_LOG_CATEGORY_VIDEO:
                categoryPrefix = "[VID] ";
                break;
            case SDL_LOG_CATEGORY_RENDER:
                categoryPrefix = "[RND] ";
                break;
            case SDL_LOG_CATEGORY_INPUT:
                categoryPrefix = "[INP] ";
                break;
            case SDL_LOG_CATEGORY_TEST:
                categoryPrefix = "[TST] ";
                break;
            default:
                if (category >= SDL_LOG_CATEGORY_CUSTOM) {
                    switch (category - SDL_LOG_CATEGORY_CUSTOM) {
                        case 0: categoryPrefix = "[GFX] "; break; // GRAPHICS
                        case 1: categoryPrefix = "[SCN] "; break; // SCENE
                        case 2: categoryPrefix = "[PIP] "; break; // PIPELINE
                        default: categoryPrefix = "[CST] "; break;
                    }
                } else {
                    categoryPrefix = "[UNK] ";
                }
                break;
        }
        
        // 输出到控制台
        std::cout << levelPrefix << categoryPrefix << message << std::endl;
    }, nullptr);
    
    // 设置默认日志级别
    setAllLogLevel(defaultLevel);
    
    m_initialized = true;
    
    // 记录初始化成功
    info(LogCategory::SYSTEM, "日志系统初始化完成");
}

void Logger::setLogLevel(LogCategory category, LogLevel level) {
    if (!m_initialized) {
        initialize();
    }
    
    SDL_LogSetPriority(static_cast<int>(category), static_cast<SDL_LogPriority>(level));
}

void Logger::setAllLogLevel(LogLevel level) {
    if (!m_initialized) {
        initialize();
    }
    
    SDL_LogSetAllPriority(static_cast<SDL_LogPriority>(level));
}

void Logger::verbose(LogCategory category, const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    SDL_LogVerbose(static_cast<int>(category), "%s", message.c_str());
}

void Logger::debug(LogCategory category, const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    SDL_LogDebug(static_cast<int>(category), "%s", message.c_str());
}

void Logger::info(LogCategory category, const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    SDL_LogInfo(static_cast<int>(category), "%s", message.c_str());
}

void Logger::warn(LogCategory category, const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    SDL_LogWarn(static_cast<int>(category), "%s", message.c_str());
}

void Logger::error(LogCategory category, const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    SDL_LogError(static_cast<int>(category), "%s", message.c_str());
}

void Logger::critical(LogCategory category, const std::string& message) {
    if (!m_initialized) {
        initialize();
    }
    
    SDL_LogCritical(static_cast<int>(category), "%s", message.c_str());
}

std::string Logger::format(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // 计算需要的缓冲区大小
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);
    
    if (size <= 0) {
        va_end(args);
        return "";
    }
    
    // 分配缓冲区并格式化字符串
    std::string result(size + 1, '\0');
    vsnprintf(&result[0], size + 1, format, args);
    va_end(args);
    
    // 移除末尾的null字符
    result.resize(size);
    return result;
}

} // namespace Platform
} // namespace Core
