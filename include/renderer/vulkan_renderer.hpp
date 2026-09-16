#pragma once

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "renderer/renderer.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace renderer {

class VulkanRenderer final : public Renderer {
public:
    ~VulkanRenderer() override;
    bool Initialize(const CreateInfo&, std::string&) override;
    bool Resize(uint32_t, uint32_t, std::string&) override;
    bool BeginFrame(std::string&) override;
    bool EndFrame(std::string&) override;
    void Shutdown() noexcept override;
    const DeviceInfo& Device() const noexcept override { return device_info_; }

private:
    bool CreateInstance(bool validation, std::string&);
    bool CreateDevice(std::string&);
    bool CreateSwapchain(uint32_t, uint32_t, std::string&);
    void DestroySwapchain() noexcept;
    bool Check(VkResult, const char*, std::string&);

    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;
    uint32_t graphics_family_ = UINT32_MAX;
    uint32_t present_family_ = UINT32_MAX;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> images_;
    std::vector<VkImageView> views_;
    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
    VkSemaphore image_available_ = VK_NULL_HANDLE;
    VkSemaphore render_finished_ = VK_NULL_HANDLE;
    VkFence frame_fence_ = VK_NULL_HANDLE;
    uint32_t image_index_ = 0;
    bool frame_active_ = false;
    DeviceInfo device_info_;
};

}  // namespace renderer
