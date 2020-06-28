cmake_minimum_required(VERSION 3.16)
project(Renderer)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(Renderer
        main.cpp
        rasterize.hh mesh.hh)

INCLUDE_DIRECTORIES(C:\\Users\\letba\\scoop\\apps\\sdl2\\current\\include\\SDL2)
TARGET_LINK_LIBRARIES(Renderer C:\\Users\\letba\\scoop\\apps\\sdl2\\current\\lib\\SDL2.dll)