#import <Cocoa/Cocoa.h>

struct BLRenderContext;

@interface GlView : NSView {
  BLRenderContext* m_render_context;
}

- (id)initWithFrame:(NSRect)frameRect;
- (id)initWithFrame:(NSRect)frameRect shareContext:(NSOpenGLContext*)context;

@end
