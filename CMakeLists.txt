cmake_minimum_required(VERSION 3.0)
project(learnopengl)

# includes
include(config.h)
include_directories(${PROJECT_SOURCE_DIR}/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/glad/include)

# library
add_library(glad_lib ${PROJECT_SOURCE_DIR}/glad/src/glad.c)
set(libs
    opengl32.lib
    ${PROJECT_SOURCE_DIR}/glfw/bin/glfw3.lib
    glad_lib
)

# main
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
add_executable(learnopengl main.cpp)

# link
target_link_libraries(learnopengl ${libs})
