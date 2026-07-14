/*!
@file    	Fan.h
@author  	r.santhosh@digipen.edu
@date    	13/07/2026

Declares the Fan model: a 3D table fan built from one 8-vertex indexed cube,
drawn seven times with different model matrices on the Transform matrix stack.
Each part is tinted by its own PARTCOLOR uniform.

*//*__________________________________________________________________________*/
#pragma once
/**
 * Fan.h
 *
 * A 3D table fan built from ONE 8-vertex indexed cube, drawn seven times with
 * different model matrices composed on the provided Transform matrix stack.
 * Each part is tinted by its own PARTCOLOR uniform (one glUniform3fv per draw).
 *
 * Part hierarchy (all parts share the same world transform: pull-back + Y tilt):
 *
 *          [B3]
 *            |
 *   [B2]--(hub)--[B0]     blades: rotate about the hub's Z axis, 90 deg apart,
 *            |            driven by spinAngle
 *          [B1]
 *            |
 *           pole          pole connects hub to base
 *            |
 *        __base__         base sits on the table
 *
 *   world : T(0, 0.8, -8) -> R(20 deg, Y)
 *   base  : push -> T(0,-2.60,0)    -> S(1.60, 0.25, 0.80) -> draw(brown)     -> pop
 *   pole  : push -> T(0,-1.21,0)    -> S(0.15, 2.53, 0.15) -> draw(gray)      -> pop
 *   hub   : push -> T(0, 0.20,0)    -> S(0.30, 0.30, 0.30) -> draw(dark gray) -> pop
 *   blade : push -> T(0, 0.20,0.15) -> R(spinAngle + i*90 deg, Z)
 *                -> T(0, 0.55,0)    -> S(0.22, 0.80, 0.05) -> draw(colour[i]) -> pop
 *
 * Gestures (touch on Android, left mouse button on desktop/web):
 *   TAP   - down + release with movement below kTapThreshold: toggles the fan
 *           ON/OFF and logs "Fan ON" / "Fan OFF".
 *   SWIPE - while dragging, the drag velocity (pixels per millisecond, timed
 *           with std::chrono) is converted into a clamped speed boost, so a
 *           fast flick spins the blades visibly faster. Releasing clears the
 *           boost and the fan returns to its base speed.
 */


#include "Platform.h"
#include "Model.h"
#include "Transform.h"

#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

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

    void TouchEventDown(float x, float y)    override;
    void TouchEventMove(float x, float y)    override;
    void TouchEventRelease(float x, float y) override;

private:
    void drawPart(float r, float g, float b);

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0, vao = 0, vbo = 0, ibo = 0;
    GLint  uMVP = -1, uPartColor = -1;

    Transform transform;              // provided matrix stack (model/view/projection)

    float spinAngle = 0.0f;           // current blade angle (degrees)
    bool  fanOn     = true;           // toggled by a tap
    float dragBoost = 0.0f;           // extra speed while dragging, 0 otherwise

    static constexpr float kBaseSpeed    =  1.5f;   // deg/frame when ON
    static constexpr float kMaxBoost     = 20.0f;   // clamp for the drag boost
    static constexpr float kBoostScale   =  8.0f;   // px/ms -> deg/frame
    static constexpr float kTapThreshold = 12.0f;   // px: below this it's a tap

    float lastX = 0.0f, lastY = 0.0f; // previous touch position
    float movedDistance = 0.0f;       // total movement since touch-down
    std::chrono::steady_clock::time_point lastMoveTime;
};