#pragma once

#include "vulkan_route_abi.h"

#include <stdint.h>

#if defined(_WIN32)
#define DLSSG_NGX_API __declspec(dllexport)
#define DLSSG_NGX_CALL __cdecl
#else
#define DLSSG_NGX_API
#define DLSSG_NGX_CALL
#endif

typedef struct DlssgVulkanNgx DlssgVulkanNgx;

typedef struct DlssgVulkanNgxStatus {
    uint32_t struct_size;
    uint32_t abi_version;
    uint32_t required_exports;
    uint32_t resolved_exports;
    uint32_t ready;
    uint32_t supports_extended_init;
    uint32_t supports_feature_v1;
    uint32_t supports_shutdown_v1;
} DlssgVulkanNgxStatus;

typedef struct DlssgVulkanLifecycle {
    uint32_t struct_size;
    uint32_t abi_version;
    DlssgVulkanDevice device;
    VkSwapchainKHR swapchain;
    uint32_t width;
    uint32_t height;
    VkFormat format;
} DlssgVulkanLifecycle;

#ifdef __cplusplus
extern "C" {
#endif

DLSSG_NGX_API VkResult DLSSG_NGX_CALL
DlssgVulkanNgx_Load(const wchar_t* dll_path, DlssgVulkanNgx** loader);

DLSSG_NGX_API void DLSSG_NGX_CALL
DlssgVulkanNgx_Unload(DlssgVulkanNgx* loader);

DLSSG_NGX_API VkResult DLSSG_NGX_CALL
DlssgVulkanNgx_GetStatus(const DlssgVulkanNgx* loader,
                         DlssgVulkanNgxStatus* status);

DLSSG_NGX_API VkResult DLSSG_NGX_CALL
DlssgVulkanNgx_OnDeviceCreated(DlssgVulkanNgx* loader,
                               const DlssgVulkanDevice* device);

DLSSG_NGX_API VkResult DLSSG_NGX_CALL
DlssgVulkanNgx_OnSwapchainChanged(DlssgVulkanNgx* loader,
                                  const DlssgVulkanLifecycle* lifecycle);

DLSSG_NGX_API const char* DLSSG_NGX_CALL
DlssgVulkanNgx_LastError(const DlssgVulkanNgx* loader);

#ifdef __cplusplus
}
#endif
