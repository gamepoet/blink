#import <OpenGL/glu.h>
#include "gl_view.h"
#include "../gl_device.h"
#include "../application.h"
#include <blink/base.h>

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

  if (!m_gl_context) {
    m_gl_context = bl_gl_context_create((__bridge_retained BLGlSurface*)self);

    bl_gl_context_set_vsync(m_gl_context, false);

    // look for changes in view size
    // note: -reshape will not be called automatically on size changes
    // because NSView does not export it to override
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(reshape)
                                                 name:NSViewGlobalFrameDidChangeNotification
                                               object:self];

    application_set_gl_context(m_gl_context);
  }
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self
                                                  name:NSViewGlobalFrameDidChangeNotification
                                                object:self];

  bl_gl_context_destroy(m_gl_context);
}

- (void)reshape {
  bl_gl_context_lock(m_gl_context);

//  [m_gl_context makeCurrentContext];

//  [m_gl_context update];

  bl_gl_context_unlock(m_gl_context);
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

@end
