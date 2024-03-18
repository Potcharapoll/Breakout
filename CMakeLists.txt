cmake_minimum_required(VERSION 3.7)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(breakout)

add_subdirectory(lib/cglm)
add_subdirectory(lib/glfw)

include_directories(lib/glad/include)
include_directories(lib/stb)

add_executable(${PROJECT_NAME} 
    lib/glad/src/glad.c
    src/main.c)

target_compile_options(${PROJECT_NAME} PUBLIC -std=c2x -g3 -Wall -Wextra
    -Wstrict-aliasing -Wpedantic)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw cglm)
