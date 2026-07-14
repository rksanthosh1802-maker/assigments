@echo off
REM =============================================================================
REM build_web.bat - Build the 3D Fan for WebGL using Emscripten (Windows)
REM
REM Prerequisites:
REM   1. Install the Emscripten SDK: https://emscripten.org/docs/getting_started/
REM   2. Run emsdk_env.bat in the same shell before this script.
REM   3. Run this script from the project root.
REM
REM Output: web\index.html (+ index.js, index.wasm, index.data)
REM Serve:  cd web && python -m http.server 8080
REM =============================================================================

IF NOT EXIST web MKDIR web

REM GLM is header-only and is not bundled with Emscripten - fetch it once.
IF NOT EXIST external\glm (
    echo Fetching GLM ...
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git external/glm
    IF %ERRORLEVEL% NEQ 0 ( echo Failed to fetch GLM. & pause & exit /b 1 )
)

emcc -std=c++17 ^
     Scene/main.cpp ^
     Scene/Renderer.cpp ^
     Scene/Transform.cpp ^
     Scene/Fan.cpp ^
     -IScene ^
     -Iexternal/glm ^
     -DUSE_GLFW ^
     -s USE_GLFW=3 ^
     -s USE_WEBGL2=1 ^
     -s MIN_WEBGL_VERSION=2 ^
     -s FULL_ES3=1 ^
     -s WASM=1 ^
     --preload-file android/app/src/main/assets/shader@assets/shader ^
     -o web/index.html

IF %ERRORLEVEL% NEQ 0 (
    echo Build FAILED.
    pause
    exit /b 1
)

echo.
echo Build succeeded.
echo Run:  cd web ^&^& python -m http.server 8080
echo Open: http://localhost:8080/
pause