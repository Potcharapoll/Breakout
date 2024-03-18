cmake_minimum_required(VERSION 3.8.7)
project(glad)

set(DEFAULT_LINKAGE_TYPE SHARED)
if (WIN32) 
    set(DEFAULT_LINKAGE_TYPE STATIC)
endif(WIN32)

add_library(${PROJECT_NAME} ${DEFAULT_LINKAGE_TYPE} include/glad/glad.h src/glad.c)
target_include_directories(${PROJECT_NAME} PUBLIC include)
