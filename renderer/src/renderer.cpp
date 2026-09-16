#include "renderer/renderer.h"

#include <utility>

namespace dlssg::renderer {

std::unique_ptr<Renderer> createVulkanRenderer();
std::unique_ptr<Renderer> createDx9Renderer();

class UnavailableRenderer final : public Renderer {
public:
    explicit UnavailableRenderer(Backend backend) : backend_(backend) {}

    Backend backend() const noexcept override { return backend_; }
    const char* backendName() const noexcept override { return dlssg::renderer::backendName(backend_); }

    Result initialize(const NativeWindow&, const RendererOptions&) override {
        return Result::failure(std::string(backendName()) + " backend is not available in this build");
    }
    Result resize(uint32_t, uint32_t) override { return Result::failure("Renderer is not initialized"); }
    Result beginFrame() override { return Result::failure("Renderer is not initialized"); }
    Result endFrame() override { return Result::failure("Renderer is not initialized"); }
    void shutdown() noexcept override {}

private:
    Backend backend_;
};

#ifndef DLSSG_HAS_VULKAN
std::unique_ptr<Renderer> createVulkanRenderer() {
    return std::make_unique<UnavailableRenderer>(Backend::Vulkan);
}
#endif

#ifndef DLSSG_HAS_DX9
std::unique_ptr<Renderer> createDx9Renderer() {
    return std::make_unique<UnavailableRenderer>(Backend::Direct3D9);
}
#endif

const char* backendName(Backend backend) noexcept {
    switch (backend) {
    case Backend::Legacy: return "legacy";
    case Backend::Vulkan: return "vulkan";
    case Backend::Direct3D9: return "dx9";
    }
    return "unknown";
}

std::unique_ptr<Renderer> createRenderer(Backend backend) {
    switch (backend) {
    case Backend::Vulkan: return createVulkanRenderer();
    case Backend::Direct3D9: return createDx9Renderer();
    case Backend::Legacy:
    default: return std::make_unique<UnavailableRenderer>(backend);
    }
}

std::vector<Backend> compiledBackends() {
    return {Backend::Legacy, Backend::Vulkan, Backend::Direct3D9};
}

} // namespace dlssg::renderer
