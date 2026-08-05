#define LOG_TAG "Renderer"
#include "Renderer.h"
#include "Scene3D.h"
#include "SceneHUD.h"

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
    scene3D->InitModel();
    sceneHUD->InitModel();
    return true;
}

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    scene3D  = new Scene3D(assetMgr);
    sceneHUD = new SceneHUD(assetMgr);
#else
    scene3D  = new Scene3D();
    sceneHUD = new SceneHUD();
#endif
    LOGI("Renderer: Scene3D + SceneHUD created");
}

void Renderer::clearModels()
{
    delete scene3D;  scene3D  = nullptr;
    delete sceneHUD; sceneHUD = nullptr;
}

// ---------------------------------------------------------------------------

void Renderer::resize(int w, int h)
{
    screenWidth  = w;
    screenHeight = h;
    glViewport(0, 0, w, h);

    scene3D->Resize(w, h);
    sceneHUD->Resize(w, h);

    LOGI("Renderer::resize %d x %d", w, h);
}

void Renderer::syncHudToScene()
{
    // HUD owns the live speed/zoom values (Part 6/7); Scene3D owns the
    // behaviour. This is the one place that reads from one and pushes into
    // the other, per the handout's "HUD reports input, scene owns
    // behaviour" split.
    scene3D->SetFanSpeed(sceneHUD->CurrentSpeed());
    scene3D->SetCameraDistance(sceneHUD->CurrentCameraDistance());
}

void Renderer::render()
{
    syncHudToScene();

    glClearColor(0.05f, 0.07f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    scene3D->Render();                 // fan must occlude itself correctly

    glDisable(GL_DEPTH_TEST);
    sceneHUD->Render();                // HUD must never hide behind "closer" 3D geometry

    glEnable(GL_DEPTH_TEST);           // restore state for next frame's Scene3D pass
}

// ---------------------------------------------------------------------------

void Renderer::TouchEventDown(float x, float y)
{
    // HUD gets first refusal - if it consumes the click, the click must not
    // also trigger a 3D pick (Part 5 -> Part 8 hand-off).
    if (sceneHUD->TouchDown(x, y)) return;
    scene3D->PickAt(x, y);
}

void Renderer::TouchEventMove(float /*x*/, float /*y*/)
{
    // No drag behaviour in Quiz 3 (HUD buttons are discrete presses; the old
    // drag-to-boost gesture was removed from Fan along with the tap toggle).
}

void Renderer::TouchEventRelease(float /*x*/, float /*y*/)
{
    sceneHUD->TouchRelease();
}
