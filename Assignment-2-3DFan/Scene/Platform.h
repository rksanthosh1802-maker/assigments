#pragma once

/**
 * Platform.h
 *
 * Single-header platform detection for the TouchEvents demo.
 * Supports four targets:
 *   PLATFORM_EMSCRIPTEN  – WebGL 2.0 via Emscripten + GLFW (default) or SDL2
 *   PLATFORM_WINDOWS     – OpenGL (ES compat) via GLFW + GLEW
 *   PLATFORM_ANDROID     – OpenGL ES 3.0 via Android NDK
 *   PLATFORM_IOS         – OpenGL ES 3.0 via Apple EAGLContext (iOS 12+)
 *
 * IMPORTANT: Emscripten must be detected BEFORE Windows because emcc
 * defines both __EMSCRIPTEN__ AND _WIN32 on some toolchain setups.
 * Apple must be detected BEFORE the generic fallback.
 */

// ---------------------------------------------------------------------------
// Platform detection
// ---------------------------------------------------------------------------
#ifdef __EMSCRIPTEN__
    #define PLATFORM_EMSCRIPTEN
#elif defined(_WIN32)
    #define PLATFORM_WINDOWS
#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IOS
        #define PLATFORM_IOS
    #endif
#endif

// ---------------------------------------------------------------------------
// Platform-specific OpenGL and system headers
// ---------------------------------------------------------------------------
#ifdef PLATFORM_EMSCRIPTEN
    #ifdef USE_GLFW
        // GLFW path – Emscripten provides GLFW when built with -s USE_GLFW=3.
        // GLFW_INCLUDE_ES3 tells glfw3.h to pull in <GLES3/gl3.h> automatically.
        #define GLFW_INCLUDE_ES3
        #include <GLFW/glfw3.h>
    #else
        // SDL2 path – built with -s USE_SDL=2
        // WebGL 2.0 is exposed via the GLES3 header in Emscripten
        #include <GLES3/gl3.h>
        #include <SDL2/SDL.h>
    #endif
    #include <emscripten.h>
    #include <cstdio>
    #include <cstring>

    #define LOGI(...) do { printf("[INFO] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGE(...) do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGD(...) do { printf("[DEBUG] " __VA_ARGS__); printf("\n"); } while(0)

#elif defined(PLATFORM_WINDOWS)
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
    #include <cstdio>
    #include <cstring>

    #define LOGI(...) do { printf("[INFO] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGE(...) do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGD(...) do { printf("[DEBUG] " __VA_ARGS__); printf("\n"); } while(0)

#elif defined(PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
    #include <android/log.h>
    #include <cstring>

    // LOG_TAG must be defined in each .cpp file before including Platform.h
    #ifndef LOG_TAG
    #define LOG_TAG "TouchEvents"
    #endif

    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#elif defined(PLATFORM_IOS)
    // OpenGL ES is deprecated in iOS 12+ but remains fully functional on iOS 17/18.
    // Suppress the deprecation warnings — this is intentional ES 3.0 teaching code.
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>
    #pragma clang diagnostic pop
    #include <cstdio>
    #include <cstring>

    #define LOGI(...) do { printf("[INFO] "); printf(__VA_ARGS__); printf("\n"); } while(0)
    #define LOGE(...) do { printf("[ERROR] "); printf(__VA_ARGS__); printf("\n"); } while(0)
    #define LOGD(...) do { printf("[DEBUG] "); printf(__VA_ARGS__); printf("\n"); } while(0)
#endif
