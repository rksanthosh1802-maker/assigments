@echo off
REM =============================================================================
REM build_web.bat – Build OpenGLES3-Primitives-Modern for WebGL using Emscripten
REM
REM Prerequisites:
REM   1. Install Emscripten SDK: https://emscripten.org/docs/getting_started/
REM   2. Run emsdk_env.bat in the same shell before this script, OR
REM      call it from here by editing the EMSDK_PATH below.
REM   3. Run this script from the project root.
REM
REM Output: web\index.html (+ index.js, index.wasm, index.data)
REM Serve:  cd web && python -m http.server 8080
REM
REM Windowing backend (choose one):
REM   GLFW  – default, recommended  (-DUSE_GLFW  -s USE_GLFW=3)
REM   SDL2  – legacy fallback        (no -DUSE_GLFW, -s USE_SDL=2)
REM =============================================================================

REM -- Optional: activate emsdk automatically --
REM SET EMSDK_PATH=C:\emsdk
REM CALL "%EMSDK_PATH%\emsdk_env.bat"

IF NOT EXIST web MKDIR web

REM --- GLFW backend (default) -----------------------------------------------
emcc -std=c++17 ^
     Scene/main.cpp ^
     Scene/Renderer.cpp ^
     Scene/Primitives.cpp ^
     -IScene ^
     -DUSE_GLFW ^
     -s USE_GLFW=3 ^
     -s USE_WEBGL2=1 ^
     -s FULL_ES3=1 ^
     -s WASM=1 ^
     -s ALLOW_MEMORY_GROWTH=1 ^
     --preload-file android/app/src/main/assets/shader@assets/shader ^
     -o web/index.html

REM --- SDL2 backend (opt-in) ------------------------------------------------
REM To use SDL2 instead, comment out the GLFW block above and uncomment below:
REM emcc -std=c++17 ^
REM      Scene/main.cpp ^
REM      Scene/Renderer.cpp ^
REM      Scene/Primitives.cpp ^
REM      -IScene ^
REM      -s USE_SDL=2 ^
REM      -s USE_WEBGL2=1 ^
REM      -s FULL_ES3=1 ^
REM      -s WASM=1 ^
REM      -s ALLOW_MEMORY_GROWTH=1 ^
REM      --preload-file android/app/src/main/assets/shader@assets/shader ^
REM      -o web/index.html

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
