/*!
@file    	Ground.h
@author  	(Quiz 3 addition, built on the Fan.h/Transform.h framework)
@date    	Android Programming Quiz 3

Declares Ground: a single large quad positioned under the fan. The
checkerboard pattern is computed entirely in the fragment shader from
object-space XZ position (Part 3) - no texture, no image asset - and lit
with the same eye-space Phong function the Fan uses (Part 2), fed through
the shared RenderContext values Scene3D passes to both.

*//*__________________________________________________________________________*/
#pragma once

#include "Platform.h"
#include "Model.h"
#include "Transform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager* assetMgr);
#else
    Ground();
#endif
    ~Ground() override;

    void InitModel() override;
    void Render()    override;
    void Resize(int w, int h) override;

    void SetViewProjection(const glm::mat4& view, const glm::mat4& proj);
    void SetLight(const glm::vec3& eyeSpaceLightPos, const glm::vec3& lightColor);

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0, vao = 0, vbo = 0, ibo = 0;
    GLint  uMVP = -1, uMV = -1, uNormalMatrix = -1;
    GLint  uLightPosEye = -1, uLightColor = -1;

    Transform transform;

    glm::vec3 lightPosEye{ 0.0f, 5.0f, 5.0f };
    glm::vec3 lightColor{ 1.0f, 1.0f, 1.0f };
};
