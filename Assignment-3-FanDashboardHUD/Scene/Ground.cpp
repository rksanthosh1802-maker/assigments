/*!
@file    	Ground.cpp
@date    	Android Programming Quiz 3

Implements Ground: one flat quad (two triangles) in the XZ plane, checkerboard
generated procedurally in the fragment shader (Part 3), Phong-lit (Part 2).

*//*__________________________________________________________________________*/
#define LOG_TAG "Fan3D"
#include "Ground.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <cstddef>

namespace {
struct GroundVertex { GLfloat px, py, pz, nx, ny, nz; };

// One big flat quad at y = 0. The fragment shader repeats the checkerboard
// with mod() so this single finite quad still reads as an infinite tiled
// floor (Part 3 requirement), even though the geometry itself is finite.
constexpr float kHalfSize = 60.0f;
const GroundVertex kQuadVerts[4] = {
    { -kHalfSize, 0, -kHalfSize,  0, 1, 0 },
    {  kHalfSize, 0, -kHalfSize,  0, 1, 0 },
    {  kHalfSize, 0,  kHalfSize,  0, 1, 0 },
    { -kHalfSize, 0,  kHalfSize,  0, 1, 0 },
};
// Winding chosen so the front face (CCW, per Transform::TransformInit's
// glFrontFace(GL_CCW)) points +Y - visible from above, where the camera
// sits - rather than the reverse, which backface culling would discard.
const GLushort kQuadIndices[6] = { 0, 2, 1, 0, 3, 2 };

constexpr GLuint ATTRIB_POSITION = 0;
constexpr GLuint ATTRIB_NORMAL   = 1;
} // namespace

#ifdef PLATFORM_ANDROID
Ground::Ground(AAssetManager* assetMgr) : mgr(assetMgr) { modelType = TriangleType; }
#else
Ground::Ground() { modelType = TriangleType; }
#endif

Ground::~Ground()
{
    if (vao)     { glDeleteVertexArrays(1, &vao); vao     = 0; }
    if (vbo)     { glDeleteBuffers(1, &vbo);      vbo     = 0; }
    if (ibo)     { glDeleteBuffers(1, &ibo);      ibo     = 0; }
    if (program) { glDeleteProgram(program);      program = 0; }
}

void Ground::InitModel()
{
    LOGI("Ground::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr, "shader/GroundVertex.glsl", "shader/GroundFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("GroundVertex.glsl", "GroundFragment.glsl");
#endif

    if (!program) { LOGE("Ground: failed to build shader program"); return; }

    uMVP          = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uMV           = glGetUniformLocation(program, "MODELVIEWMATRIX");
    uNormalMatrix = glGetUniformLocation(program, "NORMALMATRIX");
    uLightPosEye  = glGetUniformLocation(program, "LIGHTPOS_EYE");
    uLightColor   = glGetUniformLocation(program, "LIGHTCOLOR");

    transform.TransformInit();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(GroundVertex), (void*)offsetof(GroundVertex, px));
    glEnableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(GroundVertex), (void*)offsetof(GroundVertex, nx));

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kQuadIndices), kQuadIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Ground::Resize(int, int) {}

void Ground::SetViewProjection(const glm::mat4& view, const glm::mat4& proj)
{
    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadMatrix(const_cast<glm::mat4*>(&view));
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadMatrix(const_cast<glm::mat4*>(&proj));
}

void Ground::SetLight(const glm::vec3& eyeSpaceLightPos, const glm::vec3& color)
{
    lightPosEye = eyeSpaceLightPos;
    lightColor  = color;
}

void Ground::Render()
{
    if (!program || !vao) return;

    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();
    // The fan's base sits at local y = -2.60 with half-height 0.125 (Fan.cpp),
    // so its bottom edge is at y = -2.725. Push the ground down to sit right
    // under that instead of at y = 0 (which would float up near the hub).
    transform.TransformTranslate(0.0f, -2.73f, 0.0f);

    glUniformMatrix4fv(uMVP, 1, GL_FALSE,
                       glm::value_ptr(*transform.TransformGetModelViewProjectionMatrix()));
    glUniformMatrix4fv(uMV, 1, GL_FALSE,
                       glm::value_ptr(*transform.TransformGetModelViewMatrix()));

    glm::mat3 normalMat;
    transform.TransformGetNormalMatrix(&normalMat);
    glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMat));

    glUniform3fv(uLightPosEye, 1, glm::value_ptr(lightPosEye));
    glUniform3fv(uLightColor,  1, glm::value_ptr(lightColor));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);
}
