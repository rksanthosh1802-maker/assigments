#define LOG_TAG "Renderer"
#include "Renderer.h"
#include "Primitives.h"

// ---------------------------------------------------------------------------

Renderer::~Renderer() { clearModels(); }

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr) { assetMgr = mgr; }
#endif

// ---------------------------------------------------------------------------

bool Renderer::initializeRenderer()
{
    LOGI("Renderer::initializeRenderer");
    createModels();
    initializeModels();
    return true;
}

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    models.push_back(new Primitives(assetMgr));
#else
    models.push_back(new Primitives());
#endif
    LOGI("Renderer: %zu model(s) created", models.size());
}

void Renderer::initializeModels()
{
    for (Model* m : models) m->InitModel();
}

void Renderer::clearModels()
{
    for (Model* m : models) delete m;
    models.clear();
}

// ---------------------------------------------------------------------------

void Renderer::resize(int w, int h)
{
    screenWidth  = w;
    screenHeight = h;
    glViewport(0, 0, w, h);
    LOGI("Renderer::resize %d x %d", w, h);
}

void Renderer::render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (Model* m : models) m->Render();
}

// ---------------------------------------------------------------------------

void Renderer::TouchEventDown(float x, float y)
{
    for (Model* m : models) m->TouchEventDown(x, y);
}

void Renderer::TouchEventMove(float x, float y)
{
    for (Model* m : models) m->TouchEventMove(x, y);
}

void Renderer::TouchEventRelease(float x, float y)
{
    for (Model* m : models) m->TouchEventRelease(x, y);
}
