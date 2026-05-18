cmake_minimum_required(VERSION 3.10)
project(Breakout)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/..)
link_directories(${CMAKE_SOURCE_DIR}/..)

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

add_custom_command(TARGET breakout_week2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/levels
            $<TARGET_FILE_DIR:breakout_week2>/levels
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/src/config.json
            $<TARGET_FILE_DIR:breakout_week2>/config.json
)
