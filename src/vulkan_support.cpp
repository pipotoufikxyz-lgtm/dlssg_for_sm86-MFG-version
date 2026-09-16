#include "vulkan_support.hpp"

#include <utility>

namespace dlssg::vulkan {

namespace {

bool ValidContext(const DeviceContext& context) {
    return context.instance != VK_NULL_HANDLE &&
           context.physical_device != VK_NULL_HANDLE &&
           context.device != VK_NULL_HANDLE && context.queue != VK_NULL_HANDLE &&
           context.queue_family != VK_QUEUE_FAMILY_IGNORED;
}

void TransitionIfValid(VkCommandBuffer command_buffer,
                       ImageResource& image,
                       VkImageLayout new_layout,
                       VkPipelineStageFlags source_stage,
                       VkPipelineStageFlags destination_stage,
                       VkAccessFlags source_access,
                       VkAccessFlags destination_access) {
    if (image.image != VK_NULL_HANDLE && image.layout != new_layout) {
        Bridge::Transition(command_buffer, image, image.layout, new_layout,
                           source_stage, destination_stage, source_access,
                           destination_access);
        image.layout = new_layout;
    }
}

}  // namespace

Bridge::~Bridge() {
    Shutdown();
}

VkResult Bridge::Initialize(const DeviceContext& context,
                            EvaluateCallback evaluate) {
    if (!ValidContext(context) || !evaluate) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    Shutdown();
    context_ = context;
    evaluate_ = std::move(evaluate);

    VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = context_.queue_family;
    VkResult result =
        vkCreateCommandPool(context_.device, &pool_info, nullptr, &command_pool_);
    if (result != VK_SUCCESS) {
        Shutdown();
        return result;
    }

    VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    result = vkCreateFence(context_.device, &fence_info, nullptr, &fence_);
    if (result != VK_SUCCESS) {
        Shutdown();
        return result;
    }
    owns_sync_objects_ = true;
    ready_ = true;
    return VK_SUCCESS;
}

void Bridge::Shutdown() {
    ready_ = false;
    evaluate_ = {};
    if (context_.device != VK_NULL_HANDLE && owns_sync_objects_) {
        if (fence_ != VK_NULL_HANDLE) {
            vkDestroyFence(context_.device, fence_, nullptr);
        }
        if (command_pool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(context_.device, command_pool_, nullptr);
        }
    }
    fence_ = VK_NULL_HANDLE;
    command_pool_ = VK_NULL_HANDLE;
    owns_sync_objects_ = false;
    context_ = {};
}

VkResult Bridge::Reset(const DeviceContext& context) {
    if (!ValidContext(context)) {
        Shutdown();
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    if (ready_ && context.device == context_.device &&
        context.queue == context_.queue &&
        context.queue_family == context_.queue_family) {
        return VK_SUCCESS;
    }
    EvaluateCallback callback = std::move(evaluate_);
    Shutdown();
    return Initialize(context, std::move(callback));
}

VkResult Bridge::RecordEvaluation(VkCommandBuffer command_buffer,
                                  EvaluationResources& resources) {
    if (!ready_ || command_buffer == VK_NULL_HANDLE) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    TransitionIfValid(command_buffer, resources.input,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                      VK_ACCESS_SHADER_READ_BIT);
    TransitionIfValid(command_buffer, resources.flow,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                      VK_ACCESS_SHADER_READ_BIT);
    TransitionIfValid(command_buffer, resources.depth,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                      VK_ACCESS_SHADER_READ_BIT);
    TransitionIfValid(command_buffer, resources.output,
                      VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
                      VK_ACCESS_SHADER_WRITE_BIT);

    VkResult result = evaluate_(command_buffer, resources);
    if (result != VK_SUCCESS) {
        return result;
    }

    TransitionIfValid(command_buffer, resources.output,
                      resources.output.final_layout,
                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_ACCESS_SHADER_WRITE_BIT, 0);
    return VK_SUCCESS;
}

VkResult Bridge::SubmitAndWait(VkCommandBuffer command_buffer) {
    if (!ready_ || command_buffer == VK_NULL_HANDLE) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &command_buffer;
    VkResult result = vkResetFences(context_.device, 1, &fence_);
    if (result != VK_SUCCESS) {
        return result;
    }
    result = vkQueueSubmit(context_.queue, 1, &submit, fence_);
    if (result != VK_SUCCESS) {
        return result;
    }
    return vkWaitForFences(context_.device, 1, &fence_, VK_TRUE, UINT64_MAX);
}

VkResult Bridge::CreateCommandBuffer(VkCommandBuffer* command_buffer) {
    if (!ready_ || command_buffer == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkCommandBufferAllocateInfo info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    info.commandPool = command_pool_;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;
    return vkAllocateCommandBuffers(context_.device, &info, command_buffer);
}

void Bridge::DestroyCommandBuffer(VkCommandBuffer command_buffer) {
    if (ready_ && command_buffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(context_.device, command_pool_, 1, &command_buffer);
    }
}

void Bridge::Transition(VkCommandBuffer command_buffer,
                        const ImageResource& image, VkImageLayout old_layout,
                        VkImageLayout new_layout,
                        VkPipelineStageFlags source_stage,
                        VkPipelineStageFlags destination_stage,
                        VkAccessFlags source_access,
                        VkAccessFlags destination_access) {
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.srcAccessMask = source_access;
    barrier.dstAccessMask = destination_access;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image.image;
    barrier.subresourceRange.aspectMask = image.aspect;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);
}

}  // namespace dlssg::vulkan
