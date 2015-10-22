#import <Cocoa/Cocoa.h>

struct BLGlContext;

@interface GlView : NSView {
  BLGlContext* m_gl_context;
}

- (id)initWithFrame:(NSRect)frameRect;
- (id)initWithFrame:(NSRect)frameRect shareContext:(NSOpenGLContext*)context;

@end
