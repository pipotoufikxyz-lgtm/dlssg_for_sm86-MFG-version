#include "renderer/renderer.h"

#include <vulkan/vulkan.h>

#include <sstream>
#include <utility>

namespace dlssg::renderer {
namespace {

class VulkanRenderer final : public Renderer {
public:
    ~VulkanRenderer() override { shutdown(); }

    Backend backend() const noexcept override { return Backend::Vulkan; }
    const char* backendName() const noexcept override { return "vulkan"; }

    Result initialize(const NativeWindow&, const RendererOptions& options) override {
        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = "DLSSG renderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "DLSSG renderer";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        createInfo.pApplicationInfo = &appInfo;
        const VkResult created = vkCreateInstance(&createInfo, nullptr, &instance_);
        if (created != VK_SUCCESS) {
            return Result::failure("vkCreateInstance failed with code " + std::to_string(created));
        }

        uint32_t deviceCount = 0;
        if (vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr) != VK_SUCCESS || deviceCount == 0) {
            shutdown();
            return Result::failure("No Vulkan physical device was found");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        if (vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data()) != VK_SUCCESS) {
            shutdown();
            return Result::failure("Unable to enumerate Vulkan physical devices");
        }

        for (VkPhysicalDevice candidate : devices) {
            uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, families.data());
            for (uint32_t family = 0; family < familyCount; ++family) {
                if ((families[family].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                    physicalDevice_ = candidate;
                    graphicsFamily_ = family;
                    break;
                }
            }
            if (physicalDevice_ != VK_NULL_HANDLE) {
                break;
            }
        }

        if (physicalDevice_ == VK_NULL_HANDLE) {
            shutdown();
            return Result::failure("No Vulkan graphics queue family was found");
        }

        float priority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueInfo.queueFamilyIndex = graphicsFamily_;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;
        VkDeviceCreateInfo deviceInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        const VkResult deviceResult = vkCreateDevice(physicalDevice_, &deviceInfo, nullptr, &device_);
        if (deviceResult != VK_SUCCESS) {
            shutdown();
            return Result::failure("vkCreateDevice failed with code " + std::to_string(deviceResult));
        }
        vkGetDeviceQueue(device_, graphicsFamily_, 0, &graphicsQueue_);
        validationRequested_ = options.enableValidation;
        return Result::success();
    }

    Result resize(uint32_t, uint32_t) override { return initialized() ? Result::success() : notInitialized(); }
    Result beginFrame() override { return initialized() ? Result::success() : notInitialized(); }
    Result endFrame() override { return initialized() ? Result::success() : notInitialized(); }

    void shutdown() noexcept override {
        if (device_ != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(device_);
            vkDestroyDevice(device_, nullptr);
            device_ = VK_NULL_HANDLE;
        }
        if (instance_ != VK_NULL_HANDLE) {
            vkDestroyInstance(instance_, nullptr);
            instance_ = VK_NULL_HANDLE;
        }
        physicalDevice_ = VK_NULL_HANDLE;
        graphicsQueue_ = VK_NULL_HANDLE;
    }

private:
    bool initialized() const noexcept { return device_ != VK_NULL_HANDLE; }
    static Result notInitialized() { return Result::failure("Vulkan renderer is not initialized"); }

    VkInstance instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    uint32_t graphicsFamily_ = 0;
    bool validationRequested_ = false;
};

} // namespace

std::unique_ptr<Renderer> createVulkanRenderer() {
    return std::make_unique<VulkanRenderer>();
}

} // namespace dlssg::renderer
