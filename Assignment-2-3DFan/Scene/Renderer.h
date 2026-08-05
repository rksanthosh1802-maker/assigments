#pragma once

/**
 * Renderer.h
 *
 * Singleton renderer that owns all Model instances and drives the render loop.
 * Platform-agnostic – shared by Android, Desktop, and WebGL builds.
 *
 * Android-specific members (AAssetManager) are wrapped in PLATFORM_ANDROID guards.
 */

#include "Model.h"
#include "Platform.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer {
public:
    static Renderer& Instance() {
        static Renderer instance;
        return instance;
    }

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    // -----------------------------------------------------------------------
    // Platform-specific initialisation helpers
    // -----------------------------------------------------------------------
#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

    // Touch / mouse delegation
    void TouchEventDown(float x, float y);
    void TouchEventMove(float x, float y);
    void TouchEventRelease(float x, float y);

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    std::vector<Model*> models;
    int screenWidth  = 0;
    int screenHeight = 0;
};
