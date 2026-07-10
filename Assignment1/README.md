# AndroidProgrammingQuiz-CrossPlatformCI

Refactor of the Tutorial 3 single-file `Triangle` renderer into a proper
`Model` + `Renderer` architecture, built and run across Desktop, Android, and
Web, with a second shape (`Square`) added alongside the triangle.

## Project layout

```
AndroidProgrammingQuiz-CrossPlatformCI/
├── CMakeLists.txt              <- Desktop entry point (GLFW + GLEW)
├── script_build_and_run.bat    <- Desktop convenience wrapper
├── build_web.bat / .sh         <- Web entry point (Emscripten)
├── cmake/ImportDependencies.cmake
├── Scene/                      <- shared by all three platforms
│   ├── Platform.h, Model.h, Renderer.h/.cpp, ShaderHelper.h
│   ├── Triangle.h/.cpp, Square.h/.cpp
│   └── main.cpp                <- Desktop + Web entry point
├── android/                    <- Android Studio / Gradle project
├── build_desktop/              <- Desktop build output (gitignored)
├── web/                        <- Web build output (gitignored)
└── README.md
```

## Architecture

- `Model` (`Scene/Model.h`) is the lifecycle every shape implements:
  `InitModel()` once, `Render()` per frame, `Resize()` on surface change.
- `Renderer` (`Scene/Renderer.h/.cpp`) is a singleton that owns a
  `std::vector<Model*>` and is the only thing `main.cpp` /
  `NativeTemplate.cpp` talk to. It never includes a concrete shape header in
  its own header - only `Renderer.cpp` knows `Triangle` and `Square` exist.
- `ShaderHelper` (`Scene/ShaderHelper.h`) is a header-only, reusable place for
  shader compilation. It supports two ways of getting source text in:
  inline strings (`buildProgram`) and files on disk / APK assets
  (`buildProgramFromFile` / `buildProgramFromAssets`), both funneling into the
  same compile/link core.
- `Triangle` and `Square` are concrete `Model`s. Each owns its own GL
  program/VAO/VBO and loads its shader source from the `.glsl` files in
  `android/app/src/main/assets/shader/` via the file-based `ShaderHelper`
  path.

## Building

### Desktop (Windows)

```
script_build_and_run.bat
```

This configures + builds `build_desktop/` with CMake, then launches the
executable. Fetches GLFW and GLEW automatically via `FetchContent` on first
configure. Press `ESC` to quit.

### Android

Open the `android/` folder in Android Studio, let it sync, then Run on an
emulator or device. `nativeInit` converts the Java `AssetManager` to a native
`AAssetManager*` and hands it to `Renderer::Instance()` before initializing,
so the `.glsl` files can be read out of the APK.

### Web (Emscripten)

```
build_web.sh      # or build_web.bat on Windows
cd web
python3 -m http.server 8080
```

Then open `http://localhost:8080/` in a browser. Requires the Emscripten SDK
activated in the current shell (`emcc --version` should succeed).

## What to expect

A rotating RGB triangle centered in the viewport, with a static yellow square
offset to its right - both rendering simultaneously, on all three platforms,
from the same `Scene/` source.
