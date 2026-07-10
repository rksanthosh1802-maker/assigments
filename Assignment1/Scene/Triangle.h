/**
 * @file    Triangle.h
 * @brief   Rotating RGB triangle. Concrete Model - nothing outside Renderer.cpp
 *          is allowed to know this class exists.
 * @author  Santhosh Ravikumar
 * @date    2026
 */
#pragma once
#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Triangle : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Triangle(AAssetManager* assetMgr);
#else
    Triangle();
#endif
    ~Triangle() override;

    void InitModel()          override;
    void Render()              override;
    void Resize(int w, int h) override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint programID   = 0;
    GLuint vao          = 0;
    GLuint vboPos        = 0;
    GLuint vboColor      = 0;
    GLint  uRadianAngle  = -1;
    float  degree        = 0.0f;
};
