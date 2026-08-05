/*!
@file    	Scene3D.cpp
@date    	Android Programming Quiz 3
*//*__________________________________________________________________________*/
#define LOG_TAG "Fan3D"
#include "Scene3D.h"

#include <algorithm>
#include <limits>
#include <cmath>

namespace {
// Ray-sphere intersection. Returns true and sets outT (ray parameter, >= 0)
// to the nearest entry point if the ray hits the sphere.
bool RaySphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                const glm::vec3& center, float radius, float& outT) {
    glm::vec3 oc = rayOrigin - center;
    float b = glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - radius * radius;
    float disc = b * b - c;
    if (disc < 0.0f) return false;
    float sq = std::sqrt(disc);
    float t0 = -b - sq;
    float t1 = -b + sq;
    float t = (t0 >= 0.0f) ? t0 : t1;
    if (t < 0.0f) return false;
    outT = t;
    return true;
}

// Closest-approach test between a ray (t >= 0) and a finite segment [A, B],
// treated as a capsule of the given radius - the "segment test" the handout
// points to for the fan's thin, elongated blades, where a center-point
// sphere test alone would be too imprecise. Sets outT to the ray parameter
// at closest approach.
bool RaySegmentCapsule(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                        const glm::vec3& A, const glm::vec3& B,
                        float radius, float& outT) {
    glm::vec3 d1 = rayDir;             // normalized ray direction
    glm::vec3 d2 = B - A;              // segment direction (not normalized)
    glm::vec3 r  = rayOrigin - A;

    float a = glm::dot(d1, d1);        // == 1
    float e = glm::dot(d2, d2);
    float f = glm::dot(d2, r);

    float s, t; // s = ray param, t = segment param [0,1]
    if (e < 1e-8f) {
        s = std::max(0.0f, -glm::dot(d1, r));
        t = 0.0f;
    } else {
        float c = glm::dot(d1, r);
        float b = glm::dot(d1, d2);
        float denom = a * e - b * b;
        s = (std::fabs(denom) > 1e-8f) ? (b * f - c * e) / denom : 0.0f;
        t = (b * s + f) / e;
        t = std::clamp(t, 0.0f, 1.0f);
    }
    s = std::max(0.0f, s);

    glm::vec3 closestOnRay = rayOrigin + d1 * s;
    glm::vec3 closestOnSeg = A + d2 * t;
    float dist = glm::length(closestOnRay - closestOnSeg);

    if (dist <= radius) {
        outT = s;
        return true;
    }
    return false;
}
} // namespace

#ifdef PLATFORM_ANDROID
Scene3D::Scene3D(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Scene3D::Scene3D() {}
#endif

Scene3D::~Scene3D()
{
    delete fan;
    delete ground;
}

void Scene3D::InitModel()
{
    LOGI("Scene3D::InitModel");

#ifdef PLATFORM_ANDROID
    fan    = new Fan(mgr);
    ground = new Ground(mgr);
#else
    fan    = new Fan();
    ground = new Ground();
#endif
    fan->InitModel();
    ground->InitModel();

    cameraTransform.TransformInit();
}

void Scene3D::Resize(int w, int h)
{
    screenWidth  = w;
    screenHeight = h;

    const float aspect = (h > 0) ? static_cast<float>(w) / static_cast<float>(h) : 1.0f;
    cameraTransform.TransformSetMatrixMode(PROJECTION_MATRIX);
    cameraTransform.TransformLoadIdentity();
    cameraTransform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0.0f);
    projection = *cameraTransform.TransformGetProjectionMatrix();

    fan->Resize(w, h);
    ground->Resize(w, h);
}

void Scene3D::SetCameraDistance(float d)
{
    cameraDistance = std::clamp(d, kMinCameraDistance, kMaxCameraDistance);
}

void Scene3D::SetFanSpeed(float speed)
{
    fan->SetSpeed(speed);
}

glm::mat4 Scene3D::buildViewMatrix()
{
    // Same pull-back + tilt Quiz 2 baked into the model matrix
    // (Translate(0,0.8,-8) -> Rotate(20,Y)), now driven by the adjustable
    // cameraDistance and living in the view matrix instead (Part 7).
    // Reuses cameraTransform (already initialised once in InitModel) rather
    // than constructing a fresh Transform every frame, since TransformInit()
    // has GL side effects (glEnable/glClear) that shouldn't run per-frame.
    cameraTransform.TransformSetMatrixMode(VIEW_MATRIX);
    cameraTransform.TransformLoadIdentity();
    cameraTransform.TransformTranslate(0.0f, 0.8f, -cameraDistance);
    cameraTransform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);
    return *cameraTransform.TransformGetViewMatrix();
}

void Scene3D::Render()
{
    glm::mat4 view = buildViewMatrix();
    glm::vec3 lightPosEye = glm::vec3(view * glm::vec4(lightPosWorld, 1.0f));

    ground->SetViewProjection(view, projection);
    ground->SetLight(lightPosEye, lightColor);
    ground->Render();

    fan->SetViewProjection(view, projection);
    fan->SetLight(lightPosEye, lightColor);
    fan->Render();
}

bool Scene3D::PickAt(float screenX, float screenY)
{
    glm::mat4 view = buildViewMatrix();

    // Top-left-origin screenY -> bottom-left-origin winY that
    // TransformUnproject expects (matches gluUnProject's viewport convention).
    float winY = static_cast<float>(screenHeight) - screenY;
    int viewport[4] = { 0, 0, screenWidth, screenHeight };

    glm::vec3 nearP, farP;
    cameraTransform.TransformUnproject(screenX, winY, 0.0f, &view, &projection, viewport,
                                        &nearP.x, &nearP.y, &nearP.z);
    cameraTransform.TransformUnproject(screenX, winY, 1.0f, &view, &projection, viewport,
                                        &farP.x, &farP.y, &farP.z);

    glm::vec3 rayOrigin = nearP;
    glm::vec3 rayDir = glm::normalize(farP - nearP);

    float bestT = std::numeric_limits<float>::max();
    FanPart bestPart = FanPart::None;

    for (const auto& part : fan->PickableParts()) {
        if (part.id == FanPart::None) continue;
        float t;
        bool hit = part.isSegment
            ? RaySegmentCapsule(rayOrigin, rayDir, part.segA, part.segB, part.radius, t)
            : RaySphere(rayOrigin, rayDir, part.center, part.radius, t);
        if (hit && t < bestT) {
            bestT = t;
            bestPart = part.id;
        }
    }

    if (bestPart == FanPart::None) {
        fan->SetHighlightedPart(FanPart::None);
        return false;
    }

    // Clicking the already-highlighted part again clears the highlight.
    if (fan->HighlightedPart() == bestPart) {
        fan->SetHighlightedPart(FanPart::None);
        LOGI("Pick: %s deselected", Fan::PartName(bestPart));
    } else {
        fan->SetHighlightedPart(bestPart);
        LOGI("Pick: %s selected", Fan::PartName(bestPart));
    }
    return true;
}
