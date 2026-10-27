
                                                            
                                                            
  

  - DLSSG MODIFED VERSION

                                                          
 ----------------------------------------------- -----------------------------------------------


  **NEW RELEASE 2.8.5**        if you have a 20 series, please dm me on discord: kamonix_12_44321


-------------------------------------------------------------


UPDATE: **WORKS BOTH ON 20 SERIES AND 30 SERIES. no need to install two different managers anymore.**


New update:


Image quality: more stable interpolation maths and fewer reproduced artifacts. It fixes motion banding (FP32 instead of half precision), harsh transitions, and unwanted colour shifts when the optional features are off.

Optional HUD processing: corrections are more limited and the HUD fallback works as intended. The optional features stay off by default, and moving or transparent HUD elements can still show artifacts.

Performance: less setup and processing overhead, plus safer handling of CUDA contexts, GPU identity and resources. It doesn't promise more FPS or lower latency overall.

Vulkan and startup: sturdier Vulkan initialization and teardown, fixes for row alignment, buffer limits and Present semaphores, and better DXGI and version.dll proxy startup.

Other: rejects unverified resource combinations, fixes the CreateSwapChainForComposition hook, and keeps existing user configuration.




-------------------------------------------------------------

To Activate it: activate it through Nvidia Profile Inspector


**To Use Nvidia profile inspector:**

**-Open NVIDIA Profile Inspector on your computer.**

**-Locate Smooth Motion**

**-Enable it and Save**

**FOR DOWNLOADS CHECK RELEASES**

--------------------------------------------------------------------------

**DLSSG INSTALL FOR 3000S SERIES:**

<img width="1516" height="493" alt="image" src="https://github.com/user-attachments/assets/5b754f4d-e8d0-4c87-bc09-1c3114fc08a9" />

1. Fully exit the game. Back up any existing mod proxy and INI outside the game folder.
2. Copy this package's `version.dll` and `dlssg_sm86.ini` beside the actual rendering EXE.
   If that DLL name is occupied, choose ONE original-name DLL from `altnative` that
   the game loads. Preserve other mods and the game's original DLSSG files. 
3. Start the game, enable frame generation, and select X6 if supported.

-------------------------------------------------------------

**DLSSG INSTALL FOR 2000S SERIES:**

  Same Steps As 3000s series's install

  -------------------------------------------------------------


Credits: sdii1995 for dlssg.
