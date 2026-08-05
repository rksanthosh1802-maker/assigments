/*!
@file    	Fan.h
@author  	r.santhosh@digipen.edu
@date    	13/07/2026

Declares the Fan model: a 3D table fan built from one cube mesh, drawn seven
times with different model matrices on the Transform matrix stack. Each part
is tinted by its own PARTCOLOR uniform.

Android Programming Quiz 3 changes from Quiz 2:
  - TouchEventDown/Move/Release's tap-to-toggle / drag-to-boost gesture code
    is REMOVED (Scene3D/SceneHUD own input now). fanOn / dragBoost /
    kTapThreshold are gone, not just disabled.
  - Speed is now driven every frame by Scene3D (fed from SceneHUD's +SPD/-SPD
    buttons) via SetSpeed(), instead of a fixed kBaseSpeed.
  - Cube gained a per-corner normal attribute (replacing the old per-corner
    "Shade" gradient) for real per-fragment Phong shading (Part 2).
  - The camera pull-back that used to be baked into this class's model matrix
    now lives in Scene3D as an adjustable camera distance (Part 7); Fan only
    receives the resulting view/projection matrices from Scene3D.
  - Each part's world-space position/extent is cached every Render() call so
    Scene3D::PickAt (Part 8) can hit-test base/pole/hub/each blade without
    recomputing the hierarchy transforms itself.

*//*__________________________________________________________________________*/
#pragma once

#include "Platform.h"
#include "Model.h"
#include "Transform.h"

#include <array>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

// Identifies which part of the fan a pick or highlight refers to.
enum class FanPart {
    None   = -1,
    Base   = 0,
    Pole   = 1,
    Hub    = 2,
    Blade0 = 3,
    Blade1 = 4,
    Blade2 = 5,
    Blade3 = 6,
    Count  = 7
};

// One pickable part's world-space bounding info, refreshed every Render().
// Base/Pole/Hub are box-ish -> tested as a bounding sphere by Scene3D.
// Blades are thin, elongated boxes -> tested as a capsule (segment + radius) -
// the handout's note that a single center-point test is too imprecise for them.
struct FanPickablePart {
    FanPart id = FanPart::None;
    bool    isSegment = false;
    glm::vec3 center{};          // used when isSegment == false
    float     radius = 0.0f;
    glm::vec3 segA{}, segB{};    // used when isSegment == true
};

class Fan : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetMgr);
#else
    Fan();
#endif
    ~Fan() override;

    void InitModel() override;
    void Render()    override;
    void Resize(int w, int h) override;

    // Called by Scene3D once per frame, before Render() (Part 1/2/7).
    void SetViewProjection(const glm::mat4& view, const glm::mat4& proj);
    void SetLight(const glm::vec3& eyeSpaceLightPos, const glm::vec3& lightColor);

    // Called by Scene3D, fed from SceneHUD::CurrentSpeed() (Part 6).
    void SetSpeed(float degreesPerFrame) { speed = degreesPerFrame; }

    // Part 8: highlight state, set by Scene3D::PickAt.
    void SetHighlightedPart(FanPart part) { highlighted = part; }
    FanPart HighlightedPart() const { return highlighted; }

    const std::array<FanPickablePart, (size_t)FanPart::Count>& PickableParts() const { return pickables; }
    static const char* PartName(FanPart part);

private:
    void drawPart(FanPart id, float r, float g, float b);
    void spinAngleAdvance();

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0, vao = 0, vbo = 0, ibo = 0;
    GLint  uMVP = -1, uMV = -1, uNormalMatrix = -1, uPartColor = -1;
    GLint  uLightPosEye = -1, uLightColor = -1;

    Transform transform;              // provided matrix stack (model matrix only now - Part 7)

    float speed = 8.0f;                // degrees/frame, driven by SceneHUD via Scene3D (Part 6)
    float spinAngle = 0.0f;            // current blade angle (degrees)
    FanPart highlighted = FanPart::None;

    glm::vec3 lightPosEye{ 0.0f, 5.0f, 5.0f };
    glm::vec3 lightColor{ 1.0f, 1.0f, 1.0f };

    std::array<FanPickablePart, (size_t)FanPart::Count> pickables{};
};
