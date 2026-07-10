/**
 * @file    Square.cpp
 * @brief   Square implementation, drawn as a GL_TRIANGLE_FAN of 4 vertices.
 *          Shader source loads from its own YellowSquare .glsl files via
 *          ShaderHelper's file-based path, same as Triangle.
 * @author  Santhosh Ravikumar
 * @date    2026
 */
#include "Square.h"
#include "ShaderHelper.h"

namespace {
    // Offset to the right side of clip space so it never fully overlaps
    // the triangle, which is centered at the origin.
    const GLfloat kPositions[] = {
        0.45f,  0.55f, 0.0f, 1.0f,   // top-left
        0.45f,  0.15f, 0.0f, 1.0f,   // bottom-left
        0.85f,  0.15f, 0.0f, 1.0f,   // bottom-right
        0.85f,  0.55f, 0.0f, 1.0f,   // top-right
    };
}

#ifdef PLATFORM_ANDROID
Square::Square(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Square::Square() {}
#endif

Square::~Square()
{
    if (vboPos)    glDeleteBuffers(1, &vboPos);
    if (vao)       glDeleteVertexArrays(1, &vao);
    if (programID) glDeleteProgram(programID);
}

void Square::InitModel()
{
#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(mgr, "shader/YellowSquareVertex.glsl", "shader/YellowSquareFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile("YellowSquareVertex.glsl", "YellowSquareFragment.glsl");
#endif

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Square::Resize(int w, int h)
{
    glViewport(0, 0, w, h);
}

void Square::Render()
{
    // Note: the screen is cleared once per frame in Renderer::render(),
    // not here - this Model only draws its own geometry.
    glUseProgram(programID);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}
