cmake_minimum_required(VERSION 3.7)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(breakout)

add_subdirectory(lib/cglm)
add_subdirectory(lib/glfw)

include_directories(lib/glad/include)
include_directories(lib/stb)

add_executable(${PROJECT_NAME} 
    lib/glad/src/glad.c
    src/main.c
    src/gfx/game.c
    src/gfx/renderer.c
    src/gfx/text.c
    src/gfx/texture.c
    src/gfx/vao.c
    src/gfx/vbo.c
    src/gfx/shader.c
    src/entity/entity.c
    src/util/linked_list.c
    src/util/sound.c
    )

target_compile_options(${PROJECT_NAME} PRIVATE -std=c2x -O2 -Wall -Wextra -Wstrict-aliasing -Wpedantic -Wno-unused-result)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw cglm -lopenal -laudio)
