cmake_minimum_required(VERSION 3.20)
project(dlssg_vulkan_support LANGUAGES CXX)

option(DLSSG_BUILD_VULKAN_SUPPORT "Build the Vulkan bridge library" ON)
if(DLSSG_BUILD_VULKAN_SUPPORT)
    add_subdirectory(src)
endif()
