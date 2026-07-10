#define LOG_TAG "Primitives"
#include "Primitives.h"
#include "ShaderHelper.h"

// ---------------------------------------------------------------------------
// Geometry – 10 vertices, 2 per row, in clip space (x: -0.5..0.5, y: -1..1)
// Matches the original layout from the OpenGL ES 3.0 Cookbook Chapter 2.
// ---------------------------------------------------------------------------
static const GLfloat kPositions[] = {
     0.5f,  1.0f,
    -0.5f,  1.0f,
     0.5f,  0.5f,
    -0.5f,  0.5f,
     0.5f,  0.0f,
    -0.5f,  0.0f,
     0.5f, -0.5f,
    -0.5f, -0.5f,
     0.5f, -1.0f,
    -0.5f, -1.0f
};

static const GLfloat kColors[] = {
    0.0f, 0.0f, 1.0f,   // blue
    1.0f, 0.0f, 1.0f,   // magenta
    1.0f, 1.0f, 0.0f,   // yellow
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f
};

static constexpr int       kVertexCount  = 10;
static constexpr GLuint    ATTRIB_POSITION = 0;
static constexpr GLuint    ATTRIB_COLOR    = 1;

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
Primitives::Primitives(AAssetManager* assetMgr)
    : mgr(assetMgr)
{
#else
Primitives::Primitives()
{
#endif
    modelType = TriangleType;
    primitive = GL_LINES;   // Start with GL_LINES
}

Primitives::~Primitives()
{
    if (vao)     { glDeleteVertexArrays(1, &vao);     vao     = 0; }
    if (vboPos)  { glDeleteBuffers(1, &vboPos);       vboPos  = 0; }
    if (vboColor){ glDeleteBuffers(1, &vboColor);     vboColor= 0; }
    if (program) { glDeleteProgram(program);           program = 0; }
}

// ---------------------------------------------------------------------------
// InitModel – compile shaders, create VAO/VBO
// ---------------------------------------------------------------------------
void Primitives::InitModel()
{
    LOGI("Primitives::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/PrimitiveVertex.glsl",
        "shader/PrimitiveFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "PrimitiveVertex.glsl",
        "PrimitiveFragment.glsl");
#endif

    if (!program) {
        LOGE("Primitives: failed to build shader program");
        return;
    }

    // --- VAO ---
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // --- Position VBO ---
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // --- Color VBO ---
    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kColors), kColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glVertexAttribPointer(ATTRIB_COLOR, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOGI("Primitives::InitModel done – starting with GL_LINES");
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------
void Primitives::Render()
{
    if (!program || !vao) return;

    glUseProgram(program);
    glDisable(GL_CULL_FACE);
    glLineWidth(10.0f);

    glBindVertexArray(vao);
    glDrawArrays(primitive, 0, kVertexCount);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Touch – cycle through all 7 primitive types on each tap
// GL enum values: POINTS=0, LINES=1, LINE_LOOP=2, LINE_STRIP=3,
//                 TRIANGLES=4, TRIANGLE_STRIP=5, TRIANGLE_FAN=6
// ---------------------------------------------------------------------------
void Primitives::TouchEventDown(float, float)
{
    primitive = static_cast<GLenum>((static_cast<int>(primitive) + 1) % 7);
    LOGI("Primitives: switched to primitive type %u", primitive);
}

void Primitives::TouchEventMove(float, float)    {}
void Primitives::TouchEventRelease(float, float) {}
