#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#include <blink/base.h>
#include "../render.h"
#include "../render_gl.h"

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

static NSOpenGLContext* to_ns_context(BLRenderContext* ctx) {
  return (__bridge NSOpenGLContext*)ctx;
}

BLRenderContext* bl_render_gl_context_create(BLRenderContextCreateAttr* attr) {
  NSView* view = (__bridge_transfer NSView*)attr->surface;

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
  BLRenderContext* ctx = (__bridge_retained BLRenderContext*)gl_ctx;

  bl_render_gl_context_set_vsync(ctx, attr->vsync);

  return ctx;
}

void bl_render_gl_context_destroy(BLRenderContext* ctx) {
  // release the gl context
  NSOpenGLContext* gl_ctx = (__bridge_transfer NSOpenGLContext*)ctx;
  gl_ctx = nil;
}

void bl_render_gl_context_set_vsync(BLRenderContext* ctx, bool enabled) {
  NSOpenGLContext* gl_ctx = to_ns_context(ctx);
  GLint swapInt = enabled ? 1 : 0;
  [gl_ctx setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

void bl_render_gl_context_present(BLRenderContext* ctx) {
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

void bl_render_gl_context_get_dimensions(BLRenderContext* ctx, int* width, int* height) {
  *width = 100;
  *height = 100;
}

void bl_render_gl_context_read_frame_buffer(BLRenderContext* ctx, int width, int height, void* dest) {
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, dest);
}

void bl_render_gl_context_lock(BLRenderContext* ctx) {
  NSOpenGLContext* gl_ctx = to_ns_context(ctx);
  CGLContextObj cgl_ctx = (CGLContextObj)[gl_ctx CGLContextObj];
  CGLLockContext(cgl_ctx);
}

void bl_render_gl_context_unlock(BLRenderContext* ctx) {
  NSOpenGLContext* gl_ctx = to_ns_context(ctx);
  CGLContextObj cgl_ctx = (CGLContextObj)[gl_ctx CGLContextObj];
  CGLUnlockContext(cgl_ctx);
}
