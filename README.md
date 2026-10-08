
                                                            
                                                            
  

  - DLSSG MODIFED VERSION

                                                          
 ----------------------------------------------- -----------------------------------------------


  **NEW RELEASE 2.8.5**


-------------------------------------------------------------


UPDATE: **WORKS BOTH ON 20 SERIES AND 30 SERIES. no need to install two different managers anymore.**

**Picture quality (PSNR, higher is better) went from 30.30 to around 40 dB.**

New update:


| Scene | Fix24 wrong px | Fix28 wrong px | New wrong px | New vs Fix24 | PSNR Fix24 → New |
| :--- | ---: | ---: | ---: | :---: | :---: |
| Parallax foliage | 25,619 | 16,178 | 14,591 | −43% | 21.0 → 23.9 dB |
| Antialiased/translucent HUD + crosshair | 16,641 | 11,309 | 10,396 | −38% | 26.8 → 28.3 dB |
| Character + blade, camera orbit | 13,470 | 4,830 | 3,766 | −72% | 26.1 → 33.2 dB |
| Pole + wire crossing a still view | 13,339 | 13,716 | 11,836 | −11% | 22.7 → 25.1 dB |
| HUD text over pan | 11,178 | 6,800 | 5,816 | −48% | 24.0 → 26.5 dB |
| Fast vertical pan (44 px) | 10,960 | 8,245 | 7,794 | −29% | 31.1 → 32.2 dB |
| Pan with exposure change | 8,808 | 3,182 | 2,954 | −66% | 33.9 → 37.1 dB |
| Sword hilts over camera turn | 6,907 | 2,611 | 1,390 | −80% | 33.1 → 38.1 dB |
| Diagonal pan (half-pixel) | 6,747 | 4,593 | 4,225 | −37% | 31.3 → 32.8 dB |
| Helmet crest over starfield | 4,919 | 3,090 | 2,078 | −58% | 28.2 → 33.2 dB |
| Camera pan | 4,306 | 1,120 | 606 | −86% | 32.0 → 41.2 dB |
| Roofs over sky | 1,656 | 258 | 142 | −91% | 41.8 → 49.3 dB |
| **Total** | **124,550** | **75,932** | **65,594** | **−47%** | |
| **Mean PSNR** | **29.33 dB** | **31.90 dB** | **33.41 dB** | **+4.07 dB** | |
| **Mean abs. error** | **0.0108** | **0.0073** | **0.0065** | **−40%** | |




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
