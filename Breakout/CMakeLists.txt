cmake_minimum_required(VERSION 3.10)
project(BreakoutAsyncLoader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(breakout_async
    src/main.cpp
    src/Game.cpp
    src/Loader.cpp
)

target_include_directories(breakout_async PRIVATE ${CMAKE_SOURCE_DIR}/..)
target_link_directories(breakout_async PRIVATE ${CMAKE_SOURCE_DIR}/..)
target_link_libraries(breakout_async PRIVATE raylib m pthread dl rt X11)
