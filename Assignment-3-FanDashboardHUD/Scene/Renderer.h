#pragma once

/**
 * Renderer.h
 *
 * Singleton renderer that owns the scene and drives the render loop.
 * Platform-agnostic - shared by Android, Desktop, and WebGL builds.
 *
 * Android Programming Quiz 3 change from Quiz 2:
 *   Instead of a flat std::vector<Model*> holding just the Fan, Renderer now
 *   owns exactly two things - a Scene3D and a SceneHUD - and calls them in
 *   the fixed order the handout specifies (Part 1):
 *     Clear -> Scene3D (depth ON) -> disable depth -> SceneHUD -> enable depth
 *   Touch/click routing (TouchEventDown) now goes to SceneHUD first; only a
 *   miss (no button hit) falls through to Scene3D::PickAt (Part 5 -> Part 8
 *   hand-off), so a HUD click can never also trigger a 3D pick.
 *
 *   The PUBLIC interface below is unchanged from Quiz 2 on purpose, so
 *   main.cpp, the Android JNI bridge (NativeTemplate.cpp), and
 *   MainActivity.java all keep working without modification.
 */

#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D;
class SceneHUD;

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
    void clearModels();
    void syncHudToScene(); // pushes SceneHUD's speed/zoom values into Scene3D

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Scene3D*  scene3D  = nullptr;
    SceneHUD* sceneHUD = nullptr;

    int screenWidth  = 0;
    int screenHeight = 0;
};
