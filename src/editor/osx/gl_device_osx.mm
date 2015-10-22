#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#include <blink/base.h>
#include "../gl_device.h"

#define GL_ENSURE(stmt)                                                   \
  do {                                                                    \
    (stmt);                                                               \
    GLenum err = glGetError();                                            \
    if (err != GL_NO_ERROR) {                                             \
      report_gl_errors(err, #stmt);                                       \
    }                                                                     \
  } while (0)

static void report_gl_errors(GLenum err, const char* stmt) {
  while (err != GL_NO_ERROR) {
    const GLubyte* msg = gluErrorString(err);
    bl_log_error("GL ERROR err=%08x msg=%s stmt=%s", err, msg, stmt);
    err = glGetError();
  }
  BL_DEBUG_BREAK();
}

static NSOpenGLContext* to_ns_context(BLGlContext* ctx) {
  return (__bridge NSOpenGLContext*)ctx;
}

BLGlContext* bl_gl_context_create(BLGlSurface* surface) {
  NSView* view = (__bridge_transfer NSView*)surface;

  NSOpenGLPixelFormatAttribute attrs[] = {
    kCGLPFAAccelerated,
    kCGLPFADoubleBuffer,
    kCGLPFANoRecovery,
    kCGLPFAColorSize,     24,
    kCGLPFADepthSize,     16,
    0
  };
  NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
  if (!pixel_format) {
    bl_log_fatal("No OpenGL pixel format.");
  }

  // create the context
  NSOpenGLContext* gl_ctx = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:NULL];
  gl_ctx.view = view;
  [gl_ctx update];

  // apply the new context to the current thread
  [gl_ctx makeCurrentContext];

  // set the initial gl state
  GL_ENSURE(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
  GL_ENSURE(glClear(GL_COLOR_BUFFER_BIT));

  // take ownership of the NSOpenGLContext object from ARC
  BLGlContext* ctx = (__bridge_retained BLGlContext*)gl_ctx;
  return ctx;
}

void bl_gl_context_destroy(BLGlContext* ctx) {
  // release the gl context
  NSOpenGLContext* gl_ctx = (__bridge_transfer NSOpenGLContext*)ctx;
  gl_ctx = nil;
}

void bl_gl_context_set_vsync(BLGlContext* ctx, bool enabled) {
  NSOpenGLContext* gl_ctx = to_ns_context(ctx);
  GLint swapInt = enabled ? 1 : 0;
  [gl_ctx setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

void bl_gl_context_present(BLGlContext* ctx) {
  NSOpenGLContext* gl_ctx = to_ns_context(ctx);

  static bool up = true;
  static float red = 0.0f;
  if (up) {
    red += 0.01f;
    if (red >= 1.0f) {
      red = 1.0f;
      up = false;
    }
  }
  else {
    red -= 0.01f;
    if (red <= 0.0f) {
      red = 0.0f;
      up = true;
    }
  }
  [gl_ctx makeCurrentContext];
  GL_ENSURE(glClearColor(red, 0.0f, 0.0f, 0.0f));
  GL_ENSURE(glClear(GL_COLOR_BUFFER_BIT));

  [gl_ctx flushBuffer];
}

void bl_gl_context_lock(BLGlContext* ctx) {
  NSOpenGLContext* gl_ctx = to_ns_context(ctx);
  CGLContextObj cgl_ctx = (CGLContextObj)[gl_ctx CGLContextObj];
  CGLLockContext(cgl_ctx);
}

void bl_gl_context_unlock(BLGlContext* ctx) {
  NSOpenGLContext* gl_ctx = to_ns_context(ctx);
  CGLContextObj cgl_ctx = (CGLContextObj)[gl_ctx CGLContextObj];
  CGLUnlockContext(cgl_ctx);
}
