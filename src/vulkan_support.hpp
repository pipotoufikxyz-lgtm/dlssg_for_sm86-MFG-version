#pragma once

#include <cstdint>
#include <functional>

#include <vulkan/vulkan.h>

namespace dlssg::vulkan {

struct ImageResource {
    VkImage image = VK_NULL_HANDLE;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout final_layout = VK_IMAGE_LAYOUT_GENERAL;
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent3D extent{0, 0, 0};
};

struct EvaluationResources {
    ImageResource input;
    ImageResource output;
    ImageResource flow;
    ImageResource depth;
};

struct DeviceContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t queue_family = VK_QUEUE_FAMILY_IGNORED;
};

using EvaluateCallback =
    std::function<VkResult(VkCommandBuffer, const EvaluationResources&)>;

class Bridge final {
public:
    Bridge() = default;
    ~Bridge();

    Bridge(const Bridge&) = delete;
    Bridge& operator=(const Bridge&) = delete;

    VkResult Initialize(const DeviceContext& context, EvaluateCallback evaluate);
    void Shutdown();
    VkResult Reset(const DeviceContext& context);

    VkResult RecordEvaluation(VkCommandBuffer command_buffer,
                              EvaluationResources& resources);
    VkResult SubmitAndWait(VkCommandBuffer command_buffer);
    VkResult CreateCommandBuffer(VkCommandBuffer* command_buffer);
    void DestroyCommandBuffer(VkCommandBuffer command_buffer);

    bool Ready() const noexcept { return ready_; }
    const DeviceContext& Context() const noexcept { return context_; }

    static void Transition(VkCommandBuffer command_buffer,
                           const ImageResource& image,
                           VkImageLayout old_layout,
                           VkImageLayout new_layout,
                           VkPipelineStageFlags source_stage,
                           VkPipelineStageFlags destination_stage,
                           VkAccessFlags source_access,
                           VkAccessFlags destination_access);

private:
    DeviceContext context_{};
    EvaluateCallback evaluate_{};
    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    VkFence fence_ = VK_NULL_HANDLE;
    bool owns_sync_objects_ = false;
    bool ready_ = false;
};

}  // namespace dlssg::vulkan
