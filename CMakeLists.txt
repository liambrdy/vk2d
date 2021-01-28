cmake_minimum_required(VERSION 3.16)

project(vk2d 
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)

set(ENABLE_SPVREMAPPER OFF CACHE BOOL "Disable building spv remapper" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Disable building binaries" FORCE)

set(DEPS_DIR ${CMAKE_SOURCE_DIR}/deps)

add_subdirectory(${DEPS_DIR}/glm)
add_subdirectory(${DEPS_DIR}/glslang)
add_subdirectory(${DEPS_DIR}) # Imgui, spirv-reflect
add_subdirectory(${DEPS_DIR}/stb)
add_subdirectory(${DEPS_DIR}/vma)
add_subdirectory(${DEPS_DIR}/volk)

file(GLOB SRCS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)

add_library(vk2d STATIC
    ${SRCS}
)

target_link_libraries(vk2d PUBLIC
    glm
    glslang
    SPIRV
    imgui
    spirv-reflect
    stb
    vma
    volk
)

target_include_directories(vk2d PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

add_executable(sandbox
    ${CMAKE_SOURCE_DIR}/Sandbox/Sandbox.cpp
)

target_link_libraries(sandbox PUBLIC
    vk2d
)