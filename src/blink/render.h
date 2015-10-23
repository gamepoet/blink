#pragma once
#ifndef BL_RENDER_H
#define BL_RENDER_H

#include <blink/base.h>

struct BLRenderContext;
struct BLRenderContextCreateAttr;

#ifdef BL_PLATFORM_OSX
struct BLGlSurface;
struct BLRenderContextCreateAttr {
  BLGlSurface*  surface;
  bool          vsync;
};
#endif

void bl_render_lib_initialize();
void bl_render_lib_finalize();

BLRenderContext* bl_render_context_create(BLRenderContextCreateAttr* attr);
void bl_render_context_destroy(BLRenderContext* ctx);

void bl_render_set_vsync(BLRenderContext* ctx, bool enabled);

void bl_render_present(BLRenderContext* ctx);

#endif
