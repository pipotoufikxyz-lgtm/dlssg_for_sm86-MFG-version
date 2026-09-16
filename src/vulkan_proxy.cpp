#include "vulkan_proxy.h"

#include <windows.h>

#include <new>
#include <string>

struct DlssgVulkanProxy {
    HMODULE route_module = nullptr;
    DlssgVulkanRouteApi route_api{};
    DlssgVulkanRoute* route = nullptr;
    std::string error;
};

namespace {

using GetRouteApiFn = VkResult(DLSSG_VULKAN_CALL*)(
    DlssgVulkanRouteApi*);

void SetError(DlssgVulkanProxy* proxy, const char* message) {
    proxy->error = message;
}

VkResult LoadRoute(DlssgVulkanProxy* proxy, const wchar_t* path) {
    proxy->route_module = LoadLibraryW(path != nullptr ? path
                                                        : L"dlssg_vulkan_route.dll");
    if (proxy->route_module == nullptr) {
        SetError(proxy, "Could not load dlssg_vulkan_route.dll");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    auto get_api = reinterpret_cast<GetRouteApiFn>(
        GetProcAddress(proxy->route_module, "DlssgVulkan_GetRouteApi"));
    if (get_api == nullptr) {
        SetError(proxy, "Loaded route DLL has no Vulkan route ABI export");
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    proxy->route_api.struct_size = sizeof(proxy->route_api);
    proxy->route_api.abi_version = DLSSG_VULKAN_ABI_VERSION;
    VkResult result = get_api(&proxy->route_api);
    if (result != VK_SUCCESS) {
        SetError(proxy, "Vulkan route ABI version negotiation failed");
    }
    return result;
}

void UnloadRoute(DlssgVulkanProxy* proxy) {
    if (proxy->route != nullptr && proxy->route_api.destroy != nullptr) {
        proxy->route_api.destroy(proxy->route);
    }
    proxy->route = nullptr;
    if (proxy->route_module != nullptr) {
        FreeLibrary(proxy->route_module);
    }
    proxy->route_module = nullptr;
}

}  // namespace

extern "C" DLSSG_VULKAN_API VkResult DLSSG_VULKAN_CALL
DlssgVulkanProxy_Create(const DlssgVulkanRouteConfig* config,
                        const wchar_t* route_dll_path,
                        DlssgVulkanProxy** output) {
    if (config == nullptr || output == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    *output = nullptr;
    DlssgVulkanProxy* proxy = new (std::nothrow) DlssgVulkanProxy();
    if (proxy == nullptr) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    VkResult result = LoadRoute(proxy, route_dll_path);
    if (result == VK_SUCCESS) {
        result = proxy->route_api.create(config, &proxy->route);
        if (result != VK_SUCCESS) {
            SetError(proxy, "Vulkan route creation failed");
        }
    }
    if (result != VK_SUCCESS) {
        UnloadRoute(proxy);
        delete proxy;
        return result;
    }
    proxy->error.clear();
    *output = proxy;
    return VK_SUCCESS;
}

extern "C" DLSSG_VULKAN_API void DLSSG_VULKAN_CALL
DlssgVulkanProxy_Destroy(DlssgVulkanProxy* proxy) {
    if (proxy == nullptr) {
        return;
    }
    UnloadRoute(proxy);
    delete proxy;
}

extern "C" DLSSG_VULKAN_API VkResult DLSSG_VULKAN_CALL
DlssgVulkanProxy_Reset(DlssgVulkanProxy* proxy,
                       const DlssgVulkanDevice* device) {
    if (proxy == nullptr || device == nullptr || proxy->route == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkResult result = proxy->route_api.reset(proxy->route, device);
    if (result != VK_SUCCESS) {
        SetError(proxy, proxy->route_api.last_error(proxy->route));
    }
    return result;
}

extern "C" DLSSG_VULKAN_API VkResult DLSSG_VULKAN_CALL
DlssgVulkanProxy_Evaluate(DlssgVulkanProxy* proxy,
                          const DlssgVulkanResources* resources) {
    if (proxy == nullptr || resources == nullptr || proxy->route == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkResult result = proxy->route_api.evaluate(proxy->route, resources);
    if (result != VK_SUCCESS) {
        SetError(proxy, proxy->route_api.last_error(proxy->route));
    }
    return result;
}

extern "C" DLSSG_VULKAN_API const char* DLSSG_VULKAN_CALL
DlssgVulkanProxy_LastError(const DlssgVulkanProxy* proxy) {
    return proxy == nullptr ? "Vulkan proxy handle is null" : proxy->error.c_str();
}
