cmake_minimum_required(VERSION 3.21)
project(dlssg_renderer LANGUAGES CXX)

option(DLSSG_ENABLE_VULKAN "Build the Vulkan renderer backend when the Vulkan SDK is available" ON)
option(DLSSG_ENABLE_DX9 "Build the Direct3D 9 renderer backend on Windows" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dlssg_renderer STATIC
    renderer/src/renderer.cpp
)
target_include_directories(dlssg_renderer PUBLIC renderer/include)
target_compile_features(dlssg_renderer PUBLIC cxx_std_17)

if(DLSSG_ENABLE_VULKAN)
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
        target_sources(dlssg_renderer PRIVATE renderer/src/vulkan_renderer.cpp)
        target_link_libraries(dlssg_renderer PRIVATE Vulkan::Vulkan)
        target_compile_definitions(dlssg_renderer PRIVATE DLSSG_HAS_VULKAN=1)
        message(STATUS "DLSSG: Vulkan backend enabled")
    else()
        message(WARNING "DLSSG: Vulkan SDK not found; Vulkan backend will report unavailable")
    endif()
endif()

if(DLSSG_ENABLE_DX9 AND WIN32)
    target_sources(dlssg_renderer PRIVATE renderer/src/dx9_renderer.cpp)
    target_link_libraries(dlssg_renderer PRIVATE d3d9.lib)
    target_compile_definitions(dlssg_renderer PRIVATE DLSSG_HAS_DX9=1)
    message(STATUS "DLSSG: Direct3D 9 backend enabled")
endif()

if(MSVC)
    target_compile_options(dlssg_renderer PRIVATE /W4 /permissive-)
else()
    target_compile_options(dlssg_renderer PRIVATE -Wall -Wextra -Wpedantic)
endif()
