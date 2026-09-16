#include "vulkan_route_abi.h"

#include "vulkan_route.hpp"

#include <cstddef>
#include <new>

namespace {

class AbiAdapter final : public dlssg::vulkan::FeatureAdapter {
public:
    explicit AbiAdapter(const DlssgVulkanAdapter& adapter) : adapter_(adapter) {}

    VkResult Initialize(const dlssg::vulkan::DeviceContext& context) override {
        const DlssgVulkanDevice device = ToAbi(context);
        return adapter_.initialize(adapter_.user_data, &device);
    }

    VkResult Evaluate(
        VkCommandBuffer command_buffer,
        const dlssg::vulkan::EvaluationResources& resources) override {
        const DlssgVulkanResources converted = ToAbi(resources);
        return adapter_.evaluate(adapter_.user_data, command_buffer,
                                 &converted);
    }

    void Shutdown() override {
        if (adapter_.shutdown != nullptr) {
            adapter_.shutdown(adapter_.user_data);
        }
    }

private:
    static DlssgVulkanDevice ToAbi(
        const dlssg::vulkan::DeviceContext& context) {
        return {context.instance, context.physical_device, context.device,
                context.queue, context.queue_family};
    }

    static DlssgVulkanImage ToAbi(
        const dlssg::vulkan::ImageResource& image) {
        return {image.image, image.layout, image.final_layout, image.aspect,
                image.format, image.extent};
    }

    static DlssgVulkanResources ToAbi(
        const dlssg::vulkan::EvaluationResources& resources) {
        return {ToAbi(resources.input), ToAbi(resources.output),
                ToAbi(resources.flow), ToAbi(resources.depth)};
    }

    DlssgVulkanAdapter adapter_;
};

struct RouteHandle {
    dlssg::vulkan::Route route;
    AbiAdapter adapter;

    explicit RouteHandle(const DlssgVulkanAdapter& value) : adapter(value) {}
};

bool HasSize(uint32_t actual, size_t required) {
    return actual >= required;
}

bool ValidAdapter(const DlssgVulkanAdapter& adapter) {
    return HasSize(adapter.struct_size, sizeof(DlssgVulkanAdapter)) &&
           adapter.abi_version == DLSSG_VULKAN_ABI_VERSION &&
           adapter.initialize != nullptr && adapter.evaluate != nullptr;
}

bool ValidDevice(const DlssgVulkanDevice& device) {
    return device.instance != VK_NULL_HANDLE &&
           device.physical_device != VK_NULL_HANDLE &&
           device.device != VK_NULL_HANDLE && device.queue != VK_NULL_HANDLE &&
           device.queue_family != VK_QUEUE_FAMILY_IGNORED;
}

dlssg::vulkan::DeviceContext ToCpp(const DlssgVulkanDevice& device) {
    return {device.instance, device.physical_device, device.device,
            device.queue, device.queue_family};
}

VkResult DLSSG_VULKAN_CALL Create(const DlssgVulkanRouteConfig* config,
                                  DlssgVulkanRoute** output) {
    if (config == nullptr || output == nullptr ||
        !HasSize(config->struct_size, sizeof(DlssgVulkanRouteConfig)) ||
        config->abi_version != DLSSG_VULKAN_ABI_VERSION ||
        !ValidDevice(config->device) || !ValidAdapter(config->adapter)) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    *output = nullptr;
    RouteHandle* handle = new (std::nothrow) RouteHandle(config->adapter);
    if (handle == nullptr) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    VkResult result = handle->route.Enable(ToCpp(config->device), handle->adapter);
    if (result != VK_SUCCESS) {
        delete handle;
        return result;
    }
    *output = reinterpret_cast<DlssgVulkanRoute*>(handle);
    return VK_SUCCESS;
}

void DLSSG_VULKAN_CALL Destroy(DlssgVulkanRoute* route) {
    delete reinterpret_cast<RouteHandle*>(route);
}

VkResult DLSSG_VULKAN_CALL Reset(DlssgVulkanRoute* route,
                                 const DlssgVulkanDevice* device) {
    if (route == nullptr || device == nullptr || !ValidDevice(*device)) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return reinterpret_cast<RouteHandle*>(route)->route.Reset(ToCpp(*device));
}

VkResult DLSSG_VULKAN_CALL Evaluate(DlssgVulkanRoute* route,
                                    const DlssgVulkanResources* resources) {
    if (route == nullptr || resources == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    const DlssgVulkanResources& value = *resources;
    dlssg::vulkan::EvaluationResources converted{
        {value.input.image, value.input.initial_layout, value.input.final_layout,
         value.input.aspect, value.input.format, value.input.extent},
        {value.output.image, value.output.initial_layout,
         value.output.final_layout, value.output.aspect, value.output.format,
         value.output.extent},
        {value.flow.image, value.flow.initial_layout, value.flow.final_layout,
         value.flow.aspect, value.flow.format, value.flow.extent},
        {value.depth.image, value.depth.initial_layout,
         value.depth.final_layout, value.depth.aspect, value.depth.format,
         value.depth.extent}};
    return reinterpret_cast<RouteHandle*>(route)->route.Evaluate(converted);
}

const char* DLSSG_VULKAN_CALL LastError(const DlssgVulkanRoute* route) {
    if (route == nullptr) {
        return "Vulkan route handle is null";
    }
    return reinterpret_cast<const RouteHandle*>(route)->route.LastError();
}

}  // namespace

extern "C" DLSSG_VULKAN_API VkResult DLSSG_VULKAN_CALL
DlssgVulkan_GetRouteApi(DlssgVulkanRouteApi* api) {
    if (api == nullptr || !HasSize(api->struct_size, sizeof(DlssgVulkanRouteApi)) ||
        api->abi_version != DLSSG_VULKAN_ABI_VERSION) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    api->create = Create;
    api->destroy = Destroy;
    api->reset = Reset;
    api->evaluate = Evaluate;
    api->last_error = LastError;
    return VK_SUCCESS;
}
