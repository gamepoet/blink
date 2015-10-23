#pragma once

struct BLRenderContextCreateAttr;
struct BLRenderContext;

BLRenderContext* bl_render_gl_context_create(BLRenderContextCreateAttr* attr);
void bl_render_gl_context_destroy(BLRenderContext* ctx);
void bl_render_gl_context_set_vsync(BLRenderContext* ctx, bool enabled);

void bl_render_gl_context_present(BLRenderContext* ctx);

void bl_render_gl_context_lock(BLRenderContext* ctx);
void bl_render_gl_context_unlock(BLRenderContext* ctx);
