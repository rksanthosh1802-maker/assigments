/*!
@file    	Fan.cpp
@author  	r.santhosh@digipen.edu
@date    	13/07/2026

Implements the Fan model: cube geometry setup, VAO/VBO/EBO creation, per-part
model matrix composition, blade spin animation, and the seven draw calls.

See Fan.h for the full list of Quiz 3 changes from Quiz 2.

*//*__________________________________________________________________________*/
#define LOG_TAG "Fan3D"
#include "Fan.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// The one cube every part is drawn from. Corners are duplicated per face (24
// vertices instead of 8) so each face gets its own flat normal - Part 2's
// note that a cube's shared corners can't share one normal per face.
// Position + normal replace the old position + per-corner "Shade" gradient;
// real per-fragment Phong now does the shading work.
// ---------------------------------------------------------------------------

struct CubeVertex { GLfloat px, py, pz, nx, ny, nz; };

static const CubeVertex kCubeVerts[24] = {
    // +X
    { 0.5f,-0.5f,-0.5f,  1,0,0}, { 0.5f, 0.5f,-0.5f,  1,0,0}, { 0.5f, 0.5f, 0.5f,  1,0,0}, { 0.5f,-0.5f, 0.5f,  1,0,0},
    // -X
    {-0.5f,-0.5f, 0.5f, -1,0,0}, {-0.5f, 0.5f, 0.5f, -1,0,0}, {-0.5f, 0.5f,-0.5f, -1,0,0}, {-0.5f,-0.5f,-0.5f, -1,0,0},
    // +Y
    {-0.5f, 0.5f,-0.5f,  0,1,0}, {-0.5f, 0.5f, 0.5f,  0,1,0}, { 0.5f, 0.5f, 0.5f,  0,1,0}, { 0.5f, 0.5f,-0.5f,  0,1,0},
    // -Y
    {-0.5f,-0.5f, 0.5f,  0,-1,0}, {-0.5f,-0.5f,-0.5f, 0,-1,0}, { 0.5f,-0.5f,-0.5f, 0,-1,0}, { 0.5f,-0.5f, 0.5f, 0,-1,0},
    // +Z
    {-0.5f,-0.5f, 0.5f,  0,0,1}, { 0.5f,-0.5f, 0.5f,  0,0,1}, { 0.5f, 0.5f, 0.5f,  0,0,1}, {-0.5f, 0.5f, 0.5f,  0,0,1},
    // -Z
    { 0.5f,-0.5f,-0.5f,  0,0,-1}, {-0.5f,-0.5f,-0.5f, 0,0,-1}, {-0.5f, 0.5f,-0.5f, 0,0,-1}, { 0.5f, 0.5f,-0.5f, 0,0,-1},
};

static const GLushort kIndices[36] = {
     0, 1, 2,  0, 2, 3,       // +X
     4, 5, 6,  4, 6, 7,       // -X
     8, 9,10,  8,10,11,       // +Y
    12,13,14, 12,14,15,       // -Y
    16,17,18, 16,18,19,       // +Z
    20,21,22, 20,22,23,       // -Z
};

static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_NORMAL   = 1;

static constexpr int   kBladeCount = 4;
static constexpr float kBladeStep  = 90.0f;

static const GLfloat kBladeColors[kBladeCount][3] = {
    {0.85f, 0.15f, 0.15f},   // B0 red
    {0.15f, 0.35f, 0.90f},   // B1 blue
    {0.95f, 0.55f, 0.05f},   // B2 orange
    {0.15f, 0.75f, 0.25f},   // B3 green
};
static const GLfloat kHighlightColor[3] = { 1.0f, 0.65f, 0.0f }; // amber (Part 8)

// ---------------------------------------------------------------------------

const char* Fan::PartName(FanPart part) {
    switch (part) {
        case FanPart::Base:   return "Base";
        case FanPart::Pole:   return "Pole";
        case FanPart::Hub:    return "Hub";
        case FanPart::Blade0: return "Blade0";
        case FanPart::Blade1: return "Blade1";
        case FanPart::Blade2: return "Blade2";
        case FanPart::Blade3: return "Blade3";
        default:              return "None";
    }
}

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

    uMVP          = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uMV           = glGetUniformLocation(program, "MODELVIEWMATRIX");
    uNormalMatrix = glGetUniformLocation(program, "NORMALMATRIX");
    uPartColor    = glGetUniformLocation(program, "PARTCOLOR");
    uLightPosEye  = glGetUniformLocation(program, "LIGHTPOS_EYE");
    uLightColor   = glGetUniformLocation(program, "LIGHTCOLOR");

    transform.TransformInit();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVerts), kCubeVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(CubeVertex), (void*)offsetof(CubeVertex, px));

    glEnableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(CubeVertex), (void*)offsetof(CubeVertex, nx));

    // The IBO binding is VAO state, so it must be bound while recording.
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ---------------------------------------------------------------------------
// Quiz 2 built its own projection here. Quiz 3 (Part 7): Scene3D owns the
// camera (distance + projection) and pushes it into Fan via
// SetViewProjection() every frame, so Resize() no longer touches the
// projection matrix at all.
// ---------------------------------------------------------------------------
void Fan::Resize(int /*w*/, int /*h*/)
{
}

void Fan::SetViewProjection(const glm::mat4& view, const glm::mat4& proj)
{
    // Loaded into this Fan instance's own VIEW/PROJECTION stacks (rather than
    // rebuilt from scratch) so transform.TransformGetModelViewProjectionMatrix()
    // and TransformGetNormalMatrix() - both driven by these stacks - keep
    // working exactly as Quiz 2 used them, just fed Scene3D's shared camera
    // instead of a locally-baked one.
    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadMatrix(const_cast<glm::mat4*>(&view));

    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadMatrix(const_cast<glm::mat4*>(&proj));
}

void Fan::SetLight(const glm::vec3& eyeSpaceLightPos, const glm::vec3& color)
{
    lightPosEye = eyeSpaceLightPos;
    lightColor  = color;
}

// ---------------------------------------------------------------------------
// One draw: upload this part's MVP, normal matrix, and colour, then draw the
// shared cube. Also caches this part's world-space pick info (Part 8).
// ---------------------------------------------------------------------------
void Fan::drawPart(FanPart id, float r, float g, float b)
{
    bool isHi = (highlighted == id);
    const GLfloat color[3] = { isHi ? kHighlightColor[0] : r,
                                isHi ? kHighlightColor[1] : g,
                                isHi ? kHighlightColor[2] : b };

    glUniformMatrix4fv(uMVP, 1, GL_FALSE,
                       glm::value_ptr(*transform.TransformGetModelViewProjectionMatrix()));
    glUniformMatrix4fv(uMV, 1, GL_FALSE,
                       glm::value_ptr(*transform.TransformGetModelViewMatrix()));

    glm::mat3 normalMat;
    transform.TransformGetNormalMatrix(&normalMat); // inverse-transpose of (View*Model) upper 3x3
    glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMat));

    glUniform3fv(uPartColor,   1, color);
    glUniform3fv(uLightPosEye, 1, glm::value_ptr(lightPosEye));
    glUniform3fv(uLightColor,  1, glm::value_ptr(lightColor));

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

    // Cache this part's world-space (i.e. Fan-local-space; Scene3D's picking
    // ray is built in the same space since the camera now lives entirely in
    // the view matrix, not baked into the model matrix) position for Part 8.
    glm::mat4* modelMat = transform.TransformGetModelMatrix();
    glm::vec3 worldCenter = glm::vec3((*modelMat) * glm::vec4(0, 0, 0, 1));

    FanPickablePart& p = pickables[(size_t)id];
    p.id = id;
    if (id == FanPart::Pole) {
        // Thin but long along Y - still simple enough for a sphere test since
        // it's axis-aligned and centered; blades get the real segment test.
        p.isSegment = false;
        p.center = worldCenter;
        p.radius = 1.4f;
    } else if (id == FanPart::Blade0 || id == FanPart::Blade1 ||
               id == FanPart::Blade2 || id == FanPart::Blade3) {
        // Segment test (capsule) along the blade's long local-Y axis, since a
        // single center-point sphere is too imprecise for a thin elongated
        // blade (handout's Part 8 note).
        p.isSegment = true;
        glm::vec3 halfAxis = glm::vec3((*modelMat) * glm::vec4(0, 0.5f, 0, 0));
        p.segA = worldCenter - halfAxis;
        p.segB = worldCenter + halfAxis;
        p.radius = 0.20f;
    } else {
        p.isSegment = false;
        p.center = worldCenter;
        p.radius = (id == FanPart::Base) ? 1.1f : 0.35f; // Base or Hub
    }
}

// ---------------------------------------------------------------------------

void Fan::Render()
{
    if (!program || !vao) return;

    spinAngleAdvance();

    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();
    // No world pull-back here anymore (Part 7) - Scene3D's view matrix now
    // owns the camera distance/orientation that Quiz 2 baked in as
    // Translate(0,0.8,-8) + Rotate(20,Y).

    // Base
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -2.60f, 0.0f);
        transform.TransformScale(1.60f, 0.25f, 0.80f);
        drawPart(FanPart::Base, 0.45f, 0.28f, 0.12f);
    transform.TransformPopMatrix();

    // Pole
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -1.21f, 0.0f);
        transform.TransformScale(0.15f, 2.53f, 0.15f);
        drawPart(FanPart::Pole, 0.55f, 0.55f, 0.58f);
    transform.TransformPopMatrix();

    // Hub
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.20f, 0.0f);
        transform.TransformScale(0.30f, 0.30f, 0.30f);
        drawPart(FanPart::Hub, 0.20f, 0.20f, 0.22f);
    transform.TransformPopMatrix();

    // Blades - each pivots about the hub's Z axis, spaced 90 degrees apart.
    const FanPart bladeIds[kBladeCount] = { FanPart::Blade0, FanPart::Blade1, FanPart::Blade2, FanPart::Blade3 };
    for (int i = 0; i < kBladeCount; ++i) {
        transform.TransformPushMatrix();
            transform.TransformTranslate(0.0f, 0.20f, 0.15f);
            transform.TransformRotate(glm::radians(spinAngle + i * kBladeStep),
                                      0.0f, 0.0f, 1.0f);
            transform.TransformTranslate(0.0f, 0.55f, 0.0f);
            transform.TransformScale(0.22f, 0.80f, 0.05f);
            drawPart(bladeIds[i], kBladeColors[i][0], kBladeColors[i][1], kBladeColors[i][2]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}

void Fan::spinAngleAdvance()
{
    spinAngle += speed;
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;
    if (spinAngle < 0.0f)    spinAngle += 360.0f;
}
