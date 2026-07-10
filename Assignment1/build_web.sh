#!/usr/bin/env sh
# =============================================================================
# build_web.sh - Build AndroidProgrammingQuiz for WebGL using Emscripten
#
# Prerequisites:
#   1. Install the Emscripten SDK: https://emscripten.org/docs/getting_started/
#   2. Activate it:  source /path/to/emsdk/emsdk_env.sh
#   3. Run from the project root: ./build_web.sh
#
# Output: web/index.html (+ index.js, index.wasm, index.data)
# Serve:  cd web && python3 -m http.server 8080
# =============================================================================

set -e

mkdir -p web

emcc -std=c++17 \
    Scene/main.cpp \
    Scene/Renderer.cpp \
    Scene/Triangle.cpp \
    Scene/Square.cpp \
    -IScene \
    -DUSE_GLFW \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s FULL_ES3=1 \
    -s WASM=1 \
    --preload-file android/app/src/main/assets/shader@assets/shader \
    -o web/index.html

echo ""
echo "Build succeeded."
echo "Run:  cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"
