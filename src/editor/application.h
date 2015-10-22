#pragma once

struct BLGlContext;

void application_init();
void application_shutdown();
void application_update();

void application_set_gl_context(BLGlContext* ctx);
