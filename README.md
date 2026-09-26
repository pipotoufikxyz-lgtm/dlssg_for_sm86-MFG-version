
                                                            
                                                            
  

  - DLSSG MODIFED VERSION

**im looking for testers who have 20 series. add me on discord: kamonix_12_44321**

                                                          
 ----------------------------------------------- -----------------------------------------------


  **NEW RELEASE 2.8.2**

  Dont't forget to suggest features in discussion tab
-------------------------------------------------------------


UPDATE: **ADDED SMOOTH MOTION SUPPORT FOR 617.14 DRIVER. (wasn't working)**

<img width="1907" height="952" alt="image" src="https://github.com/user-attachments/assets/5fd0586e-4433-4a3e-9943-7158718869bb" />


-------------------------------------------------------------

To Activate it: activate it through Nvidia Profile Inspector


**To Use Nvidia profile inspector:**

**-Open NVIDIA Profile Inspector on your computer.**

**-Locate Smooth Motion**

**-Enable it and Save**

**FOR DOWNLOADS CHECK RELEASES**

--------------------------------------------------------------------------

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

If the release archive is beside `install.bat`, the installer also offers the
bundled SM75 X5/X6 package. That package installs the actual legacy
`version.dll`, `dinput8.dll`, `dlssg_sm86.ini`, and pinned `runtime` files
after backing up existing copies. This is the path that can expose the DLSSG
option in supported DX12 games; it is not a Vulkan Remix hook.

When the loader-layer DLL is included, the installer also registers its
manifest under the current user's Vulkan implicit-layer registry key. This
makes the layer discoverable by Vulkan applications and logs instance, device,
swapchain, and present lifecycle events to `dlssg_vulkan_layer.log` beside the
game executable. Remove the registration with:

```bat
reg delete "HKCU\Software\Khronos\Vulkan\ImplicitLayers" /v "C:\Path\To\Game\bin\dlssg_vulkan_layer.json" /f
```

**INSTALL FOR 3000S SERIES:**

<img width="1516" height="493" alt="image" src="https://github.com/user-attachments/assets/5b754f4d-e8d0-4c87-bc09-1c3114fc08a9" />

1. Fully exit the game. Back up any existing mod proxy and INI outside the game folder.
2. Copy this package's `version.dll` and `dlssg_sm86.ini` beside the actual rendering EXE.
   If that DLL name is occupied, choose ONE original-name DLL from `altnative` that
   the game loads. Preserve other mods and the game's original DLSSG files. 
3. Start the game, enable frame generation, and select X6 if supported.

-------------------------------------------------------------

**INSTALL FOR 2000S SERIES:**

  Same Steps As 3000s series's install

  -------------------------------------------------------------


Credits: sdii1995 for dlssg.
