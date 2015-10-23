#include "../render.h"
#include "../render_gl.h"

void bl_render_lib_initialize() {
}

void bl_render_lib_finalize() {
}

BLRenderContext* bl_render_context_create(BLRenderContextCreateAttr* attr) {
  return bl_render_gl_context_create(attr);
}

void bl_render_context_destroy(BLRenderContext* ctx) {
  bl_render_gl_context_destroy(ctx);
}

void bl_render_set_vsync(BLRenderContext* ctx, bool enabled) {
  bl_render_gl_context_set_vsync(ctx, enabled);
}

void bl_render_present(BLRenderContext* ctx) {
  bl_render_gl_context_present(ctx);
}
