# dlssg_for_sm86-MFG-version
i added x5 and x6 to the sm86 and sm75 since it didnt have any. 

**FOR DOWNLOADS CHECK RELEASES**

**INSTALL FOR 3000S SERIES:**

1. Fully exit the game. Back up any existing mod proxy and INI outside the game folder.
2. Copy this package's `version.dll` and `dlssg_sm86.ini` beside the actual rendering EXE.
   If that DLL name is occupied, choose ONE original-name DLL from `altnative` that
   the game loads. Preserve other mods and the game's original DLSSG files. 
3. Start the game, enable frame generation, and select X6 if supported.

**INSTALL FOR 2000S SERIES:**

  Same Steps As 3000s series's install

---

## ⚡ Tuning for Lower Latency & Better Quality

The default experimental configuration is tuned for maximum multiplier capability (X5/X6) and diagnostic logging. For the lowest input lag and best visual quality during normal gameplay, apply the following optimizations:

### 1. Disable Frame-by-Frame Logging (Fixes Stutter & Disk Latency)
The default `dlssg_sm86.ini` logs every single frame (`EvaluateEvery=1` at `Level=3`), which creates massive disk I/O, micro-stutters, and frame-time latency. Edit `dlssg_sm86.ini`:
```ini
[Logging]
Level=0          ; 0 = Off, 1 = Errors only
File=0           ; Disable writing to disk
DebugOutput=0
EvaluateEvery=0  ; Disable per-frame evaluation logging
```

### 2. Lower the Frame Generation Multiplier (X2 or X3)
Multi-frame generation (X5/X6) introduces significant input lag because multiple frames must be queued and interpolated between real frames. Multiple intermediate frames also degrade image quality (warping, ghosting, edge distortion).
- **Best latency & visual quality:** Set `MaxGeneratedFrames=1` (X2 - 1 generated frame per real frame).
- **Balanced smoothness & latency:** Set `MaxGeneratedFrames=2` (X3).

In `dlssg_sm86.ini`:
```ini
[FrameGeneration]
MaxGeneratedFrames=1
```

### 3. In-Game & Driver Settings
- **NVIDIA Reflex:** Set to **On + Boost** in-game. This empties the render queue and keeps GPU clock speeds pinned.
- **DLSS Super Resolution:** Set to **Quality** (or **DLAA** if base FPS is high enough). Avoid "Performance" or "Ultra Performance", as higher base resolution produces cleaner motion vectors for Frame Generation.
- **Base Framerate:** Aim for a base framerate of at least 50–60 real FPS before enabling Frame Generation.
- **G-Sync + V-Sync + Frame Cap:**
  - In NVIDIA Control Panel: Set **G-Sync = Enabled**, **Vertical Sync = On**, and **Max Frame Rate = 3-4 FPS below refresh rate** (e.g., 141 FPS for 144Hz, 237 FPS for 240Hz).
  - In Game Settings: Set **Vertical Sync = Off**.

### 4. Clear Cache
Clear old cached bundles before testing new configurations:
```
%LOCALAPPDATA%\DlssgSm86\bundles
```

