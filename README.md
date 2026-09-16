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

See [VULKAN_SUPPORT.md](VULKAN_SUPPORT.md) for the required implementation
work. Do not install this package in an RTX Remix game expecting frame
generation; use the game's existing Remix/DLSS integration until a Vulkan
backend is available.

**INSTALL FOR 3000S SERIES:**

1. Fully exit the game. Back up any existing mod proxy and INI outside the game folder.
2. Copy this package's `version.dll` and `dlssg_sm86.ini` beside the actual rendering EXE.
   If that DLL name is occupied, choose ONE original-name DLL from `altnative` that
   the game loads. Preserve other mods and the game's original DLSSG files. 
3. Start the game, enable frame generation, and select X6 if supported.

**INSTALL FOR 2000S SERIES:**

  Same Steps As 3000s series's install
