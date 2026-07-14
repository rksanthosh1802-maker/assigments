/*!
@file    	Fan.cpp
@author  	r.santhosh@digipen.edu
@date    	13/07/2026

Implements the Fan model: cube geometry setup, VAO/VBO/EBO creation, per-part
model matrix composition, blade spin animation, and the seven draw calls.

*//*__________________________________________________________________________*/
#define LOG_TAG "Fan3D"
#include "Fan.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// The one cube every part is drawn from: 8 corners, 36 indices.
// Corner shade is a grayscale factor - front face bright, back face darker -
// so the cube reads as 3D. The actual colour comes from the PARTCOLOR uniform.
// ---------------------------------------------------------------------------

static const GLfloat kPositions[8][3] = {
    {-0.5f, -0.5f,  0.5f},  // V0 front
    {-0.5f,  0.5f,  0.5f},  // V1
    { 0.5f,  0.5f,  0.5f},  // V2
    { 0.5f, -0.5f,  0.5f},  // V3
    {-0.5f, -0.5f, -0.5f},  // V4 back
    {-0.5f,  0.5f, -0.5f},  // V5
    { 0.5f,  0.5f, -0.5f},  // V6
    { 0.5f, -0.5f, -0.5f},  // V7
};

static const GLfloat kShades[8] = {
    1.00f, 0.90f, 0.85f, 0.95f,   // front corners
    0.65f, 0.60f, 0.60f, 0.65f,   // back corners
};

static const GLushort kIndices[36] = {
    0,3,1, 3,2,1,   // front
    7,4,6, 4,5,6,   // back
    4,0,5, 0,1,5,   // left
    3,7,2, 7,6,2,   // right
    1,2,5, 2,6,5,   // top
    3,0,7, 0,4,7    // bottom
};

static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_SHADE    = 1;

static constexpr GLsizeiptr kPosSize   = sizeof(kPositions);
static constexpr GLsizeiptr kShadeSize = sizeof(kShades);

static constexpr int   kBladeCount = 4;
static constexpr float kBladeStep  = 90.0f;

static const GLfloat kBladeColors[kBladeCount][3] = {
    {0.85f, 0.15f, 0.15f},   // B0 red
    {0.15f, 0.35f, 0.90f},   // B1 blue
    {0.95f, 0.55f, 0.05f},   // B2 orange
    {0.15f, 0.75f, 0.25f},   // B3 green
};

// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* assetMgr) : mgr(assetMgr) {
#else
Fan::Fan() {
#endif
    modelType = CubeType;
}

Fan::~Fan()
{
    if (vao)     { glDeleteVertexArrays(1, &vao); vao     = 0; }
    if (vbo)     { glDeleteBuffers(1, &vbo);      vbo     = 0; }
    if (ibo)     { glDeleteBuffers(1, &ibo);      ibo     = 0; }
    if (program) { glDeleteProgram(program);      program = 0; }
}

// ---------------------------------------------------------------------------

void Fan::InitModel()
{
    LOGI("Fan::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "FanVertex.glsl", "FanFragment.glsl");
#endif

    if (!program) { LOGE("Fan: failed to build shader program"); return; }

    uMVP       = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uPartColor = glGetUniformLocation(program, "PARTCOLOR");

    transform.TransformInit();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // One VBO, two sub-regions: positions first, per-corner shades after.
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, kPosSize + kShadeSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,        kPosSize,   kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, kPosSize, kShadeSize, kShades);

    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(ATTRIB_SHADE);
    glVertexAttribPointer(ATTRIB_SHADE, 1, GL_FLOAT, GL_FALSE, 0, (void*)kPosSize);

    // The IBO binding is VAO state, so it must be bound while recording.
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    lastMoveTime = std::chrono::steady_clock::now();
}

// ---------------------------------------------------------------------------

void Fan::Resize(int w, int h)
{
    const float aspect = (h > 0) ? static_cast<float>(w) / static_cast<float>(h) : 1.0f;

    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0.0f);

    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadIdentity();
}

// ---------------------------------------------------------------------------
// One draw: upload this part's MVP and colour, then draw the shared cube.
// ---------------------------------------------------------------------------
void Fan::drawPart(float r, float g, float b)
{
    const GLfloat color[3] = { r, g, b };

    glUniformMatrix4fv(uMVP, 1, GL_FALSE,
                       glm::value_ptr(*transform.TransformGetModelViewProjectionMatrix()));
    glUniform3fv(uPartColor, 1, color);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
}

// ---------------------------------------------------------------------------

void Fan::Render()
{
    if (!program || !vao) return;

    if (fanOn) spinAngle += kBaseSpeed + dragBoost;
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;

    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // Shared world transform: pull back from the camera, then tilt to show depth.
    transform.TransformTranslate(0.0f, 0.8f, -8.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    // Base
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -2.60f, 0.0f);
        transform.TransformScale(1.60f, 0.25f, 0.80f);
        drawPart(0.45f, 0.28f, 0.12f);
    transform.TransformPopMatrix();

    // Pole
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -1.21f, 0.0f);
        transform.TransformScale(0.15f, 2.53f, 0.15f);
        drawPart(0.55f, 0.55f, 0.58f);
    transform.TransformPopMatrix();

    // Hub
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.20f, 0.0f);
        transform.TransformScale(0.30f, 0.30f, 0.30f);
        drawPart(0.20f, 0.20f, 0.22f);
    transform.TransformPopMatrix();

    // Blades - each pivots about the hub's Z axis, spaced 90 degrees apart.
    for (int i = 0; i < kBladeCount; ++i) {
        transform.TransformPushMatrix();
            transform.TransformTranslate(0.0f, 0.20f, 0.15f);
            transform.TransformRotate(glm::radians(spinAngle + i * kBladeStep),
                                      0.0f, 0.0f, 1.0f);
            transform.TransformTranslate(0.0f, 0.55f, 0.0f);
            transform.TransformScale(0.22f, 0.80f, 0.05f);
            drawPart(kBladeColors[i][0], kBladeColors[i][1], kBladeColors[i][2]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Gestures
// ---------------------------------------------------------------------------

void Fan::TouchEventDown(float x, float y)
{
    lastX          = x;
    lastY          = y;
    movedDistance  = 0.0f;
    dragBoost      = 0.0f;
    lastMoveTime   = std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x, float y)
{
    const auto  now  = std::chrono::steady_clock::now();
    const float dtMs = std::chrono::duration<float, std::milli>(now - lastMoveTime).count();

    const float dx   = x - lastX;
    const float dy   = y - lastY;
    const float dist = std::sqrt(dx * dx + dy * dy);

    movedDistance += dist;

    if (dtMs > 0.0f) {
        const float velocity = dist / dtMs;                     // pixels per millisecond
        dragBoost = std::min(velocity * kBoostScale, kMaxBoost);
    }

    lastX        = x;
    lastY        = y;
    lastMoveTime = now;
}

void Fan::TouchEventRelease(float x, float y)
{
    dragBoost = 0.0f;

    if (movedDistance < kTapThreshold) {
        fanOn = !fanOn;
        LOGI("Fan %s", fanOn ? "ON" : "OFF");
    }
}