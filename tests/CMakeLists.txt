cmake_minimum_required(VERSION 3.10)

set(GTEST_ROOT ${CMAKE_SOURCE_DIR}/external/googletest)
find_package(GTest REQUIRED CONFIG PATHS ${GTEST_ROOT}/lib/cmake/GTest)

add_executable(${PROJECT_NAME}_tests
    unit_tests.cpp
    transform_tests.cpp
    clip_tests.cpp
    viewport_tests.cpp
    vertex_normals_tests.cpp
    depth_stencil_tests.cpp
    material_lighting_tests.cpp
    ${CMAKE_SOURCE_DIR}/src/core/math/vector.cpp
    ${CMAKE_SOURCE_DIR}/src/core/math/matrix.cpp
    ${CMAKE_SOURCE_DIR}/src/core/types/color.cpp
    ${CMAKE_SOURCE_DIR}/src/core/types/vertex.cpp
    ${CMAKE_SOURCE_DIR}/src/core/types/material.cpp
    ${CMAKE_SOURCE_DIR}/src/core/types/texture.cpp
    ${CMAKE_SOURCE_DIR}/src/renderer/pipeline/render_target.cpp
    ${CMAKE_SOURCE_DIR}/src/core/platform/logger.cpp
)

target_include_directories(${PROJECT_NAME}_tests PRIVATE ${GTEST_ROOT}/include ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/external/SDL2/include)

# 链接 GTest 与 SDL2（Logger 依赖）
find_library(SDL2_LIBRARY SDL2 PATHS ${CMAKE_SOURCE_DIR}/external/SDL2/lib)
if(NOT SDL2_LIBRARY)
    message(FATAL_ERROR "Failed to locate SDL2 library at external/SDL2 for tests.")
endif()
target_link_libraries(${PROJECT_NAME}_tests PRIVATE GTest::gtest GTest::gtest_main ${SDL2_LIBRARY})

add_test(NAME all_tests COMMAND ${PROJECT_NAME}_tests)
