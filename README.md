# dlssg_for_sm86-MFG-version
i added x5 and x6 to the sm86 and sm75 since it didnt have any. 

**FOR DOWNLOADS CHECK RELEASES**

## API compatibility

The shipped DLL package still supports the DirectX loader path only. It does
**not** support Vulkan or RTX Remix games. Standalone Vulkan bridge and route
sources are now included for integration work, but they are not wired into the
prebuilt proxy DLLs. The bundled NVIDIA runtime contains
`NVSDK_NGX_VULKAN_*` exports, but the injected SM75 backend and proxy do not
implement Vulkan resource import, synchronization, or presentation. Adding a
`Vulkan` setting to `dlssg_sm86.ini` therefore has no effect.

The source package includes a versioned C ABI in
`src/vulkan_route_abi.h` (`DlssgVulkan_GetRouteApi`) for a future proxy or
backend DLL to connect Remix's Vulkan lifecycle and NGX Vulkan callbacks.
The verified route DLL is available at `vulkan/dlssg_vulkan_route.dll`; it
does not replace or modify the existing proxy DLLs. The companion
`vulkan/dlssg_vulkan_proxy.dll` consumes that route ABI and forwards
device/reset/evaluate calls from a caller-provided NGX adapter.

## Renderer backends

The source tree also provides a selectable backend library:

- `renderer::Backend::Vulkan` creates a Win32 Vulkan instance, selects a
  graphics/present queue pair, creates a device, swapchain, image views,
  command pool, frame fence, semaphores, resize path, and frame
  acquire/submit/present lifecycle.
- `renderer::Backend::Direct3D9` creates a hardware-accelerated D3D9 device,
  reports adapter information, handles lost-device checks, reset on resize,
  and BeginScene/EndScene/Present.

Use `renderer::ParseBackend("vulkan")` or
`renderer::ParseBackend("dx9")`, then pass the result to
`renderer::CreateRenderer`. The existing backend remains an explicit
`Backend::Existing` option and fails clearly because no existing application
renderer source is present in this repository.

The renderer interface also exposes backend-owned `CreateBuffer`,
`CreateTexture`, `CreateShader`, and `DestroyResource` operations. Vulkan
expects SPIR-V shader bytecode and allocates Vulkan device memory/image views;
Direct3D 9 accepts compiled D3D9 shader bytecode and creates managed buffers
and textures. Resource creation errors are returned to the caller instead of
being silently downgraded.

See [VULKAN_SUPPORT.md](VULKAN_SUPPORT.md) for the required implementation
work. Do not install this package in an RTX Remix game expecting frame
generation; use the game's existing Remix/DLSS integration until a Vulkan
backend is available.

## Vulkan integration installer

After building or obtaining the three DLLs in `vulkan\`, run the installer
with either the game's binary directory or the rendering executable:

```bat
install.bat "C:\Path\To\Game\bin"
install.bat "C:\Path\To\Game\bin\game.exe"
```

The script validates the target and package files, backs up any existing
`dlssg_vulkan_*.dll` files into a random-named backup directory, and copies
the route, proxy, and NGX loader DLLs. If the game is installed under
`Program Files`, run the script from an Administrator command prompt. It
intentionally does not replace `version.dll`, `dinput8.dll`,
`vulkan-1.dll`, or any proprietary game files.

The installed DLLs are integration components, not an automatic RTX Remix
hook. A host must load `dlssg_vulkan_proxy.dll`, provide the Vulkan/NGX adapter
callbacks, and connect Remix's device and swapchain lifecycle. This repository
does not contain the proprietary Remix hook contract, so copying these files
alone cannot enable frame generation in an RTX Remix game.

When the loader-layer DLL is included, the installer also registers its
manifest under the current user's Vulkan implicit-layer registry key. This
makes the layer discoverable by Vulkan applications and logs instance, device,
swapchain, and present lifecycle events to `dlssg_vulkan_layer.log` beside the
game executable. Remove the registration with:

```bat
reg delete "HKCU\Software\Khronos\Vulkan\ImplicitLayers" /v "C:\Path\To\Game\bin\dlssg_vulkan_layer.json" /f
```

**INSTALL FOR 3000S SERIES:**

1. Fully exit the game. Back up any existing mod proxy and INI outside the game folder.
2. Copy this package's `version.dll` and `dlssg_sm86.ini` beside the actual rendering EXE.
   If that DLL name is occupied, choose ONE original-name DLL from `altnative` that
   the game loads. Preserve other mods and the game's original DLSSG files. 
3. Start the game, enable frame generation, and select X6 if supported.

**INSTALL FOR 2000S SERIES:**

  Same Steps As 3000s series's install
