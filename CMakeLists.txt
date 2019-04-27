cmake_minimum_required(VERSION 3.0)
project(learnopengl)

# includes
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/glad/include)

# sources
file(GLOB_RECURSE files_stb "stb/*.?pp")
source_group("stb" FILES ${files_stb})
file(GLOB_RECURSE files_glm "glm/*.?pp")
source_group("glm" FILES ${files_glm})
file(GLOB_RECURSE files_render "render/*.?pp")
source_group("render" FILES ${files_render})
file(GLOB_RECURSE files_utils "utils/*.?pp")
source_group("utils" FILES ${files_utils})
file(GLOB files_other "*.?pp")
source_group("source" FILES ${files_other})

file(GLOB_RECURSE files_all "*.?pp")

# library
add_library(glad_lib ${PROJECT_SOURCE_DIR}/glad/src/glad.c)
set(libs
    opengl32.lib
    ${PROJECT_SOURCE_DIR}/glfw/bin/glfw3.lib
    glad_lib
)

# main
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
add_executable(learnopengl ${files_all})
set_property(TARGET learnopengl PROPERTY CXX_STANDARD 17)
target_link_libraries(learnopengl ${libs})
