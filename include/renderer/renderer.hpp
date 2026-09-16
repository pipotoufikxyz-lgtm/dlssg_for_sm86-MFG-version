#pragma once

#include <cstdint>
#include <memory>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace renderer {

enum class Backend { Existing, Vulkan, Direct3D9 };

struct CreateInfo {
    Backend backend = Backend::Existing;
    void* native_window = nullptr;
    uint32_t width = 1280;
    uint32_t height = 720;
    bool validation = false;
};

struct DeviceInfo {
    std::string name;
    std::string api;
    uint32_t vendor_id = 0;
    uint32_t device_id = 0;
};

enum class BufferUsage { Vertex, Index, Uniform, Storage };
struct BufferDesc {
    size_t size = 0;
    BufferUsage usage = BufferUsage::Vertex;
    const void* initial_data = nullptr;
};
struct TextureDesc {
    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t mip_levels = 1;
    bool render_target = false;
    const void* initial_data = nullptr;
    size_t initial_row_pitch = 0;
};
struct ShaderDesc {
    const void* bytecode = nullptr;
    size_t bytecode_size = 0;
};

class Renderer {
public:
    virtual ~Renderer() = default;
    virtual bool Initialize(const CreateInfo&, std::string& error) = 0;
    virtual bool Resize(uint32_t width, uint32_t height, std::string& error) = 0;
    virtual bool BeginFrame(std::string& error) = 0;
    virtual bool EndFrame(std::string& error) = 0;
    virtual void Shutdown() noexcept = 0;
    virtual const DeviceInfo& Device() const noexcept = 0;
    virtual void* CreateBuffer(const BufferDesc&, std::string&) = 0;
    virtual void* CreateTexture(const TextureDesc&, std::string&) = 0;
    virtual void* CreateShader(const ShaderDesc&, std::string&) = 0;
    virtual void DestroyResource(void*) noexcept = 0;
};

std::unique_ptr<Renderer> CreateRenderer(const CreateInfo&, std::string& error);
Backend ParseBackend(const std::string& value);
const char* BackendName(Backend) noexcept;

}  // namespace renderer
