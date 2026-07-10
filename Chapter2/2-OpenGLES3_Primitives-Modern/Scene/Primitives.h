#pragma once

/**
 * Primitives.h
 *
 * Renders 2D primitives using OpenGL ES 3.0 VAO/VBO.
 * Touch/click cycles through 7 primitive types:
 *   GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP,
 *   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN
 *
 * Platform-agnostic – shared by Android, Desktop, and WebGL builds.
 * On Android an AAssetManager pointer is stored for shader loading;
 * on other platforms file-system loading is used via ShaderHelper.
 */

#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Primitives : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Primitives(AAssetManager* assetMgr);
#else
    Primitives();
#endif
    ~Primitives() override;

    void InitModel()                         override;
    void Render()                            override;

    void TouchEventDown(float x, float y)    override;
    void TouchEventMove(float x, float y)    override;
    void TouchEventRelease(float x, float y) override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program  = 0;
    GLuint vao      = 0;
    GLuint vboPos   = 0;
    GLuint vboColor = 0;

    // Current primitive type – cycles through GL_POINTS(0) … GL_TRIANGLE_FAN(6)
    GLenum primitive = GL_LINES;
};
