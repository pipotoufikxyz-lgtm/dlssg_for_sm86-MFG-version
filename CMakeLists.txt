cmake_minimum_required(VERSION 3.20)
project(dlssg_vulkan_support LANGUAGES CXX)

add_library(renderer_backends STATIC
    src/renderer/renderer.cpp
    src/renderer/vulkan_renderer.cpp
    src/renderer/dx9_renderer.cpp)
target_include_directories(renderer_backends PUBLIC include)
target_compile_features(renderer_backends PUBLIC cxx_std_17)
find_package(Vulkan REQUIRED)
target_link_libraries(renderer_backends PUBLIC Vulkan::Vulkan d3d9)

option(DLSSG_BUILD_VULKAN_SUPPORT "Build the Vulkan bridge library" ON)
if(DLSSG_BUILD_VULKAN_SUPPORT)
    add_subdirectory(src)
endif()
