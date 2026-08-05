/*!
@file    	SceneHUD.h
@date    	Android Programming Quiz 3

Declares SceneHUD: four buttons (+SPD, -SPD, +ZOOM, -ZOOM) and a 20-block
speed bar, all flat-coloured quads/triangles drawn with an orthographic
projection built via the provided Transform::TransformOrtho (Part 4).

SceneHUD owns no 3D state itself - it owns the *values* (speed, requested
camera distance) that Renderer reads every frame and pushes into Scene3D,
matching the handout's "HUD reports input, scene owns behaviour" split.

*//*__________________________________________________________________________*/
#pragma once

#include "Platform.h"
#include "Model.h"
#include "Transform.h"
#include <array>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

enum class HudButton { PlusSpeed = 0, MinusSpeed = 1, PlusZoom = 2, MinusZoom = 3, Count = 4 };

struct HudRect { float x, y, w, h; }; // top-left origin, pixels

class SceneHUD : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager* assetMgr);
#else
    SceneHUD();
#endif
    ~SceneHUD() override;

    void InitModel() override;
    void Render()    override;
    void Resize(int w, int h) override;

    // Returns true (and consumes the event) if (x,y) hit a button.
    // Renderer must route touch/click to this BEFORE 3D picking (Part 8) -
    // a HUD hit must never fall through to Scene3D::PickAt.
    bool TouchDown(float x, float y);
    void TouchRelease();

    float CurrentSpeed()          const { return speed; }
    float CurrentCameraDistance() const { return cameraDistance; }

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0;
    GLuint buttonsVao = 0, buttonsVbo = 0;
    GLuint barVao = 0, barVbo = 0;
    GLint  uProj = -1;

    Transform transform; // used only to build the screen-space ortho projection

    int screenW = 1280, screenH = 720;
    glm::mat4 ortho{ 1.0f };

    std::array<HudRect, (size_t)HudButton::Count> buttonRects{};
    HudRect barOrigin{};

    float speed = 8.0f;            // 0..20, Part 6
    float cameraDistance = 8.0f;   // Part 7, same clamp range as Scene3D

    void rebuildButtonGeometry();
    void rebuildBarGeometry();
    bool pointInRect(float x, float y, const HudRect& r) const;

    static constexpr int   kBarSlots  = 20;
    static constexpr float kBarSlotW  = 10.0f;
    static constexpr float kBarSlotH  = 20.0f;
};
