#include "renderer/renderer.hpp"

#include "renderer/dx9_renderer.hpp"
#include "renderer/vulkan_renderer.hpp"

#include <algorithm>
#include <cctype>

namespace renderer {

const char* BackendName(Backend backend) noexcept {
    switch (backend) {
    case Backend::Vulkan: return "vulkan";
    case Backend::Direct3D9: return "dx9";
    default: return "existing";
    }
}

Backend ParseBackend(const std::string& value) {
    std::string normalized = value;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (normalized == "vulkan" || normalized == "vk") return Backend::Vulkan;
    if (normalized == "dx9" || normalized == "direct3d9") return Backend::Direct3D9;
    return Backend::Existing;
}

std::unique_ptr<Renderer> CreateRenderer(const CreateInfo& info,
                                         std::string& error) {
    switch (info.backend) {
    case Backend::Vulkan: return std::make_unique<VulkanRenderer>();
    case Backend::Direct3D9: return std::make_unique<Dx9Renderer>();
    default:
        error = "The existing renderer is not supplied by this repository";
        return nullptr;
    }
}

}  // namespace renderer
