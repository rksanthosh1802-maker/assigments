/*!
@file    	SceneHUD.cpp
@date    	Android Programming Quiz 3
*//*__________________________________________________________________________*/
#define LOG_TAG "Fan3D"
#include "SceneHUD.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <cmath>

namespace {
struct HudVertex { GLfloat x, y, r, g, b; };

void AddQuad(std::vector<HudVertex>& out, const HudRect& rect, const glm::vec3& color) {
    float x0 = rect.x, y0 = rect.y, x1 = rect.x + rect.w, y1 = rect.y + rect.h;
    HudVertex v0{ x0, y0, color.r, color.g, color.b };
    HudVertex v1{ x1, y0, color.r, color.g, color.b };
    HudVertex v2{ x1, y1, color.r, color.g, color.b };
    HudVertex v3{ x0, y1, color.r, color.g, color.b };
    // Winding order matters here: TransformOrtho(0,w,h,0,-1,1) flips Y (screen
    // top-left origin -> NDC bottom-left origin), which reverses handedness.
    // v0->v1->v2 in screen space becomes clockwise in NDC, and Transform::
    // TransformInit enables backface culling (CCW front), so that ordering
    // gets silently discarded. v0->v2->v1 / v0->v3->v2 is the corrected,
    // front-facing winding - same fix as Ground's quad (Part 3).
    out.push_back(v0); out.push_back(v2); out.push_back(v1);
    out.push_back(v0); out.push_back(v3); out.push_back(v2);
}

const glm::vec3 kSpeedButtonColor(0.20f, 0.45f, 0.85f);
const glm::vec3 kZoomButtonColor (0.85f, 0.55f, 0.15f);
const glm::vec3 kBarActiveColor  (0.25f, 0.85f, 0.30f);
const glm::vec3 kBarInactiveColor(0.35f, 0.35f, 0.35f);

constexpr GLuint ATTRIB_POSITION = 0;
constexpr GLuint ATTRIB_COLOR    = 1;
} // namespace

#ifdef PLATFORM_ANDROID
SceneHUD::SceneHUD(AAssetManager* assetMgr) : mgr(assetMgr) { modelType = TriangleType; }
#else
SceneHUD::SceneHUD() { modelType = TriangleType; }
#endif

SceneHUD::~SceneHUD()
{
    if (buttonsVao) { glDeleteVertexArrays(1, &buttonsVao); buttonsVao = 0; }
    if (buttonsVbo) { glDeleteBuffers(1, &buttonsVbo);      buttonsVbo = 0; }
    if (barVao)     { glDeleteVertexArrays(1, &barVao);     barVao     = 0; }
    if (barVbo)     { glDeleteBuffers(1, &barVbo);          barVbo     = 0; }
    if (program)     { glDeleteProgram(program);            program    = 0; }
}

void SceneHUD::InitModel()
{
    LOGI("SceneHUD::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr, "shader/HUDVertex.glsl", "shader/HUDFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("HUDVertex.glsl", "HUDFragment.glsl");
#endif

    if (!program) { LOGE("SceneHUD: failed to build shader program"); return; }

    uProj = glGetUniformLocation(program, "PROJECTIONMATRIX");

    transform.TransformInit();

    glGenVertexArrays(1, &buttonsVao);
    glGenBuffers(1, &buttonsVbo);
    glGenVertexArrays(1, &barVao);
    glGenBuffers(1, &barVbo);
}

void SceneHUD::Resize(int w, int h)
{
    screenW = w;
    screenH = h;

    // Top-left origin, Y down - matches mouse/touch coordinates directly,
    // so HUD hit-testing (TouchDown) never needs unprojection (Part 4/5).
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformOrtho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f, -1.0f, 1.0f);
    ortho = *transform.TransformGetProjectionMatrix();

    constexpr float margin = 30.0f, btnW = 120.0f, btnH = 50.0f, gap = 20.0f;
    buttonRects[(size_t)HudButton::PlusSpeed]  = { margin, margin, btnW, btnH };
    buttonRects[(size_t)HudButton::MinusSpeed] = { margin, margin + btnH + gap, btnW, btnH };
    buttonRects[(size_t)HudButton::PlusZoom]   = { w - margin - btnW, margin, btnW, btnH };
    buttonRects[(size_t)HudButton::MinusZoom]  = { w - margin - btnW, margin + btnH + gap, btnW, btnH };

    float barTotalW = kBarSlots * kBarSlotW;
    barOrigin = { (w - barTotalW) * 0.5f, h - 60.0f, barTotalW, kBarSlotH };

    rebuildButtonGeometry();
    rebuildBarGeometry();
}

void SceneHUD::rebuildButtonGeometry()
{
    std::vector<HudVertex> verts;
    AddQuad(verts, buttonRects[(size_t)HudButton::PlusSpeed],  kSpeedButtonColor);
    AddQuad(verts, buttonRects[(size_t)HudButton::MinusSpeed], kSpeedButtonColor);
    AddQuad(verts, buttonRects[(size_t)HudButton::PlusZoom],   kZoomButtonColor);
    AddQuad(verts, buttonRects[(size_t)HudButton::MinusZoom],  kZoomButtonColor);

    glBindVertexArray(buttonsVao);
    glBindBuffer(GL_ARRAY_BUFFER, buttonsVbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(HudVertex), verts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(HudVertex), (void*)offsetof(HudVertex, x));
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glVertexAttribPointer(ATTRIB_COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(HudVertex), (void*)offsetof(HudVertex, r));
    glBindVertexArray(0);
}

void SceneHUD::rebuildBarGeometry()
{
    // Dynamic HUD element (Part 6): rebuilt whenever speed changes so the
    // first `speed` blocks are green and the rest grey.
    std::vector<HudVertex> verts;
    verts.reserve(kBarSlots * 6);
    int litCount = std::clamp(static_cast<int>(std::round(speed)), 0, kBarSlots);
    for (int i = 0; i < kBarSlots; ++i) {
        HudRect slot{ barOrigin.x + i * kBarSlotW, barOrigin.y, kBarSlotW - 2.0f, kBarSlotH };
        AddQuad(verts, slot, (i < litCount) ? kBarActiveColor : kBarInactiveColor);
    }
    glBindVertexArray(barVao);
    glBindBuffer(GL_ARRAY_BUFFER, barVbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(HudVertex), verts.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(HudVertex), (void*)offsetof(HudVertex, x));
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glVertexAttribPointer(ATTRIB_COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(HudVertex), (void*)offsetof(HudVertex, r));
    glBindVertexArray(0);
}

bool SceneHUD::pointInRect(float x, float y, const HudRect& r) const
{
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

bool SceneHUD::TouchDown(float x, float y)
{
    for (size_t i = 0; i < buttonRects.size(); ++i) {
        if (!pointInRect(x, y, buttonRects[i])) continue;

        auto btn = static_cast<HudButton>(i);
        switch (btn) {
            case HudButton::PlusSpeed:
                speed = std::clamp(speed + 1.0f, 0.0f, 20.0f);
                LOGI("+SPD pressed (speed=%.1f)", speed);
                break;
            case HudButton::MinusSpeed:
                speed = std::clamp(speed - 1.0f, 0.0f, 20.0f);
                LOGI("-SPD pressed (speed=%.1f)", speed);
                break;
            case HudButton::PlusZoom: // closer
                cameraDistance = std::clamp(cameraDistance - 1.0f, 3.0f, 20.0f);
                LOGI("+ZOOM pressed (dist=%.1f)", cameraDistance);
                break;
            case HudButton::MinusZoom: // farther
                cameraDistance = std::clamp(cameraDistance + 1.0f, 3.0f, 20.0f);
                LOGI("-ZOOM pressed (dist=%.1f)", cameraDistance);
                break;
            default: break;
        }
        rebuildBarGeometry();
        return true; // consumed - must not also trigger 3D picking (Part 8)
    }
    return false;
}

void SceneHUD::TouchRelease()
{
    // No press-and-hold in the core requirements; nothing to release here.
}

void SceneHUD::Render()
{
    if (!program) return;

    glUseProgram(program);
    glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(ortho));

    glBindVertexArray(buttonsVao);
    glDrawArrays(GL_TRIANGLES, 0, 4 * 6);

    glBindVertexArray(barVao);
    glDrawArrays(GL_TRIANGLES, 0, kBarSlots * 6);

    glBindVertexArray(0);
}
