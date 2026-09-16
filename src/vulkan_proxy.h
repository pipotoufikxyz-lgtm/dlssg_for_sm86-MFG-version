#pragma once

#include "vulkan_route_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DlssgVulkanProxy DlssgVulkanProxy;

DLSSG_VULKAN_API VkResult DLSSG_VULKAN_CALL
DlssgVulkanProxy_Create(const DlssgVulkanRouteConfig* config,
                        const wchar_t* route_dll_path,
                        DlssgVulkanProxy** proxy);

DLSSG_VULKAN_API void DLSSG_VULKAN_CALL
DlssgVulkanProxy_Destroy(DlssgVulkanProxy* proxy);

DLSSG_VULKAN_API VkResult DLSSG_VULKAN_CALL
DlssgVulkanProxy_Reset(DlssgVulkanProxy* proxy,
                       const DlssgVulkanDevice* device);

DLSSG_VULKAN_API VkResult DLSSG_VULKAN_CALL
DlssgVulkanProxy_Evaluate(DlssgVulkanProxy* proxy,
                          const DlssgVulkanResources* resources);

DLSSG_VULKAN_API const char* DLSSG_VULKAN_CALL
DlssgVulkanProxy_LastError(const DlssgVulkanProxy* proxy);

#ifdef __cplusplus
}
#endif
