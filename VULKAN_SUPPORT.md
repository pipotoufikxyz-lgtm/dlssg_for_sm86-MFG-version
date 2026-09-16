# Vulkan support status

Vulkan support is not present in the current release. This is especially
important for RTX Remix titles: Remix presents the game's original API through
its own runtime and the final rendering path is Vulkan, so the existing
D3D12-oriented injection path cannot be enabled by changing the INI file.

## What was verified

- `runtime/nvngx_dlssg.dll` exports the NGX Vulkan entry points, including
  `NVSDK_NGX_VULKAN_Init`, `NVSDK_NGX_VULKAN_CreateFeature`, and
  `NVSDK_NGX_VULKAN_EvaluateFeature`.
- The external `runtime/sm75_backend.dll` is the selected backend for this
  package and is tied to the D3D12/SM86 bridge. It does not provide a Vulkan
  resource or queue implementation.
- `dlssg_sm86.ini` has no Vulkan mode. Unknown keys are not a Vulkan adapter;
  adding one would silently leave the D3D12 route unchanged.

## Required implementation

A real RTX Remix implementation needs a source-level Vulkan backend and proxy
changes, not another binary capability patch:

1. Add a Vulkan route to the proxy and backend ABI, including instance,
   physical-device, logical-device, queue, and command-buffer ownership.
2. Map Vulkan images and buffers to the NGX Vulkan feature descriptors,
   preserving format, extent, usage, and memory lifetime.
3. Insert the required image-layout transitions and queue synchronization
   around every evaluation and generated-frame readback.
4. Handle Remix device recreation, swapchain changes, reset, alt-tab, and
   frame-generation enable/disable without retaining stale Vulkan handles.
5. Keep the SM75 capability and frame-count patches independent from the API
   route, then validate X2-X6 on real RTX Remix titles for correctness,
   pacing, latency, and device-removal behavior.
6. Add an explicit API selection setting only after the loader has a working
   Vulkan route; reject Vulkan startup with a visible log error until the
   required backend is installed.

The repository now includes a standalone `src/vulkan_support.*` bridge and
`src/vulkan_route.*` route. The route owns synchronization objects, records
the required image transitions, supports device reset, and delegates the
actual NGX evaluation through `FeatureAdapter`. The adapter is intentionally
an integration boundary: the proprietary NGX Vulkan ABI and the existing
proxy are not available as source here.

The C ABI is declared in `src/vulkan_route_abi.h` and exported by
`DlssgVulkan_GetRouteApi`. It is versioned (`DLSSG_VULKAN_ABI_VERSION`), uses
opaque route handles, validates every struct size/version, and exposes:

- `create` / `destroy` for route ownership;
- `reset` for Remix device recreation;
- `evaluate` for one frame's input/output/flow/depth images;
- `last_error` for visible startup and runtime failures.

The adapter callbacks are the NGX boundary: the proxy supplies
`initialize`, `evaluate`, and optional `shutdown` callbacks that call the
matching `NVSDK_NGX_VULKAN_*` functions. No C++ ABI or STL type crosses the
DLL boundary.

Build it with a Vulkan SDK installed:

```sh
cmake -S . -B build
cmake --build build
```

This produces a reusable `dlssg_vulkan_route.dll`, not a replacement for the
proprietary `version.dll` or `dinput8.dll`. A proxy must still instantiate
`Route`, provide a `FeatureAdapter` backed by
`NVSDK_NGX_VULKAN_*`, and connect Remix's device/swapchain lifecycle before
the shipped DLL package can claim Vulkan support.

The verified route DLL is included under `vulkan/` with its SHA-256 checksum.
Install it only beside a proxy that consumes `DlssgVulkan_GetRouteApi`; the
existing `version.dll` and `dinput8.dll` do not load this route automatically.
The existing proprietary proxy DLLs remain unchanged.

`src/vulkan_proxy.*` now provides that consumer as a separate Windows DLL.
`DlssgVulkanProxy_Create` loads the route DLL, negotiates the versioned ABI,
creates the route with the caller's NGX adapter callbacks, and forwards reset
and evaluation calls. It intentionally does not hook a game's exports or
pretend to implement the proprietary NGX Vulkan adapter.

`src/vulkan_ngx_loader.*` implements the safe part of the missing NGX layer:
it dynamically loads `nvngx_dlssg.dll`, verifies the five required Vulkan
exports, tracks device creation and swapchain changes, and rejects stale or
cross-device lifecycle events. It deliberately does not cast or call the
proprietary NGX function signatures, which are not shipped in this repository;
the caller must provide the correctly versioned `FeatureAdapter` callbacks.

## Public integration sources reviewed

Two public projects provide useful reference implementations, but neither is
a drop-in RTX Remix adapter:

- [`thierbig/bg3fgvk`](https://github.com/thierbig/bg3fgvk) is MIT-licensed and
  implements Vulkan DLSS-G for Baldur's Gate 3. Its `fgvk.dll` is loaded by
  BG3's Native Mod Loader, hooks BG3's DLSS Super Resolution call to obtain
  depth, motion vectors, jitter, and the HUD-less color, then feeds NVIDIA
  Streamline's DLSS-G plugin. Its hooks and resource recipe are game-specific.
- [`NVIDIAGameWorks/bridge-remix`](https://github.com/NVIDIAGameWorks/bridge-remix)
  is MIT-licensed but deprecated and is a 32-bit D3D9-to-64-bit bridge. It
  does not expose a Vulkan DLSS-G interception path. NVIDIA points users to
  [`dxvk-remix`](https://github.com/NVIDIAGameWorks/dxvk-remix) for the current
  Remix runtime.

The BG3 project confirms the missing work is an actual interception layer:
hook the Remix Vulkan dispatch/swapchain, identify the final color, motion
vectors, depth, jitter, and frame timing, then submit those resources through
Streamline or the exact NGX Vulkan ABI. This repository has no Remix runtime
hook contract or reliable way to identify those game resources, so copying
BG3's hooks would not enable RTX Remix and could corrupt unrelated Vulkan
calls. The packaged route/proxy/loader remains an integration component, not a
drop-in frame-generation mod.

The packaged pair is:

- `vulkan/dlssg_vulkan_route.dll`
- `vulkan/dlssg_vulkan_proxy.dll`

Keep both files together. The proxy loads the route DLL by name (or by the
explicit path passed to `DlssgVulkanProxy_Create`) and reports load, ABI, and
route errors through `DlssgVulkanProxy_LastError`.
