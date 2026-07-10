@echo off
REM =============================================================================
REM build_web.bat - Build AndroidProgrammingQuiz for WebGL (Emscripten/Windows)
REM
REM Prerequisites:
REM   1. Install the Emscripten SDK: https://emscripten.org/docs/getting_started/
REM   2. Activate emsdk in this shell before running, OR edit EMSDK_PATH below.
REM   3. Run from the project root.
REM
REM Output: web\index.html (+ index.js, index.wasm, index.data)
REM Serve:  cd web && python -m http.server 8080
REM =============================================================================

REM SET EMSDK_PATH=C:\emsdk
REM CALL "%EMSDK_PATH%\emsdk_env.bat"

IF NOT EXIST web MKDIR web

emcc -std=c++17 ^
     Scene/main.cpp ^
     Scene/Renderer.cpp ^
     Scene/Triangle.cpp ^
     Scene/Square.cpp ^
     -IScene ^
     -DUSE_GLFW ^
     -s USE_GLFW=3 ^
     -s USE_WEBGL2=1 ^
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
