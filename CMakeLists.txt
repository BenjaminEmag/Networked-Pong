cmake_minimum_required(VERSION 3.29)
project(network_pong)

set(CMAKE_CXX_STANDARD 26)

add_executable(network_pong main.cpp

 "Game.cpp" "Game.h")

add_subdirectory(raylib)

add_subdirectory(simplesock)
target_link_libraries(network_pong raylib simplesocklib)
target_link_libraries(network_pong wsock32 ws2_32)