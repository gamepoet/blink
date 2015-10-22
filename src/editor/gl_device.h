#pragma once

struct BLGlContext;
struct BLGlSurface;

BLGlContext* bl_gl_context_create(BLGlSurface* surface);
void bl_gl_context_destroy(BLGlContext* ctx);
void bl_gl_context_set_vsync(BLGlContext* ctx, bool enabled);

void bl_gl_context_present(BLGlContext* ctx);

void bl_gl_context_lock(BLGlContext* ctx);
void bl_gl_context_unlock(BLGlContext* ctx);
