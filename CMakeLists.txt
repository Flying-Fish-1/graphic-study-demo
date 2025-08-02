cmake_minimum_required(VERSION 3.10)

# 仅在macOS上设置SDK路径
if(APPLE)
    set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk)
endif()

project(graphic-study-demo)

# 设置C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置SDL2路径
set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2")

# 包含目录
include_directories(${SDL2_PATH}/include)

# 查找SDL2库
find_library(SDL2_LIBRARY SDL2 PATHS ${SDL2_PATH}/lib)

# 添加可执行文件
add_executable(${PROJECT_NAME} src/main.cpp)

# 链接SDL2库
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})

# 设置运行时搜索路径
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${SDL2_PATH}/lib")