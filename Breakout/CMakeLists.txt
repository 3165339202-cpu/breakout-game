cmake_minimum_required(VERSION 3.10)
project(Breakout)

set(CMAKE_CXX_STANDARD 17)

set(RAYLIB_ROOT ${CMAKE_SOURCE_DIR}/..)

include_directories(${RAYLIB_ROOT})
link_directories(${RAYLIB_ROOT})

add_executable(breakout_week2 
    src/main.cpp 
    src/Ball.cpp 
    src/Paddle.cpp 
    src/Brick.cpp
    src/Game.cpp
    src/ExtendPaddleEffect.cpp
    src/MultiBallEffect.cpp
    src/SlowBallEffect.cpp
    src/PowerUpFactory.cpp  
    src/PowerUp.cpp
    src/NetworkSession.cpp
)

target_link_libraries(breakout_week2 raylib m pthread dl rt X11)


add_custom_target(copy_breakout_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/src/config.json
            ${CMAKE_BINARY_DIR}/config.json
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/fonts
            ${CMAKE_BINARY_DIR}/fonts
)
add_dependencies(breakout_week2 copy_breakout_assets)
