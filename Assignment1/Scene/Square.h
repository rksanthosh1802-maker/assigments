/**
 * @file    Square.h
 * @brief   Static yellow square, offset to the right of the triangle so both
 *          shapes are visible at once
 * @author  Santhosh Ravikumar
 * @date    2026
 */
#pragma once
#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Square : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Square(AAssetManager* assetMgr);
#else
    Square();
#endif
    ~Square() override;

    void InitModel()          override;
    void Render()              override;
    void Resize(int w, int h) override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint programID = 0;
    GLuint vao        = 0;
    GLuint vboPos      = 0;
};
