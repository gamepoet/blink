#include <chrono>
#include <string.h>
#include <mongoose.h>
#include <blink/asset.h>
#include <blink/io.h>
#include <blink/job.h>
#include <blink/render.h>
#include "application.h"

static mg_mgr s_server;
static BLRenderContext* s_render_context;

static std::chrono::high_resolution_clock::time_point s_time;

static void mg_str_to_cstr(char* dest, size_t dest_size, const mg_str* src) {
  size_t copy_bytes = src->len;
  if (copy_bytes > dest_size - 1) {
    copy_bytes = dest_size - 1;
  }
  memcpy(dest, src->p, dest_size);
  dest[copy_bytes] = 0;
}

static void log_request(http_message* req) {
  char method[8];
  char path[1024];
  mg_str_to_cstr(method, sizeof(method), &req->method);
  mg_str_to_cstr(path, sizeof(path), &req->uri);

  bl_log_info("%s %s", method, path);
}

class Profiler {
public:
  void begin(const char* name) {
    m_name = name;
    m_beg = std::chrono::high_resolution_clock::now();
  }
  void end() {
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(now - m_beg).count();
    bl_log_info("[profile] %0.5f %s", ms, m_name);
  }

private:
  const char* m_name;
  std::chrono::high_resolution_clock::time_point m_beg;
};

static void send_frame_buffer(mg_connection* conn) {
  if (!s_render_context) {
    return;
  }

  Profiler profiler;

  int width, height;
  bl_render_get_dimensions(s_render_context, &width, &height);

  profiler.begin("read_frame_buffer");
  int buf_size = width * height * 4;
  char* buf = (char*)bl_alloc(buf_size, 64);
  bl_render_read_frame_buffer(s_render_context, width, height, buf);
  profiler.end();

  profiler.begin("send_websocket_frame");
  mg_send_websocket_frame(conn, WEBSOCKET_OP_BINARY, buf, buf_size);
  profiler.end();

  bl_free(buf);
}

static void server_on_evt(mg_connection* conn, int evt, void* evt_data) {
  http_message* msg_http = (http_message*)evt_data;
  websocket_message* msg_ws = (websocket_message*)evt_data;

  mg_serve_http_opts http_opts;
  memset(&http_opts, 0, sizeof(http_opts));
  http_opts.document_root = "/Users/bscott/dev/blink/src/editor/web";

  switch (evt) {
    case MG_EV_HTTP_REQUEST:
      log_request(msg_http);

      mg_serve_http(conn, msg_http, http_opts);
      conn->flags |= MG_F_SEND_AND_CLOSE;
      break;

    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
      bl_log_info("got new websocket connection");
      break;

    case MG_EV_WEBSOCKET_FRAME:
      bl_log_info("new websocket message (len=%d)", msg_ws->size);
      if (msg_ws->size == 5) {
        if (0 == memcmp(msg_ws->data, "frame", 5)) {
          send_frame_buffer(conn);
        }
        else {
          bl_log_error("unknown websocket message (len=%d)", msg_ws->size);
        }
      }
      else {
        bl_log_error("unknown websocket message (len=%d)", msg_ws->size);
      }
      break;

    case MG_EV_CLOSE:
      if (conn->flags & MG_F_IS_WEBSOCKET) {
        bl_log_info("disconnect (websocket)");
      }
      else {
        bl_log_info("disconnect");
      }
      break;

    case MG_EV_POLL:
//      if (conn->flags & MG_F_IS_WEBSOCKET) {
//        send_frame_buffer(conn);
//      }
      // ignore this spam
      break;

    default:
//      bl_log_info("unexpected mongoose evt: %d", evt);
      break;
  }
}

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
  bl_render_lib_initialize();

  // init time to the recent past
  s_time = std::chrono::high_resolution_clock::now() - std::chrono::milliseconds(16);

  // create an http server that wants to accept websocket connections
  mg_mgr_init(&s_server, nullptr);
  mg_connection* conn = mg_bind(&s_server, ":8080", &server_on_evt);
  mg_set_protocol_http_websocket(conn);
}

void application_shutdown() {
  mg_mgr_free(&s_server);

  bl_render_lib_finalize();
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

//  bl_log_info("dt ( s): %f", dt);

  mg_mgr_poll(&s_server, 0);

  if (s_render_context) {
//    glClearColor(0.4f, 0.4f, 0.8f, 0.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bl_render_present(s_render_context);
  }
}

void application_set_gl_context(BLRenderContext* ctx) {
  s_render_context = ctx;
}
