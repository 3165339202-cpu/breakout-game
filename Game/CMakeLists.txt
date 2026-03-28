cmake_minimum_required(VERSION 3.10)
project(ShadowEcho)

set(CMAKE_CXX_STANDARD 17)

# 包含你自己的头文件和 raylib 头文件
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    /home/ricepaper/raylib/src   # raylib 头文件就在 src/ 下
)

# 添加所有源文件
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(shadow_echo ${SOURCES})

# 链接 raylib 静态库和系统库
target_link_libraries(shadow_echo
    /home/ricepaper/raylib/src/libraylib.a
    m dl pthread GL rt X11
)