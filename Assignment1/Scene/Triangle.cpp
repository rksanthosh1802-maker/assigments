/**
 * @file    Triangle.cpp
 * @brief   Triangle implementation. Shader source is loaded at runtime from
 *          the BlueTriangleVertex/Fragment .glsl files via ShaderHelper's
 *          file-based path - no shader text is embedded as C-string literals
 *          here anymore.
 * @author  Santhosh Ravikumar
 * @date    2026
 */
#include "Triangle.h"
#include "ShaderHelper.h"

namespace {
    // Interleavable position / color data for a single RGB-gradient triangle,
    // centered at the origin in clip space.
    const GLfloat kPositions[] = {
         0.0f,  0.6f, 0.0f, 1.0f,
        -0.6f, -0.6f, 0.0f, 1.0f,
         0.6f, -0.6f, 0.0f, 1.0f,
    };

    const GLfloat kColors[] = {
        0.0f, 1.0f, 0.0f, 1.0f,   // top    - green
        0.0f, 0.0f, 1.0f, 1.0f,   // left   - blue
        1.0f, 0.0f, 0.0f, 1.0f,   // right  - red
    };
}

#ifdef PLATFORM_ANDROID
Triangle::Triangle(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Triangle::Triangle() {}
#endif

Triangle::~Triangle()
{
    if (vboPos)   glDeleteBuffers(1, &vboPos);
    if (vboColor) glDeleteBuffers(1, &vboColor);
    if (vao)      glDeleteVertexArrays(1, &vao);
    if (programID) glDeleteProgram(programID);
}

void Triangle::InitModel()
{
#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(mgr, "shader/BlueTriangleVertex.glsl", "shader/BlueTriangleFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile("BlueTriangleVertex.glsl", "BlueTriangleFragment.glsl");
#endif

    uRadianAngle = glGetUniformLocation(programID, "RadianAngle");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kColors), kColors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Triangle::Resize(int w, int h)
{
    glViewport(0, 0, w, h);
}

void Triangle::Render()
{
    degree += 0.6f;
    if (degree >= 360.0f) degree -= 360.0f;
    float radian = degree * 3.14159265f / 180.0f;

    // Note: the screen is cleared once per frame in Renderer::render(),
    // not here - this Model only draws its own geometry.
    glUseProgram(programID);
    glUniform1f(uRadianAngle, radian);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
