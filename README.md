# dlssg_for_sm86-MFG-version
i added x5 and x6 to the sm86 and sm75 since it didnt have any. 

**FOR DOWNLOADS CHECK RELEASES**

## Native renderer extension point

The repository now includes a source-level renderer foundation under `renderer/`
with a CMake build. It preserves the existing packaged DLL workflow while
providing explicit Vulkan and Direct3D 9 backend entry points for future
resource, shader, swapchain, and frame-synchronization work.

Configure and build on Windows with:

```powershell
cmake -S . -B build -DDLSSG_ENABLE_VULKAN=ON -DDLSSG_ENABLE_DX9=ON
cmake --build build --config Release
```

Vulkan is discovered through the installed Vulkan SDK. Direct3D 9 uses the
Windows SDK. Neither proprietary runtime DLLs nor SDK files are copied into
the repository.

**INSTALL FOR 3000S SERIES:**

1. Fully exit the game. Back up any existing mod proxy and INI outside the game folder.
2. Copy this package's `version.dll` and `dlssg_sm86.ini` beside the actual rendering EXE.
   If that DLL name is occupied, choose ONE original-name DLL from `altnative` that
   the game loads. Preserve other mods and the game's original DLSSG files. 
3. Start the game, enable frame generation, and select X6 if supported.

**INSTALL FOR 2000S SERIES:**

  Same Steps As 3000s series's install
