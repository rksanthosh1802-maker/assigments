// GLView.mm — OpenGL ES 3.0 rendering surface for the TouchEvents demo (iOS).
//
// Design:
//   • CAEAGLLayer backing + EAGLContext(ES3) + CADisplayLink at 60 fps.
//   • chdir(bundlePath) before Renderer::initialise() so ShaderHelper's
//     std::ifstream paths ("assets/shader/TriangleVertex.glsl", …) resolve
//     to the shader files Xcode copied into the app bundle.
//   • Touch begin/move/end → Renderer::Instance().TouchEventDown/Move/Release()
//     The Triangle then responds with colour changes (red/green/blue) — the same
//     logic that runs on Android and Desktop.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"   // OpenGL ES deprecated iOS 12+

#import "GLView.h"
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <unistd.h>

// --- C++ Scene headers ---------------------------------------------------
#define PLATFORM_IOS
#include "../Scene/Renderer.h"
// -------------------------------------------------------------------------

@interface GLView () {
    EAGLContext   *_context;
    CADisplayLink *_displayLink;

    GLuint _framebuffer;
    GLuint _colorRenderbuffer;
    GLuint _depthRenderbuffer;
}
@end

@implementation GLView

+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (!self) return nil;

    [self p_setupLayer];
    [self p_setupContext];

    // Point the working directory at the app bundle so ShaderHelper can open
    // "assets/shader/TriangleVertex.glsl" etc. with a plain std::ifstream.
    NSString *bundlePath = [NSBundle mainBundle].bundlePath;
    chdir([bundlePath UTF8String]);

    // Initialise the shared C++ renderer (loads shaders, builds VAO/VBO).
    Renderer::Instance().initialise();

    return self;
}

// -----------------------------------------------------------------------
// Private setup
// -----------------------------------------------------------------------

- (void)p_setupLayer {
    CAEAGLLayer *layer = (CAEAGLLayer *)self.layer;
    layer.opaque = YES;
    layer.drawableProperties = @{
        kEAGLDrawablePropertyRetainedBacking : @NO,
        kEAGLDrawablePropertyColorFormat     : kEAGLColorFormatRGBA8,
    };
    self.contentScaleFactor = UIScreen.mainScreen.nativeScale;
}

- (void)p_setupContext {
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    NSAssert(_context, @"OpenGL ES 3.0 is not available on this device.");
    [EAGLContext setCurrentContext:_context];
}

- (void)p_buildFramebuffer {
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    // Color
    glGenRenderbuffers(1, &_colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER
                     fromDrawable:(CAEAGLLayer *)self.layer];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, _colorRenderbuffer);

    // Depth
    GLint w = 0, h = 0;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH,  &w);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);

    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, _depthRenderbuffer);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    NSAssert(status == GL_FRAMEBUFFER_COMPLETE,
             @"Framebuffer incomplete: 0x%04X", status);

    Renderer::Instance().resize(w, h);
}

- (void)p_destroyFramebuffer {
    glDeleteFramebuffers(1,  &_framebuffer);       _framebuffer       = 0;
    glDeleteRenderbuffers(1, &_colorRenderbuffer); _colorRenderbuffer = 0;
    glDeleteRenderbuffers(1, &_depthRenderbuffer); _depthRenderbuffer = 0;
}

// -----------------------------------------------------------------------
// Layout — rebuild the framebuffer whenever bounds change (rotation etc.)
// -----------------------------------------------------------------------

- (void)layoutSubviews {
    [super layoutSubviews];
    [EAGLContext setCurrentContext:_context];

    if (_framebuffer) {
        [self p_destroyFramebuffer];
    }
    [self p_buildFramebuffer];
}

// -----------------------------------------------------------------------
// Render loop
// -----------------------------------------------------------------------

- (void)startRendering {
    if (_displayLink) return;
    _displayLink = [CADisplayLink displayLinkWithTarget:self
                                              selector:@selector(p_renderFrame:)];
    if (@available(iOS 15.0, *)) {
        _displayLink.preferredFrameRateRange = CAFrameRateRangeMake(30, 60, 60);
    }
    [_displayLink addToRunLoop:[NSRunLoop mainRunLoop]
                       forMode:NSRunLoopCommonModes];
}

- (void)stopRendering {
    [_displayLink invalidate];
    _displayLink = nil;
}

- (void)p_renderFrame:(CADisplayLink *)sender {
    [EAGLContext setCurrentContext:_context];
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    Renderer::Instance().render();

    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [_context presentRenderbuffer:GL_RENDERBUFFER];
}

// -----------------------------------------------------------------------
// Touch forwarding — maps iOS UITouch to C++ Renderer touch interface.
// The Triangle model responds:
//   TouchEventDown    → red
//   TouchEventMove    → green
//   TouchEventRelease → blue
// -----------------------------------------------------------------------

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    UITouch *t  = touches.anyObject;
    CGPoint  pt = [t locationInView:self];
    float scale = (float)self.contentScaleFactor;
    Renderer::Instance().TouchEventDown((float)pt.x * scale,
                                        (float)pt.y * scale);
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    UITouch *t  = touches.anyObject;
    CGPoint  pt = [t locationInView:self];
    float scale = (float)self.contentScaleFactor;
    Renderer::Instance().TouchEventMove((float)pt.x * scale,
                                        (float)pt.y * scale);
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    UITouch *t  = touches.anyObject;
    CGPoint  pt = [t locationInView:self];
    float scale = (float)self.contentScaleFactor;
    Renderer::Instance().TouchEventRelease((float)pt.x * scale,
                                           (float)pt.y * scale);
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self touchesEnded:touches withEvent:event];
}

// -----------------------------------------------------------------------
// Cleanup
// -----------------------------------------------------------------------

- (void)dealloc {
    [self stopRendering];
    [EAGLContext setCurrentContext:_context];
    [self p_destroyFramebuffer];
    [EAGLContext setCurrentContext:nil];
}

@end

#pragma clang diagnostic pop
