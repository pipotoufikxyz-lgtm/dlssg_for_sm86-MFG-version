#pragma once

#include "renderer/renderer.hpp"

#if defined(_WIN32)
#include <d3d9.h>
#endif

namespace renderer {

class Dx9Renderer final : public Renderer {
public:
    ~Dx9Renderer() override;
    bool Initialize(const CreateInfo&, std::string&) override;
    bool Resize(uint32_t, uint32_t, std::string&) override;
    bool BeginFrame(std::string&) override;
    bool EndFrame(std::string&) override;
    void Shutdown() noexcept override;
    const DeviceInfo& Device() const noexcept override { return device_info_; }
    void* CreateBuffer(const BufferDesc&, std::string&) override;
    void* CreateTexture(const TextureDesc&, std::string&) override;
    void* CreateShader(const ShaderDesc&, std::string&) override;
    void DestroyResource(void*) noexcept override;

private:
#if defined(_WIN32)
    IDirect3D9* api_ = nullptr;
    IDirect3DDevice9* device_ = nullptr;
    D3DPRESENT_PARAMETERS parameters_{};
#endif
    DeviceInfo device_info_;
};

}  // namespace renderer
