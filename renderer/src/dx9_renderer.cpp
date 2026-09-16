#include "renderer/renderer.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>

#include <string>

namespace dlssg::renderer {
namespace {

class Dx9Renderer final : public Renderer {
public:
    ~Dx9Renderer() override { shutdown(); }

    Backend backend() const noexcept override { return Backend::Direct3D9; }
    const char* backendName() const noexcept override { return "dx9"; }

    Result initialize(const NativeWindow& window, const RendererOptions&) override {
        if (window.handle == nullptr) {
            return Result::failure("DX9 requires a native HWND in NativeWindow::handle");
        }
        d3d_ = Direct3DCreate9(D3D_SDK_VERSION);
        if (d3d_ == nullptr) {
            return Result::failure("Direct3DCreate9 failed");
        }

        D3DPRESENT_PARAMETERS params{};
        params.Windowed = TRUE;
        params.SwapEffect = D3DSWAPEFFECT_DISCARD;
        params.hDeviceWindow = static_cast<HWND>(window.handle);
        params.BackBufferWidth = window.width;
        params.BackBufferHeight = window.height;
        params.BackBufferFormat = D3DFMT_X8R8G8B8;
        params.EnableAutoDepthStencil = TRUE;
        params.AutoDepthStencilFormat = D3DFMT_D24S8;

        const HRESULT result = d3d_->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            static_cast<HWND>(window.handle),
            D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &params,
            &device_);
        if (FAILED(result)) {
            shutdown();
            return Result::failure("IDirect3D9::CreateDevice failed with HRESULT " + std::to_string(result));
        }
        return Result::success();
    }

    Result resize(uint32_t width, uint32_t height) override {
        if (device_ == nullptr) return notInitialized();
        if (width == 0 || height == 0) return Result::success();
        return Result::failure("DX9 reset requires the host presentation parameters");
    }
    Result beginFrame() override {
        if (device_ == nullptr) return notInitialized();
        const HRESULT result = device_->TestCooperativeLevel();
        if (result == D3DERR_DEVICELOST) return Result::failure("DX9 device is lost");
        return SUCCEEDED(result) ? Result::success() : Result::failure("DX9 device requires reset");
    }
    Result endFrame() override {
        if (device_ == nullptr) return notInitialized();
        const HRESULT result = device_->Present(nullptr, nullptr, nullptr, nullptr);
        return SUCCEEDED(result) ? Result::success() : Result::failure("IDirect3DDevice9::Present failed");
    }

    void shutdown() noexcept override {
        if (device_ != nullptr) {
            device_->Release();
            device_ = nullptr;
        }
        if (d3d_ != nullptr) {
            d3d_->Release();
            d3d_ = nullptr;
        }
    }

private:
    static Result notInitialized() { return Result::failure("DX9 renderer is not initialized"); }
    IDirect3D9* d3d_ = nullptr;
    IDirect3DDevice9* device_ = nullptr;
};

} // namespace

std::unique_ptr<Renderer> createDx9Renderer() {
    return std::make_unique<Dx9Renderer>();
}

} // namespace dlssg::renderer
