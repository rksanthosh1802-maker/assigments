/**
 * @file    Platform.h
 * @brief   Single-header platform detection.Selects the right GL headers / log macros for
 *          Desktop (GLFW+GLEW), Web (Emscripten), and Android (NDK).
 * @author Santhosh Ravikumar
 * @date    2026
 */
#pragma once

// Emscripten must be checked before _WIN32 - emcc defines both on some setups.
#ifdef __EMSCRIPTEN__
    #ifndef PLATFORM_EMSCRIPTEN
    #define PLATFORM_EMSCRIPTEN
    #endif
#elif defined(_WIN32)
    #ifndef PLATFORM_WINDOWS
    #define PLATFORM_WINDOWS
    #endif
#elif defined(__ANDROID__)
    #ifndef PLATFORM_ANDROID
    #define PLATFORM_ANDROID
    #endif
#endif

#ifdef PLATFORM_EMSCRIPTEN
    #ifdef USE_GLFW
        #define GLFW_INCLUDE_ES3
        #include <GLFW/glfw3.h>
    #else
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

    #ifndef LOG_TAG
    #define LOG_TAG "AndroidProgrammingQuiz"
    #endif

    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif
