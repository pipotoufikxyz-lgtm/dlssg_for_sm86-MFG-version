# Native renderer foundation

This directory is the source-level extension point for rendering backends. It is
deliberately separate from the existing opaque DLL distribution.

## Configure

```powershell
cmake -S . -B build -DDLSSG_ENABLE_VULKAN=ON -DDLSSG_ENABLE_DX9=ON
cmake --build build --config Release
```

`find_package(Vulkan)` uses the standard Vulkan SDK discovery rules. Direct3D 9
uses the Windows SDK and is only enabled on Windows. If the Vulkan SDK is not
installed, configuration still succeeds and the factory reports Vulkan as
unavailable at runtime. No SDK or runtime DLL is copied into this repository.

The current implementation establishes instance/physical-device/logical-device
creation for Vulkan and adapter/device/presentation setup for DX9. Swapchain,
surface, resource, shader, and full frame synchronization work belongs behind
this interface and can now be added without modifying the packaged legacy path.
