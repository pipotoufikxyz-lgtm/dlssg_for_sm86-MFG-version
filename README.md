# dlssg_for_sm86-MFG-version

DLSS Frame Generation (DLSSG) mod for RTX 20-series (SM75) and RTX 30-series (SM86) GPUs, with Multi-Frame Generation (MFG) support and **dedicated Low-Latency & High-Quality profiles**.

**FOR DOWNLOADS CHECK RELEASES (or `SM75-X5-X6-experimental.zip`)**

---

## ⚡ Profiles Included in this Mod

| Variant Folder | Multiplier | Generated Frames | Latency Profile | Visual Quality | Best Use Case |
| --- | --- | --- | --- | --- | --- |
| **`SM75-X2-LowLatency`** | **X2** | **1** | **Lowest (DLSS 3 Baseline)** | **Maximum (Pristine, 0 ghosting)** | Competitive / Fast-Paced / Lowest Lag |
| **`SM75-X3-Quality`** | **X3** | **2** | **Low (Balanced)** | **High (Smooth, minimal artifacts)** | High Refresh Gaming / Balanced |
| **`SM75-X5-experimental`** | **X5** | **4** | Higher | Experimental Multi-frame | High Multiplier Exploration |
| **`SM75-X6-experimental`** | **X6** | **5** | Highest | Experimental Multi-frame | Max Multiplier Demonstration |

---

## 🛠️ Optimizations Applied to Lower Latency & Improve Quality

1. **Zero-Overhead Logging:** Frame-by-frame disk logging (`Level=0`, `File=0`, `EvaluateEvery=0`) is disabled across all configurations by default. This eliminates synchronous disk write stalls, micro-stutters, and frame-time latency spikes during gameplay.
2. **Reduced Display Pipeline Queue:** The `SM75-X2-LowLatency` and `SM75-X3-Quality` profiles eliminate the 4-to-5 frame display buffer delay of high multipliers, slashing input latency by up to 50–100ms+.
3. **Pristine Optical Flow Quality:** Lower multipliers prevent accumulated interpolation drift, warping, HUD smearing, and ghosting artifacts.

---

## 🚀 Installation

### For RTX 20-Series (SM75) & RTX 30-Series (SM86):

1. **Fully exit the game.** Back up any existing mod proxy DLLs, INI, and runtime folders outside the game directory.
2. Choose your variant from the package:
   - For **lowest latency and best visual quality**: choose **`SM75-X2-LowLatency`**.
   - For **balanced extra fluidity**: choose **`SM75-X3-Quality`**.
   - For **extreme frame rate multiplier**: choose **`SM75-X5-experimental`** or **`SM75-X6-experimental`**.
3. Copy:
   - `version.dll`
   - `dinput8.dll`
   - `dlssg_sm86.ini`
   - `runtime/` folder (containing `nvngx_dlssg.dll` and `sm75_backend.dll`)
   directly beside the game's actual rendering executable (e.g., `bin\x64_dx12` for Witcher 3 or Cyberpunk 2077).
4. Clear old cached bundles before launching:
   ```cmd
   rmdir /s /q "%LOCALAPPDATA%\DlssgSm86\bundles"
   ```
5. Launch the game, enable Frame Generation in the display / graphics settings.

---

## ⚙️ Recommended In-Game & Driver Settings

- **NVIDIA Reflex:** Set to **On + Boost** in-game. This keeps GPU clocks pinned and prevents render queue latency.
- **DLSS Super Resolution:** Set to **Quality** (or **DLAA**). Avoid "Performance" or "Ultra Performance", as higher base resolution provides sharper motion vectors for the optical flow interpolator.
- **Base Framerate:** Aim for at least 50–60 real FPS before enabling Frame Generation.
- **G-Sync & V-Sync:**
  - NVIDIA Control Panel: **G-Sync = Enabled**, **Vertical Sync = On**, **Max Frame Rate = 3-4 FPS below refresh rate** (e.g. 141 FPS on 144Hz).
  - In-game: **Vertical Sync = Off**.
