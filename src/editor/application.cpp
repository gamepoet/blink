#include <chrono>
#include <blink/asset.h>
#include <blink/io.h>
#include <blink/job.h>
#include "application.h"
#include "gl_device.h"

static BLGlContext* s_gl_context;

static std::chrono::high_resolution_clock::time_point s_time;

void application_init() {
  BLBaseInitAttr attr;
  attr.log_filename = nullptr;
  bl_base_lib_initialize(&attr);
#ifndef NDEBUG
  bl_log_set_level(BL_LOG_LEVEL_DEBUG);
#endif

  BLJobLibInitParams params;
  params.worker_thread_count = 2;
  bl_job_lib_initialize(&params);

  bl_io_lib_initialize();
  bl_asset_lib_initialize();

  // init time to the recent past
  s_time = std::chrono::high_resolution_clock::now() - std::chrono::milliseconds(16);
}

void application_shutdown() {
  bl_asset_lib_finalize();
  bl_io_lib_finalize();
  bl_job_lib_finalize();
  bl_base_lib_finalize();
}

void application_update() {
  std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
  bl_log_info("dt (ms): %f", std::chrono::duration<float, std::milli>(now - s_time).count());
  float dt = std::chrono::duration<float, std::ratio<1,1>>(now - s_time).count();
  s_time = now;

  bl_log_info("dt ( s): %f", dt);

  if (s_gl_context) {
//    glClearColor(0.4f, 0.4f, 0.8f, 0.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bl_gl_context_present(s_gl_context);

//    check_gl_error();
  }
}

void application_set_gl_context(BLGlContext* ctx) {
  s_gl_context = ctx;
}
