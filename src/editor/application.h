#pragma once

struct BLRenderContext;

void application_init();
void application_shutdown();
void application_update();

void application_set_gl_context(BLRenderContext* ctx);
