cmake_minimum_required(VERSION 3.10)

project(graphic-study-demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_SDL_PREVIEW "Enable SDL2 preview window" ON)

set(SOURCES
    src/main.cpp
    src/core/math/vector.cpp
    src/core/math/matrix.cpp
    src/core/types/color.cpp
    src/core/types/vertex.cpp
    src/core/types/triangle.cpp
    src/core/types/material.cpp
    src/core/types/texture.cpp
    src/renderer/pipeline/render_target.cpp
    src/renderer/pipeline/geometry_stage.cpp
    src/renderer/pipeline/geometry_processor.cpp
    src/renderer/pipeline/render_queue.cpp
    src/renderer/pipeline/shading_pipeline.cpp
    src/renderer/pipeline/triangle_rasterizer.cpp
    src/renderer/pipeline/software_renderer.cpp
    src/renderer/lighting/light.cpp
    src/renderer/effects/ssaa.cpp
    src/util/ffmpeg_utils.cpp
    src/scene/mesh.cpp
    src/scene/camera.cpp
    src/scene/scene.cpp
    src/core/platform/logger.cpp
)

# SDL2 始终可用（用于 Logger 以及可选的预览窗口）
set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2")
include_directories(${SDL2_PATH}/include)
find_library(SDL2_LIBRARY SDL2 PATHS ${SDL2_PATH}/lib)
if(NOT SDL2_LIBRARY)
    message(FATAL_ERROR "Failed to locate SDL2 library at external/SDL2.")
endif()

if(ENABLE_SDL_PREVIEW)
    list(APPEND SOURCES src/renderer/preview/sdl_preview.cpp)
else()
    list(APPEND SOURCES src/renderer/preview/sdl_preview_stub.cpp)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src ${SDL2_PATH}/include)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})

if(ENABLE_SDL_PREVIEW)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_SDL_PREVIEW)
endif()

option(BUILD_TESTING "Enable unit tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
