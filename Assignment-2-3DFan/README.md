# Assignment 2 — 3D Fan

A 3D table fan (base, pole, hub, four spinning blades) rendered with OpenGL ES 3.0.
The whole fan is drawn from **one** 8-vertex indexed cube, submitted seven times with
different model matrices composed on the provided `Transform` matrix stack, and tinted
per part through a `PARTCOLOR` uniform — one `glUniform3fv` per draw call.

One shared C++17 code base in `Scene/` builds for three targets: **Windows desktop**,
**Android**, and **WebGL**.

**Live WebGL build:** <https://r.santhosh.github.io/Assignment-2-3DFan/>

## Part hierarchy

```
          [B3]
            |
   [B2]--(hub)--[B0]     4 blades, 90 deg apart, spinning about the hub's Z axis
            |
          [B1]
            |
          (pole)         static
            |
          (base)         static
```

Base, pole, and hub are static. Only the four blades rotate, driven by `spinAngle`
inside their own push/pop blocks.

## Controls

| Gesture | Effect |
| --- | --- |
| Tap / click | Toggles the fan ON / OFF. Logs `Fan ON` / `Fan OFF`. |
| Swipe / click-drag | Boosts the spin speed in proportion to the drag velocity (measured with `std::chrono`, clamped). Releasing returns the fan to its base speed. |
| `ESC` | Quits (desktop only). |

The log line appears in the console on desktop, the browser console on web, and in
logcat under the tag `Fan3D` on Android (`adb logcat -s Fan3D`).

## Project layout

```
Assignment-2-3DFan/
├── .github/workflows/      CI/CD: desktop.yaml, android.yaml, pages.yaml
├── CMakeLists.txt          Desktop entry point (GLFW + GLEW + GLM)
├── cmake/                  FetchContent dependency setup
├── script_build_and_run.bat
├── build_web.sh / .bat     Web entry point (Emscripten)
├── Scene/                  Shared C++ — Fan.h/.cpp is the assignment
├── android/                Android Studio / Gradle project
└── README.md
```

Shaders live in `android/app/src/main/assets/shader/` and are shared by all three
targets: CMake copies them next to the desktop executable, Emscripten packs them into
the virtual filesystem, and Android reads them from the APK.

---

## Desktop (Windows)

**Prerequisites**
- Visual Studio 2022 (Desktop development with C++)
- CMake 3.15+ on `PATH`
- Internet access — GLFW, GLEW and GLM are downloaded automatically on the first configure

**Build and run**
```bat
script_build_and_run.bat
```

Or manually:
```bat
cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64
cmake --build build_desktop --config Debug
cd build_desktop\Debug
Fan3D.exe
```

The first configure downloads the dependencies and takes a few minutes.

**Output:** `build_desktop\Debug\Fan3D.exe`

---

## Web (WebGL 2.0 via Emscripten)

**Prerequisites**
- The Emscripten SDK, activated in the current shell:
```bat
C:\emsdk\emsdk_env.bat
```
  (on Linux/macOS: `source /path/to/emsdk/emsdk_env.sh`)
- Git on `PATH` — GLM is cloned into `external/glm` on the first build

**Build**
```bat
build_web.bat
```
```sh
./build_web.sh
```

**Run**
```sh
cd web
python -m http.server 8080
```
Then open <http://localhost:8080/>.

**Output:** `web/index.html` (plus `index.js`, `index.wasm`, `index.data`)

---

## Android

**Prerequisites**
- Android Studio
- SDK Platform 36, NDK 27, CMake 3.22.1 (install via SDK Manager)
- A device or emulator running API 34+ with OpenGL ES 3.0

**Build and run**
1. Open the `android/` folder in Android Studio (not the repo root).
2. Let Gradle sync — it fetches GLM through CMake `FetchContent` on the first native build.
3. Press **Run**.

Or from the command line:
```sh
cd android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

**Output:** `android/app/build/outputs/apk/debug/app-debug.apk`

---

## CI/CD

Every push to `main` triggers three GitHub Actions workflows:

- **`desktop.yaml`** — builds with CMake on Windows and Linux in parallel.
- **`android.yaml`** — builds the APK and uploads it as a downloadable artifact.
- **`pages.yaml`** — compiles the Emscripten build and deploys it to GitHub Pages.

The Pages source is set to **GitHub Actions** in the repository settings.