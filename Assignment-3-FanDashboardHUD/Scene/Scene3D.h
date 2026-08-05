/*!
@file    	Scene3D.h
@date    	Android Programming Quiz 3

Declares Scene3D: owns the Fan (from Quiz 2) plus the new Ground (Part 3).
Forwards Resize() to both, builds the shared Phong light + adjustable camera
distance (Part 7) once per frame and pushes it into both via
SetViewProjection/SetLight, and owns PickAt() (Part 8) - 3D picking via
TransformUnproject -> world-space ray -> closest-hit against the Fan's own
cached part positions, the same technique the Scene Graph PickAt() examples
use, applied to this fan's own geometry/hierarchy.

*//*__________________________________________________________________________*/
#pragma once

#include "Platform.h"
#include "Model.h"
#include "Transform.h"
#include "Fan.h"
#include "Ground.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Scene3D(AAssetManager* assetMgr);
#else
    Scene3D();
#endif
    ~Scene3D() override;

    void InitModel() override;
    void Render()    override;
    void Resize(int w, int h) override;

    // Called by Renderer, fed from SceneHUD's zoom buttons (Part 7).
    void SetCameraDistance(float d);
    float CameraDistance() const { return cameraDistance; }

    // Called by Renderer, fed from SceneHUD's speed buttons (Part 6).
    void SetFanSpeed(float speed);

    // Part 8: returns true if the click landed on a fan part (and highlights
    // it). Renderer only calls this when SceneHUD::TouchDown already missed
    // for this click (a HUD button consumes the event first).
    bool PickAt(float screenX, float screenY);

private:
    glm::mat4 buildViewMatrix(); // uses cameraTransform's VIEW_MATRIX stack; not const

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    Fan*    fan    = nullptr;
    Ground* ground = nullptr;

    Transform cameraTransform; // used only to build the shared view/projection matrices

    float cameraDistance = 8.0f;
    static constexpr float kMinCameraDistance = 3.0f;
    static constexpr float kMaxCameraDistance = 20.0f;

    int screenWidth = 1280, screenHeight = 720;
    glm::mat4 projection{ 1.0f };

    glm::vec3 lightPosWorld{ 5.0f, 8.0f, 5.0f };
    glm::vec3 lightColor{ 1.0f, 1.0f, 1.0f };
};
