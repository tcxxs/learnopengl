cmake_minimum_required(VERSION 3.0)
project(learnopengl)

# includes
include_directories(${PROJECT_SOURCE_DIR}/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/glad/include)
file(GLOB files_hpp "*.hpp")
source_group("include" FILES ${files_hpp})

# sources
file(GLOB files_cpp "*.cpp")
source_group("source" FILES ${files_cpp})

# library
add_library(glad_lib ${PROJECT_SOURCE_DIR}/glad/src/glad.c)
set(libs
    opengl32.lib
    ${PROJECT_SOURCE_DIR}/glfw/bin/glfw3.lib
    glad_lib
)

# main
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
add_executable(learnopengl ${files_hpp} ${files_cpp})

# link
target_link_libraries(learnopengl ${libs})
