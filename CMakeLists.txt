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

# 添加源文件 - 新架构版本
set(SOURCES
    # 主程序
    src/main.cpp
    
    # 新架构核心模块
    src/core/math/vector.cpp
    src/core/math/matrix.cpp
    src/core/types/color.cpp
    src/core/platform/sdl_wrapper.cpp
    
    # 新增的核心类型
    src/core/types/vertex.cpp
    src/core/types/triangle.cpp
    src/core/types/material.cpp
    src/core/types/texture.cpp
    
    # 重构后的模块
    src/graphics/graphics.cpp
    src/renderer/effects/antialiasing.cpp
    src/demos/transform_demo.cpp
    src/demos/scene_3d_demo.cpp
    
    # 新增的渲染管线
    src/renderer/pipeline/depth_buffer.cpp
    src/renderer/pipeline/rasterizer_3d.cpp
    
    # 新增的光照系统
    src/renderer/lighting/light.cpp
    
    # 新增的场景管理
    src/scene/mesh.cpp
    src/scene/camera.cpp
    src/scene/scene.cpp
)

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接SDL2库
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})

# 设置运行时搜索路径
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${SDL2_PATH}/lib")