#pragma once

#include "vulkan_support.hpp"

#include <string>

namespace dlssg::vulkan {

class FeatureAdapter {
public:
    virtual ~FeatureAdapter() = default;

    virtual VkResult Initialize(const DeviceContext& context) = 0;
    virtual VkResult Evaluate(VkCommandBuffer command_buffer,
                              const EvaluationResources& resources) = 0;
    virtual void Shutdown() = 0;
};

class Route final {
public:
    Route() = default;
    ~Route();

    Route(const Route&) = delete;
    Route& operator=(const Route&) = delete;

    VkResult Enable(const DeviceContext& context, FeatureAdapter& adapter);
    void Disable();
    VkResult Reset(const DeviceContext& context);

    VkResult Evaluate(EvaluationResources resources);
    bool Enabled() const noexcept { return enabled_; }
    const char* LastError() const noexcept { return last_error_.c_str(); }

private:
    VkResult SetError(VkResult result, const char* message);
    void ClearState(bool shutdown_adapter);

    DeviceContext context_{};
    FeatureAdapter* adapter_ = nullptr;
    Bridge bridge_{};
    VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
    bool enabled_ = false;
    std::string last_error_;
};

}  // namespace dlssg::vulkan
