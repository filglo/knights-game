cmake_minimum_required(VERSION 3.0.0)
project(Knights VERSION 0.1.0)

set(CMAKE_CONFIGURATION_TYPES Debug Release)


include_directories(${CMAKE_SOURCE_DIR}/src)

ADD_LIBRARY(GameLib src/Game.cpp src/GameConstants.cpp src/GameMap.cpp src/GameObject.cpp
            src/commands/Build.cpp src/commands/Attack.cpp src/commands/Move.cpp src/commands/Commands.cpp)
add_executable(Knights src/mainPlayer.cpp src/Player.cpp)
add_executable(KnightsMediator src/mainMediator.cpp src/Mediator.cpp)
add_executable(Tests tests/tests.cpp)

target_compile_options(GameLib PUBLIC -Wall --std=c++17)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

target_link_libraries(Knights GameLib)
target_link_libraries(KnightsMediator GameLib)
target_link_libraries(Tests GameLib)
