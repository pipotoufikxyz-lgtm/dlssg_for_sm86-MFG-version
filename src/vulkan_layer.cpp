#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace {

struct InstanceState {
    VkInstance instance = VK_NULL_HANDLE;
    PFN_vkGetInstanceProcAddr nextGetInstanceProcAddr = nullptr;
    PFN_vkDestroyInstance nextDestroyInstance = nullptr;
    PFN_vkCreateDevice nextCreateDevice = nullptr;
};

struct DeviceState {
    VkDevice device = VK_NULL_HANDLE;
    VkInstance instance = VK_NULL_HANDLE;
    PFN_vkGetDeviceProcAddr nextGetDeviceProcAddr = nullptr;
    PFN_vkDestroyDevice nextDestroyDevice = nullptr;
    PFN_vkCreateSwapchainKHR nextCreateSwapchain = nullptr;
    PFN_vkDestroySwapchainKHR nextDestroySwapchain = nullptr;
    PFN_vkQueuePresentKHR nextQueuePresent = nullptr;
};

std::mutex g_mutex;
std::unordered_map<VkInstance, InstanceState> g_instances;
std::unordered_map<VkDevice, DeviceState> g_devices;
std::unordered_map<VkPhysicalDevice, VkInstance> g_physicalInstances;
FILE* g_log = nullptr;

void Log(const char* message) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_log == nullptr) {
        wchar_t path[MAX_PATH]{};
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        wchar_t* slash = wcsrchr(path, L'\\');
        if (slash != nullptr) {
            *(slash + 1) = L'\0';
        }
        wchar_t logPath[MAX_PATH]{};
        _snwprintf_s(logPath, MAX_PATH, _TRUNCATE, L"%sdlssg_vulkan_layer.log", path);
        _wfopen_s(&g_log, logPath, L"a");
    }
    if (g_log != nullptr) {
        std::fprintf(g_log, "[dlssg-vulkan-layer] %s\n", message);
        std::fflush(g_log);
    }
}

InstanceState* FindInstance(VkInstance instance) {
    auto it = g_instances.find(instance);
    return it == g_instances.end() ? nullptr : &it->second;
}

DeviceState* FindDevice(VkDevice device) {
    auto it = g_devices.find(device);
    return it == g_devices.end() ? nullptr : &it->second;
}

} // namespace

extern "C" __declspec(dllexport) VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance instance, const char* name);

extern "C" __declspec(dllexport) VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice device, const char* name);

extern "C" __declspec(dllexport) VKAPI_ATTR VkResult VKAPI_CALL
vkCreateInstance(const VkInstanceCreateInfo* createInfo,
                 const VkAllocationCallbacks* allocator,
                 VkInstance* instance) {
    auto link = reinterpret_cast<VkLayerInstanceCreateInfo*>(
        const_cast<VkBaseInStructure*>(
            reinterpret_cast<const VkBaseInStructure*>(createInfo->pNext)));
    while (link != nullptr &&
           link->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO) {
        link = reinterpret_cast<VkLayerInstanceCreateInfo*>(
            const_cast<VkBaseInStructure*>(
                reinterpret_cast<const VkBaseInStructure*>(link->pNext)));
    }
    if (link == nullptr || link->function != VK_LAYER_LINK_INFO ||
        link->u.pLayerInfo == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    auto gipa = link->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    link->u.pLayerInfo = link->u.pLayerInfo->pNext;
    if (gipa == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    auto create = reinterpret_cast<PFN_vkCreateInstance>(
        gipa(VK_NULL_HANDLE, "vkCreateInstance"));
    if (create == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkResult result = create(createInfo, allocator, instance);
    if (result != VK_SUCCESS) {
        return result;
    }
    InstanceState state{};
    state.instance = *instance;
    state.nextGetInstanceProcAddr = gipa;
    state.nextDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(
        gipa(*instance, "vkDestroyInstance"));
    state.nextCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(
        gipa(*instance, "vkCreateDevice"));
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_instances.emplace(*instance, state);
    }
    Log("Vulkan instance created");
    return VK_SUCCESS;
}

extern "C" __declspec(dllexport) VKAPI_ATTR void VKAPI_CALL
vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* allocator) {
    InstanceState state{};
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_instances.find(instance);
        if (it == g_instances.end()) return;
        state = it->second;
        g_instances.erase(it);
    }
    if (state.nextDestroyInstance != nullptr) {
        state.nextDestroyInstance(instance, allocator);
    }
    Log("Vulkan instance destroyed");
}

extern "C" __declspec(dllexport) VKAPI_ATTR VkResult VKAPI_CALL
vkCreateDevice(VkPhysicalDevice physicalDevice,
               const VkDeviceCreateInfo* createInfo,
               const VkAllocationCallbacks* allocator,
               VkDevice* device) {
    VkInstance instance = VK_NULL_HANDLE;
    PFN_vkCreateDevice create = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto physical = g_physicalInstances.find(physicalDevice);
        if (physical != g_physicalInstances.end()) {
            instance = physical->second;
            auto state = g_instances.find(instance);
            if (state != g_instances.end()) {
                create = reinterpret_cast<PFN_vkCreateDevice>(
                    state->second.nextGetInstanceProcAddr(instance, "vkCreateDevice"));
            }
        }
    }
    if (create == nullptr) return VK_ERROR_INITIALIZATION_FAILED;
    auto deviceLink = reinterpret_cast<VkLayerDeviceCreateInfo*>(
        const_cast<VkBaseInStructure*>(
            reinterpret_cast<const VkBaseInStructure*>(createInfo->pNext)));
    while (deviceLink != nullptr &&
           deviceLink->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO) {
        deviceLink = reinterpret_cast<VkLayerDeviceCreateInfo*>(
            const_cast<VkBaseInStructure*>(
                reinterpret_cast<const VkBaseInStructure*>(deviceLink->pNext)));
    }
    if (deviceLink != nullptr && deviceLink->function == VK_LAYER_LINK_INFO &&
        deviceLink->u.pLayerInfo != nullptr) {
        create = reinterpret_cast<PFN_vkCreateDevice>(
            deviceLink->u.pLayerInfo->pfnNextGetInstanceProcAddr(
                instance, "vkCreateDevice"));
        deviceLink->u.pLayerInfo = deviceLink->u.pLayerInfo->pNext;
    }
    VkResult result = create(physicalDevice, createInfo, allocator, device);
    if (result != VK_SUCCESS) return result;
    auto gdpa = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
        FindInstance(instance)->nextGetInstanceProcAddr(instance, "vkGetDeviceProcAddr"));
    DeviceState state{};
    state.device = *device;
    state.instance = instance;
    state.nextGetDeviceProcAddr = gdpa;
    state.nextDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(
        gdpa(*device, "vkDestroyDevice"));
    state.nextCreateSwapchain = reinterpret_cast<PFN_vkCreateSwapchainKHR>(
        gdpa(*device, "vkCreateSwapchainKHR"));
    state.nextDestroySwapchain = reinterpret_cast<PFN_vkDestroySwapchainKHR>(
        gdpa(*device, "vkDestroySwapchainKHR"));
    state.nextQueuePresent = reinterpret_cast<PFN_vkQueuePresentKHR>(
        gdpa(*device, "vkQueuePresentKHR"));
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_devices.emplace(*device, state);
    }

    Log("Vulkan device created");
    return VK_SUCCESS;
}

extern "C" __declspec(dllexport) VKAPI_ATTR VkResult VKAPI_CALL
vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* count,
                           VkPhysicalDevice* devices) {
    InstanceState* state = FindInstance(instance);
    if (state == nullptr || state->nextGetInstanceProcAddr == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    auto enumerate = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(
        state->nextGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"));
    if (enumerate == nullptr) return VK_ERROR_INITIALIZATION_FAILED;
    VkResult result = enumerate(instance, count, devices);
    if (result == VK_SUCCESS && devices != nullptr) {
        std::lock_guard<std::mutex> lock(g_mutex);
        for (uint32_t i = 0; i < *count; ++i) g_physicalInstances[devices[i]] = instance;
    }
    return result;
}

extern "C" __declspec(dllexport) VKAPI_ATTR void VKAPI_CALL
vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* allocator) {
    DeviceState state{};
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_devices.find(device);
        if (it == g_devices.end()) return;
        state = it->second;
        g_devices.erase(it);
    }
    if (state.nextDestroyDevice != nullptr) state.nextDestroyDevice(device, allocator);
    Log("Vulkan device destroyed");
}

extern "C" __declspec(dllexport) VKAPI_ATTR VkResult VKAPI_CALL
vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* info,
                     const VkAllocationCallbacks* allocator, VkSwapchainKHR* swapchain) {
    DeviceState* state = FindDevice(device);
    if (state == nullptr || state->nextCreateSwapchain == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkResult result = state->nextCreateSwapchain(device, info, allocator, swapchain);
    if (result == VK_SUCCESS) Log("Vulkan swapchain created");
    return result;
}

extern "C" __declspec(dllexport) VKAPI_ATTR void VKAPI_CALL
vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                      const VkAllocationCallbacks* allocator) {
    DeviceState* state = FindDevice(device);
    if (state != nullptr && state->nextDestroySwapchain != nullptr) {
        state->nextDestroySwapchain(device, swapchain, allocator);
    }
}

extern "C" __declspec(dllexport) VKAPI_ATTR VkResult VKAPI_CALL
vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* info) {
    PFN_vkQueuePresentKHR present = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        for (const auto& pair : g_devices) {
            if (pair.second.nextQueuePresent != nullptr) {
                present = pair.second.nextQueuePresent;
                break;
            }
        }
    }
    if (present != nullptr) {
        VkResult result = present(queue, info);
        if (result == VK_SUCCESS) Log("Vulkan frame presented");
        return result;
    }
    return VK_ERROR_INITIALIZATION_FAILED;
}

extern "C" __declspec(dllexport) VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance instance, const char* name) {
    if (std::strcmp(name, "vkGetInstanceProcAddr") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkGetInstanceProcAddr);
    if (std::strcmp(name, "vkGetDeviceProcAddr") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkGetDeviceProcAddr);
    if (std::strcmp(name, "vkCreateInstance") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkCreateInstance);
    if (std::strcmp(name, "vkDestroyInstance") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkDestroyInstance);
    if (std::strcmp(name, "vkCreateDevice") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkCreateDevice);
    if (std::strcmp(name, "vkEnumeratePhysicalDevices") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkEnumeratePhysicalDevices);
    return nullptr;
}

VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* version) {
    if (version == nullptr || version->sType != LAYER_NEGOTIATE_INTERFACE_STRUCT) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    if (version->loaderLayerInterfaceVersion > 2) version->loaderLayerInterfaceVersion = 2;
    return VK_SUCCESS;
}

extern "C" __declspec(dllexport) VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice device, const char* name) {
    if (std::strcmp(name, "vkGetDeviceProcAddr") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkGetDeviceProcAddr);
    if (std::strcmp(name, "vkDestroyDevice") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkDestroyDevice);
    if (std::strcmp(name, "vkCreateSwapchainKHR") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkCreateSwapchainKHR);
    if (std::strcmp(name, "vkDestroySwapchainKHR") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkDestroySwapchainKHR);
    if (std::strcmp(name, "vkQueuePresentKHR") == 0) return reinterpret_cast<PFN_vkVoidFunction>(&vkQueuePresentKHR);
    DeviceState* state = FindDevice(device);
    return state == nullptr || state->nextGetDeviceProcAddr == nullptr
        ? nullptr : state->nextGetDeviceProcAddr(device, name);
}
