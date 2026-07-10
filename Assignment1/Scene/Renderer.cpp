/**
 * @file    Renderer.cpp
 * @brief   Renderer implementation. This is the ONLY translation unit allowed
 *          to know about concrete Model subclasses (Triangle, Square) - the
 *          header stays shape-agnostic.
 * @author  Santhosh Ravikumar
 * @date    2026
 */
#include "Renderer.h"
#include "Triangle.h"
#include "Square.h"

Renderer& Renderer::Instance()
{
    static Renderer instance;
    return instance;
}

Renderer::~Renderer()
{
    clearModels();
}

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr)
{
    assetMgr = mgr;
}
#endif

void Renderer::createModels()
{
    clearModels();

#ifdef PLATFORM_ANDROID
    models.push_back(new Triangle(assetMgr));
    models.push_back(new Square(assetMgr));
#else
    models.push_back(new Triangle());
    models.push_back(new Square());
#endif
}

void Renderer::initializeModels()
{
    for (Model* m : models)
        m->InitModel();
}

void Renderer::clearModels()
{
    for (Model* m : models)
        delete m;
    models.clear();
}

bool Renderer::initializeRenderer()
{
    createModels();
    initializeModels();
    return true;
}

void Renderer::resize(int w, int h)
{
    for (Model* m : models)
        m->Resize(w, h);
}

void Renderer::render()
{
    // Clear once per frame here, not inside each Model::Render() - otherwise
    // the second shape drawn would wipe out the first.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (Model* m : models)
        m->Render();
}
