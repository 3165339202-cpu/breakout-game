cmake_minimum_required(VERSION 3.16)
project(AsyncBreakoutDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(async_breakout
    main.cpp
    Game.cpp
    Loader.cpp
)

target_include_directories(async_breakout PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(raylib QUIET)
if(raylib_FOUND)
    target_link_libraries(async_breakout PRIVATE raylib)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/../libraylib.a" AND EXISTS "${CMAKE_SOURCE_DIR}/../raylib.h")
    target_include_directories(async_breakout PRIVATE "${CMAKE_SOURCE_DIR}/..")
    target_link_libraries(async_breakout PRIVATE "${CMAKE_SOURCE_DIR}/../libraylib.a")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RAYLIB REQUIRED raylib)
    target_include_directories(async_breakout PRIVATE ${RAYLIB_INCLUDE_DIRS})
    target_link_directories(async_breakout PRIVATE ${RAYLIB_LIBRARY_DIRS})
    target_link_libraries(async_breakout PRIVATE ${RAYLIB_LIBRARIES})
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(async_breakout PRIVATE m pthread dl rt X11)
endif()
