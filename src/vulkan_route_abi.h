#pragma once

#include <stdint.h>

#include <vulkan/vulkan.h>

#if defined(_WIN32)
#define DLSSG_VULKAN_API __declspec(dllexport)
#define DLSSG_VULKAN_CALL __cdecl
#else
#define DLSSG_VULKAN_API
#define DLSSG_VULKAN_CALL
#endif

#define DLSSG_VULKAN_ABI_VERSION 1u

typedef struct DlssgVulkanRoute DlssgVulkanRoute;

typedef struct DlssgVulkanImage {
    VkImage image;
    VkImageLayout initial_layout;
    VkImageLayout final_layout;
    VkImageAspectFlags aspect;
    VkFormat format;
    VkExtent3D extent;
} DlssgVulkanImage;

typedef struct DlssgVulkanResources {
    DlssgVulkanImage input;
    DlssgVulkanImage output;
    DlssgVulkanImage flow;
    DlssgVulkanImage depth;
} DlssgVulkanResources;

typedef struct DlssgVulkanDevice {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue queue;
    uint32_t queue_family;
} DlssgVulkanDevice;

typedef VkResult(DLSSG_VULKAN_CALL* DlssgVulkanInitializeFn)(
    void* user_data, const DlssgVulkanDevice* device);
typedef VkResult(DLSSG_VULKAN_CALL* DlssgVulkanEvaluateFn)(
    void* user_data, VkCommandBuffer command_buffer,
    const DlssgVulkanResources* resources);
typedef void(DLSSG_VULKAN_CALL* DlssgVulkanShutdownFn)(void* user_data);

typedef struct DlssgVulkanAdapter {
    uint32_t struct_size;
    uint32_t abi_version;
    void* user_data;
    DlssgVulkanInitializeFn initialize;
    DlssgVulkanEvaluateFn evaluate;
    DlssgVulkanShutdownFn shutdown;
} DlssgVulkanAdapter;

typedef struct DlssgVulkanRouteConfig {
    uint32_t struct_size;
    uint32_t abi_version;
    DlssgVulkanDevice device;
    DlssgVulkanAdapter adapter;
} DlssgVulkanRouteConfig;

typedef struct DlssgVulkanRouteApi {
    uint32_t struct_size;
    uint32_t abi_version;
    VkResult(DLSSG_VULKAN_CALL* create)(
        const DlssgVulkanRouteConfig* config, DlssgVulkanRoute** route);
    void(DLSSG_VULKAN_CALL* destroy)(DlssgVulkanRoute* route);
    VkResult(DLSSG_VULKAN_CALL* reset)(
        DlssgVulkanRoute* route, const DlssgVulkanDevice* device);
    VkResult(DLSSG_VULKAN_CALL* evaluate)(
        DlssgVulkanRoute* route, const DlssgVulkanResources* resources);
    const char*(DLSSG_VULKAN_CALL* last_error)(const DlssgVulkanRoute* route);
} DlssgVulkanRouteApi;

#ifdef __cplusplus
extern "C" {
#endif

DLSSG_VULKAN_API VkResult DLSSG_VULKAN_CALL
DlssgVulkan_GetRouteApi(DlssgVulkanRouteApi* api);

#ifdef __cplusplus
}
#endif
