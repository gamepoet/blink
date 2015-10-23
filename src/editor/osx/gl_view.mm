#import <OpenGL/glu.h>
#include "gl_view.h"
#include "../application.h"
#include <blink/base.h>
#include <blink/render.h>
#include <blink/render_gl.h>

@implementation GlView

- (id)initWithFrame:(NSRect)frameRect shareContext:(NSOpenGLContext*)context {
  bl_log_info("initWithFrame");
  self = [super initWithFrame:frameRect];
  return self;
}

- (id)initWithFrame:(NSRect)frameRect {
  self = [self initWithFrame:frameRect shareContext:nil];
  return self;
}

- (void)lockFocus {
  bl_log_info("lockFocus");
  [super lockFocus];

  if (!m_render_context) {
    BLRenderContextCreateAttr attr;
    attr.surface = (__bridge_retained BLGlSurface*)self;
    attr.vsync = false;
    m_render_context = bl_render_context_create(&attr);

    // look for changes in view size
    // note: -reshape will not be called automatically on size changes
    // because NSView does not export it to override
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(reshape)
                                                 name:NSViewGlobalFrameDidChangeNotification
                                               object:self];

    application_set_gl_context(m_render_context);
  }
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self
                                                  name:NSViewGlobalFrameDidChangeNotification
                                                object:self];

  bl_render_context_destroy(m_render_context);
}

- (void)reshape {
  bl_render_gl_context_lock(m_render_context);

//  [m_render_context makeCurrentContext];

//  [m_render_context update];

  bl_render_gl_context_unlock(m_render_context);
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

@end
