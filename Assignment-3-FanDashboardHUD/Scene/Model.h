#pragma once

/**
 * Model.h
 *
 * Abstract base class for all renderable objects.
 * Platform-agnostic – shared by Android, Desktop, and WebGL builds.
 */

enum ModelType {
    TriangleType = 0,
    CubeType     = 1
};

class Model {
public:
    Model() {}
    virtual ~Model() {}

    /** Load shaders, create GPU buffers, etc. */
    virtual void InitModel() = 0;

    /** Called every frame to draw this model. */
    virtual void Render() = 0;

    /** Optional per-frame update. */
    virtual void Update(float dt) {}

    /** Called when the viewport changes. */
    virtual void Resize(int w, int h) {}

    virtual ModelType GetModelType() { return modelType; }

    // Touch / mouse event interface
    virtual void TouchEventDown(float x, float y)    {}
    virtual void TouchEventMove(float x, float y)    {}
    virtual void TouchEventRelease(float x, float y) {}

protected:
    ModelType modelType = TriangleType;
};
