/**
 * @file    Renderer.h
 * @brief   Singleton that owns every Model in the scene and drives its lifecycle.
 *          Only talks to shapes through the Model interface - it never includes
 *          a concrete shape header. main.cpp / NativeTemplate.cpp only ever
 *          talk to Renderer::Instance(), never to a shape directly.
 * @author  Santhosh Ravikumar
 * @date    2026
 */
#pragma once
#include "Model.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer {
public:
    static Renderer& Instance();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();       // constructs every Model this scene needs
    void initializeModels();   // calls InitModel() on each one
    void clearModels();        // deletes and empties the collection

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    std::vector<Model*> models;
};
