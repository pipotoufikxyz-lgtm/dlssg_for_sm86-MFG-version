#include "vulkan_route.hpp"

namespace dlssg::vulkan {

Route::~Route() {
    Disable();
}

VkResult Route::SetError(VkResult result, const char* message) {
    last_error_ = message;
    return result;
}

VkResult Route::Enable(const DeviceContext& context, FeatureAdapter& adapter) {
    Disable();
    adapter_ = &adapter;
    context_ = context;

    VkResult result = adapter_->Initialize(context_);
    if (result != VK_SUCCESS) {
        adapter_ = nullptr;
        context_ = {};
        return SetError(result, "Vulkan feature adapter initialization failed");
    }

    result = bridge_.Initialize(
        context_, [this](VkCommandBuffer command_buffer,
                         const EvaluationResources& resources) {
            return adapter_->Evaluate(command_buffer, resources);
        });
    if (result != VK_SUCCESS) {
        adapter_->Shutdown();
        adapter_ = nullptr;
        context_ = {};
        return SetError(result, "Vulkan synchronization initialization failed");
    }

    result = bridge_.CreateCommandBuffer(&command_buffer_);
    if (result != VK_SUCCESS) {
        bridge_.Shutdown();
        adapter_->Shutdown();
        adapter_ = nullptr;
        context_ = {};
        return SetError(result, "Vulkan command buffer allocation failed");
    }

    enabled_ = true;
    last_error_.clear();
    return VK_SUCCESS;
}

void Route::Disable() {
    ClearState(true);
}

void Route::ClearState(bool shutdown_adapter) {
    enabled_ = false;
    if (command_buffer_ != VK_NULL_HANDLE) {
        bridge_.DestroyCommandBuffer(command_buffer_);
        command_buffer_ = VK_NULL_HANDLE;
    }
    bridge_.Shutdown();
    if (shutdown_adapter && adapter_ != nullptr) {
        adapter_->Shutdown();
    }
    adapter_ = nullptr;
    context_ = {};
}

VkResult Route::Reset(const DeviceContext& context) {
    if (!enabled_ || adapter_ == nullptr) {
        return SetError(VK_ERROR_INITIALIZATION_FAILED,
                        "Vulkan route is not enabled");
    }

    if (context.device == context_.device &&
        context.queue == context_.queue &&
        context.queue_family == context_.queue_family) {
        return VK_SUCCESS;
    }

    FeatureAdapter* adapter = adapter_;
    adapter_->Shutdown();
    ClearState(false);
    return Enable(context, *adapter);
}

VkResult Route::Evaluate(EvaluationResources resources) {
    if (!enabled_ || command_buffer_ == VK_NULL_HANDLE) {
        return SetError(VK_ERROR_INITIALIZATION_FAILED,
                        "Vulkan route is not enabled");
    }

    VkResult result = vkResetCommandBuffer(command_buffer_, 0);
    if (result != VK_SUCCESS) {
        return SetError(result, "Vulkan command buffer reset failed");
    }

    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    result = vkBeginCommandBuffer(command_buffer_, &begin);
    if (result != VK_SUCCESS) {
        return SetError(result, "Vulkan command buffer begin failed");
    }

    result = bridge_.RecordEvaluation(command_buffer_, resources);
    if (result == VK_SUCCESS) {
        result = vkEndCommandBuffer(command_buffer_);
    }
    if (result != VK_SUCCESS) {
        return SetError(result, "Vulkan DLSSG command recording failed");
    }

    result = bridge_.SubmitAndWait(command_buffer_);
    if (result != VK_SUCCESS) {
        return SetError(result, "Vulkan DLSSG queue submission failed");
    }
    return VK_SUCCESS;
}

}  // namespace dlssg::vulkan
