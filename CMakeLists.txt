cmake_minimum_required(VERSION 3.0)
project(learnopengl)

# includes
include_directories(${CMAKE_SOURCE_DIR}/contrib/include)
include_directories(${CMAKE_SOURCE_DIR}/source)

# sources
file(GLOB_RECURSE files_contrib "contrib/include/*")
source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${files_contrib})
set_property(SOURCE ${files_contrib} APPEND_STRING PROPERTY COMPILE_FLAGS "/w34996")
file(GLOB_RECURSE files_source "source/*")
source_group(TREE "${CMAKE_SOURCE_DIR}/source" FILES ${files_source})

# library
add_library(glad_lib ${CMAKE_SOURCE_DIR}/contrib/source/glad/glad.c)
file(GLOB_RECURSE imgui_src "contrib/source/imgui/*")
add_library(imgui_lib ${imgui_src})
set(libs
    opengl32.lib
    glad_lib
    imgui_lib
    ${CMAKE_SOURCE_DIR}/contrib/lib/glfw3.lib
    ${CMAKE_SOURCE_DIR}/contrib/lib/libyaml-cppmd.lib
    ${CMAKE_SOURCE_DIR}/contrib/lib/assimp.lib
    ${CMAKE_SOURCE_DIR}/contrib/lib/IrrXML.lib
    ${CMAKE_SOURCE_DIR}/contrib/lib/zlibstatic.lib
    ${CMAKE_SOURCE_DIR}/contrib/lib/freetype.lib
)

# main
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
add_executable(learnopengl ${files_contrib} ${files_source})
set_property(TARGET learnopengl PROPERTY COMPILE_FLAGS "/Od /Ob0 /we4715 /wo4996")
set_property(TARGET learnopengl PROPERTY CXX_STANDARD 17)
set_property(TARGET learnopengl PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
target_link_libraries(learnopengl ${libs})
