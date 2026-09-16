#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace dlssg::renderer {

enum class Backend {
    Legacy,
    Vulkan,
    Direct3D9,
};

struct NativeWindow {
    void* handle = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct RendererOptions {
    Backend backend = Backend::Legacy;
    bool enableValidation = false;
};

struct Result {
    bool ok = false;
    std::string message;

    static Result success() { return {true, {}}; }
    static Result failure(std::string error) { return {false, std::move(error)}; }
};

class Renderer {
public:
    virtual ~Renderer() = default;
    virtual Backend backend() const noexcept = 0;
    virtual const char* backendName() const noexcept = 0;
    virtual Result initialize(const NativeWindow&, const RendererOptions&) = 0;
    virtual Result resize(uint32_t width, uint32_t height) = 0;
    virtual Result beginFrame() = 0;
    virtual Result endFrame() = 0;
    virtual void shutdown() noexcept = 0;
};

std::unique_ptr<Renderer> createRenderer(Backend backend);
const char* backendName(Backend backend) noexcept;
std::vector<Backend> compiledBackends();

} // namespace dlssg::renderer
