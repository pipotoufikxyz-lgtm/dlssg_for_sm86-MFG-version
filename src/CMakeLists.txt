add_library(dlssg_vulkan_route SHARED vulkan_support.cpp vulkan_route.cpp
            vulkan_route_abi.cpp)
set_target_properties(dlssg_vulkan_route PROPERTIES
    OUTPUT_NAME "dlssg_vulkan_route")
target_include_directories(dlssg_vulkan_route PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(dlssg_vulkan_route PUBLIC cxx_std_17)

find_package(Vulkan REQUIRED)
target_link_libraries(dlssg_vulkan_route PUBLIC Vulkan::Vulkan)

add_library(dlssg_vulkan_proxy SHARED vulkan_proxy.cpp)
target_include_directories(dlssg_vulkan_proxy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(dlssg_vulkan_proxy PUBLIC cxx_std_17)
target_link_libraries(dlssg_vulkan_proxy PRIVATE Vulkan::Vulkan)

add_library(dlssg_vulkan_ngx SHARED vulkan_ngx_loader.cpp)
target_include_directories(dlssg_vulkan_ngx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(dlssg_vulkan_ngx PUBLIC cxx_std_17)
target_link_libraries(dlssg_vulkan_ngx PRIVATE Vulkan::Vulkan)
