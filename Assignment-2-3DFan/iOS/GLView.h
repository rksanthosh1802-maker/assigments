#pragma once
#import <UIKit/UIKit.h>

/**
 * GLView
 *
 * A UIView subclass that hosts an OpenGL ES 3.0 rendering surface via
 * EAGLContext + CAEAGLLayer + CADisplayLink.
 *
 * Touch events (finger down / move / release) are forwarded to the shared
 * Renderer singleton — Triangle changes colour:
 *   TouchDown    → red
 *   TouchMove    → green
 *   TouchRelease → blue
 *
 * The Scene C++ code is completely unchanged from the Android / Desktop builds.
 */
@interface GLView : UIView

/// Start the 60-fps CADisplayLink render loop.
- (void)startRendering;

/// Pause the render loop (called when the view disappears).
- (void)stopRendering;

@end
