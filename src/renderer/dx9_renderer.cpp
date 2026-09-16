#include "renderer/dx9_renderer.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <d3d9.h>
#endif

namespace renderer {

Dx9Renderer::~Dx9Renderer() { Shutdown(); }

bool Dx9Renderer::Initialize(const CreateInfo& info, std::string& error) {
#if defined(_WIN32)
    HWND window = static_cast<HWND>(info.native_window);
    if (window == nullptr) { error = "Direct3D 9 requires a Win32 HWND"; return false; }
    api_ = Direct3DCreate9(D3D_SDK_VERSION);
    if (!api_) { error = "Direct3DCreate9 failed"; return false; }
    D3DADAPTER_IDENTIFIER9 identifier{};
    if (FAILED(api_->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &identifier))) {
        error = "Could not query the Direct3D 9 adapter";
        return false;
    }
    parameters_.Windowed = TRUE;
    parameters_.SwapEffect = D3DSWAPEFFECT_DISCARD;
    parameters_.hDeviceWindow = window;
    parameters_.BackBufferFormat = D3DFMT_UNKNOWN;
    parameters_.BackBufferWidth = info.width;
    parameters_.BackBufferHeight = info.height;
    parameters_.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    HRESULT result = api_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &parameters_, &device_);
    if (FAILED(result)) {
        error = "Direct3D 9 device creation failed";
        return false;
    }
    device_info_.name = identifier.Description;
    device_info_.api = "Direct3D 9";
    device_info_.vendor_id = identifier.VendorId;
    device_info_.device_id = identifier.DeviceId;
    return true;
#else
    error = "Direct3D 9 is only available on Windows";
    return false;
#endif
}

bool Dx9Renderer::Resize(uint32_t width, uint32_t height, std::string& error) {
#if defined(_WIN32)
    if (!device_) { error = "Direct3D 9 device is not initialized"; return false; }
    parameters_.BackBufferWidth = width;
    parameters_.BackBufferHeight = height;
    HRESULT result = device_->Reset(&parameters_);
    if (FAILED(result)) { error = "Direct3D 9 device reset failed"; return false; }
    return true;
#else
    error = "Direct3D 9 is only available on Windows"; return false;
#endif
}

bool Dx9Renderer::BeginFrame(std::string& error) {
#if defined(_WIN32)
    HRESULT result = device_->TestCooperativeLevel();
    if (result == D3DERR_DEVICENOTRESET) return Resize(parameters_.BackBufferWidth,
                                                        parameters_.BackBufferHeight, error);
    if (FAILED(result)) { error = "Direct3D 9 device is lost"; return false; }
    result = device_->BeginScene();
    if (FAILED(result)) { error = "IDirect3DDevice9::BeginScene failed"; return false; }
    return true;
#else
    error = "Direct3D 9 is only available on Windows"; return false;
#endif
}

bool Dx9Renderer::EndFrame(std::string& error) {
#if defined(_WIN32)
    if (FAILED(device_->EndScene())) { error = "IDirect3DDevice9::EndScene failed"; return false; }
    HRESULT result = device_->Present(nullptr, nullptr, nullptr, nullptr);
    if (result == D3DERR_DEVICELOST) { error = "Direct3D 9 device lost during Present"; return false; }
    if (FAILED(result)) { error = "IDirect3DDevice9::Present failed"; return false; }
    return true;
#else
    error = "Direct3D 9 is only available on Windows"; return false;
#endif
}

void Dx9Renderer::Shutdown() noexcept {
#if defined(_WIN32)
    if (device_) device_->Release();
    if (api_) api_->Release();
    device_ = nullptr; api_ = nullptr;
#endif
}

}  // namespace renderer
