#!/usr/bin/env bash
# =============================================================================
# build_web.sh - Build the 3D Fan for WebGL using Emscripten (Linux/macOS/CI)
#
# Prerequisites:
#   1. Install the Emscripten SDK: https://emscripten.org/docs/getting_started/
#   2. Source emsdk_env.sh in the same shell before this script.
#   3. Run this script from the project root.
#
# Output: web/index.html (+ index.js, index.wasm, index.data)
# Serve:  cd web && python3 -m http.server 8080
# =============================================================================

set -euo pipefail

mkdir -p web

# GLM is header-only and is not bundled with Emscripten - fetch it once.
if [ ! -d external/glm ]; then
    echo "Fetching GLM ..."
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git external/glm
fi

emcc -std=c++17 \
     Scene/main.cpp \
     Scene/Renderer.cpp \
     Scene/Transform.cpp \
     Scene/Fan.cpp \
     -IScene \
     -Iexternal/glm \
     -DUSE_GLFW \
     -s USE_GLFW=3 \
     -s USE_WEBGL2=1 \
     -s MIN_WEBGL_VERSION=2 \
     -s FULL_ES3=1 \
     -s WASM=1 \
     --preload-file android/app/src/main/assets/shader@assets/shader \
     -o web/index.html

echo
echo "Build succeeded."
echo "Run:  cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"