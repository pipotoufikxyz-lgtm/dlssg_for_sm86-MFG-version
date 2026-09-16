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

class Renderer {
public:
    virtual ~Renderer() = default;
    virtual bool Initialize(const CreateInfo&, std::string& error) = 0;
    virtual bool Resize(uint32_t width, uint32_t height, std::string& error) = 0;
    virtual bool BeginFrame(std::string& error) = 0;
    virtual bool EndFrame(std::string& error) = 0;
    virtual void Shutdown() noexcept = 0;
    virtual const DeviceInfo& Device() const noexcept = 0;
};

std::unique_ptr<Renderer> CreateRenderer(const CreateInfo&, std::string& error);
Backend ParseBackend(const std::string& value);
const char* BackendName(Backend) noexcept;

}  // namespace renderer
