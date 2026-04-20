cmake_minimum_required(VERSION 3.10)
project(Breakout)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/../build/include)
link_directories(${CMAKE_SOURCE_DIR}/../build)

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
