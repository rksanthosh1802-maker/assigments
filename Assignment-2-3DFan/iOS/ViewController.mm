#import "ViewController.h"
#import "GLView.h"

@interface ViewController ()
@property (nonatomic, strong) GLView *glView;
@end

@implementation ViewController

- (void)loadView {
    // Use GLView as the root view — it owns the EAGLContext and CAEAGLLayer.
    self.glView = [[GLView alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.view = self.glView;
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    [self.glView startRendering];
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
    [self.glView stopRendering];
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
    return UIInterfaceOrientationMaskAllButUpsideDown;
}

@end
