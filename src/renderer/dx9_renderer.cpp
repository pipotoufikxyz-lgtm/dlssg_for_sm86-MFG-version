#include "renderer/dx9_renderer.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <d3d9.h>
#include <cstring>
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

void* Dx9Renderer::CreateBuffer(const BufferDesc& desc, std::string& error) {
#if defined(_WIN32)
    if (!device_ || desc.size == 0) { error = "Invalid Direct3D 9 buffer request"; return nullptr; }
    DWORD usage = 0;
    D3DPOOL pool = D3DPOOL_MANAGED;
    if (desc.usage == BufferUsage::Vertex) usage |= D3DUSAGE_WRITEONLY;
    if (desc.usage == BufferUsage::Index) usage |= D3DUSAGE_WRITEONLY;
    HRESULT result;
    if (desc.usage == BufferUsage::Index) {
        IDirect3DIndexBuffer9* index_buffer = nullptr;
        result = device_->CreateIndexBuffer(static_cast<UINT>(desc.size), usage,
            D3DFMT_INDEX32, pool, &index_buffer, nullptr);
        if (FAILED(result)) { error = "Direct3D 9 buffer creation failed"; return nullptr; }
        if (desc.initial_data) {
            void* mapped = nullptr;
            if (FAILED(index_buffer->Lock(0, 0, &mapped, 0))) {
                index_buffer->Release(); error = "Direct3D 9 buffer upload failed"; return nullptr;
            }
            std::memcpy(mapped, desc.initial_data, desc.size);
            index_buffer->Unlock();
        }
        return index_buffer;
    } else {
        IDirect3DVertexBuffer9* vertex_buffer = nullptr;
        result = device_->CreateVertexBuffer(static_cast<UINT>(desc.size), usage, 0,
            pool, &vertex_buffer, nullptr);
        if (FAILED(result)) { error = "Direct3D 9 buffer creation failed"; return nullptr; }
        if (desc.initial_data) {
            void* mapped = nullptr;
            if (FAILED(vertex_buffer->Lock(0, 0, &mapped, 0))) {
                vertex_buffer->Release(); error = "Direct3D 9 buffer upload failed"; return nullptr;
            }
            std::memcpy(mapped, desc.initial_data, desc.size);
            vertex_buffer->Unlock();
        }
        return vertex_buffer;
    }
#else
    error = "Direct3D 9 is only available on Windows"; return nullptr;
#endif
}

void* Dx9Renderer::CreateTexture(const TextureDesc& desc, std::string& error) {
#if defined(_WIN32)
    if (!device_ || desc.width == 0 || desc.height == 0) {
        error = "Invalid Direct3D 9 texture request"; return nullptr;
    }
    IDirect3DTexture9* texture = nullptr;
    HRESULT result = device_->CreateTexture(desc.width, desc.height, desc.mip_levels,
        desc.render_target ? D3DUSAGE_RENDERTARGET : 0, D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED, &texture, nullptr);
    if (FAILED(result)) { error = "Direct3D 9 texture creation failed"; return nullptr; }
    if (desc.initial_data) {
        D3DLOCKED_RECT locked{};
        if (FAILED(texture->LockRect(0, &locked, nullptr, 0))) {
            texture->Release(); error = "Direct3D 9 texture upload failed"; return nullptr;
        }
        const auto* source = static_cast<const uint8_t*>(desc.initial_data);
        size_t pitch = desc.initial_row_pitch ? desc.initial_row_pitch : desc.width * 4;
        for (uint32_t y = 0; y < desc.height; ++y)
            std::memcpy(static_cast<uint8_t*>(locked.pBits) + y * locked.Pitch,
                        source + y * pitch, desc.width * 4);
        texture->UnlockRect(0);
    }
    return texture;
#else
    error = "Direct3D 9 is only available on Windows"; return nullptr;
#endif
}

void* Dx9Renderer::CreateShader(const ShaderDesc& desc, std::string& error) {
#if defined(_WIN32)
    if (!device_ || !desc.bytecode || desc.bytecode_size == 0) {
        error = "Direct3D 9 shader bytecode is required"; return nullptr;
    }
    IDirect3DVertexShader9* shader = nullptr;
    if (FAILED(device_->CreateVertexShader(static_cast<const DWORD*>(desc.bytecode),
                                           &shader))) {
        error = "Direct3D 9 vertex shader creation failed"; return nullptr;
    }
    return shader;
#else
    error = "Direct3D 9 is only available on Windows"; return nullptr;
#endif
}

void Dx9Renderer::DestroyResource(void* value) noexcept {
#if defined(_WIN32)
    if (value) static_cast<IUnknown*>(value)->Release();
#else
    (void)value;
#endif
}

}  // namespace renderer
