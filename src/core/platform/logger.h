#ifndef CORE_PLATFORM_LOGGER_H
#define CORE_PLATFORM_LOGGER_H

#include <string>
#include <memory>
#include <SDL2/SDL_log.h>

namespace Core {
namespace Platform {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    VERBOSE = SDL_LOG_PRIORITY_VERBOSE,
    DEBUG = SDL_LOG_PRIORITY_DEBUG,
    INFO = SDL_LOG_PRIORITY_INFO,
    WARN = SDL_LOG_PRIORITY_WARN,
    ERROR = SDL_LOG_PRIORITY_ERROR,
    CRITICAL = SDL_LOG_PRIORITY_CRITICAL
};

/**
 * @brief 日志类别枚举
 */
enum class LogCategory {
    APPLICATION = SDL_LOG_CATEGORY_APPLICATION,
    ERROR = SDL_LOG_CATEGORY_ERROR,
    SYSTEM = SDL_LOG_CATEGORY_SYSTEM,
    AUDIO = SDL_LOG_CATEGORY_AUDIO,
    VIDEO = SDL_LOG_CATEGORY_VIDEO,
    RENDER = SDL_LOG_CATEGORY_RENDER,
    INPUT = SDL_LOG_CATEGORY_INPUT,
    TEST = SDL_LOG_CATEGORY_TEST,
    GRAPHICS = SDL_LOG_CATEGORY_CUSTOM,  // 自定义图形学类别
    SCENE = SDL_LOG_CATEGORY_CUSTOM + 1, // 自定义场景类别
    PIPELINE = SDL_LOG_CATEGORY_CUSTOM + 2 // 自定义渲染管线类别
};

/**
 * @brief 日志系统类
 * 
 * 基于SDL2日志系统的封装，提供统一的日志接口
 */
class Logger {
private:
    static std::unique_ptr<Logger> s_instance;
    bool m_initialized;
    
    Logger();
    
    // 禁用拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

public:
    ~Logger();

public:
    /**
     * @brief 获取Logger单例实例
     * @return Logger实例的引用
     */
    static Logger* getInstance();
    
    /**
     * @brief 初始化日志系统
     * @param defaultLevel 默认日志级别
     */
    void initialize(LogLevel defaultLevel = LogLevel::INFO);
    
    /**
     * @brief 设置特定类别的日志级别
     * @param category 日志类别
     * @param level 日志级别
     */
    void setLogLevel(LogCategory category, LogLevel level);
    
    /**
     * @brief 设置所有类别的日志级别
     * @param level 日志级别
     */
    void setAllLogLevel(LogLevel level);
    
    /**
     * @brief 记录详细日志
     * @param category 日志类别
     * @param message 日志消息
     */
    void verbose(LogCategory category, const std::string& message);
    
    /**
     * @brief 记录调试日志
     * @param category 日志类别
     * @param message 日志消息
     */
    void debug(LogCategory category, const std::string& message);
    
    /**
     * @brief 记录信息日志
     * @param category 日志类别
     * @param message 日志消息
     */
    void info(LogCategory category, const std::string& message);
    
    /**
     * @brief 记录警告日志
     * @param category 日志类别
     * @param message 日志消息
     */
    void warn(LogCategory category, const std::string& message);
    
    /**
     * @brief 记录错误日志
     * @param category 日志类别
     * @param message 日志消息
     */
    void error(LogCategory category, const std::string& message);
    
    /**
     * @brief 记录严重错误日志
     * @param category 日志类别
     * @param message 日志消息
     */
    void critical(LogCategory category, const std::string& message);
    
    // 便捷方法 - 使用默认的APPLICATION类别
    void verbose(const std::string& message) { verbose(LogCategory::APPLICATION, message); }
    void debug(const std::string& message) { debug(LogCategory::APPLICATION, message); }
    void info(const std::string& message) { info(LogCategory::APPLICATION, message); }
    void warn(const std::string& message) { warn(LogCategory::APPLICATION, message); }
    void error(const std::string& message) { error(LogCategory::APPLICATION, message); }
    void critical(const std::string& message) { critical(LogCategory::APPLICATION, message); }
    
    /**
     * @brief 格式化日志消息
     * @param format 格式化字符串
     * @param ... 可变参数
     * @return 格式化后的字符串
     */
    static std::string format(const char* format, ...);
    
    /**
     * @brief 检查是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const { return m_initialized; }
};

} // namespace Platform
} // namespace Core

#endif // CORE_PLATFORM_LOGGER_H
