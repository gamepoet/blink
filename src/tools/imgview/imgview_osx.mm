// Copyright (c) 2012, Ben Scott.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#import <QuartzCore/QuartzCore.h>
#include <OpenGL/glu.h>
#include "imgview_osx.h"
#include <event2/event.h>
#include <json/json.h>
#include <squish.h>
#include <tiffio.h>
#include <blink/gr.h>
#include <bayeux.h>

#define BASEURI "http://localhost:5000"

extern void issue_invalidate_rect(Plugin* __restrict plugin, float x, float y, float w, float h);
extern void url_get(Plugin* __restrict plugin, const char * __restrict url, void* context);

enum EURLRequestType {
  URL_REQUEST_TYPE_TEXTURE_BULK,
  URL_REQUEST_TYPE_TEXTURE_META,
};

struct URLRequestContext {
  EURLRequestType type;
  void*           layer;
};


struct MemFile {
  char* beg;
  char* cur;
  char* end;
};

struct TexMeta {
  int         height;
  int         width;
  const char* file_id;
};


//
// local functions
//

//------------------------------------------------------------------------------
static void log(const char* format, ...) {
  char format2[256];
  snprintf(format2, 256, "%s\n", format);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format2, args);
  va_end(args);
}

//------------------------------------------------------------------------------
static void check_gl() {
  GLenum err = glGetError();
  while (err != GL_NO_ERROR) {
    log("GL ERROR: 0x%x: %s\n", err, gluErrorString(err));
    err = glGetError();
  }
}

//------------------------------------------------------------------------------
static tsize_t mem_file_read(thandle_t handle, tdata_t buf, tsize_t size) {
  MemFile* file = (MemFile*)handle;
  tsize_t max_size = (tsize_t)(file->end - file->cur);
  tsize_t read_size = size > max_size ? max_size : size;
  memcpy(buf, file->cur, read_size);
  file->cur += read_size;
  return read_size;
}

//------------------------------------------------------------------------------
static tsize_t mem_file_write(thandle_t handle, tdata_t buf, tsize_t size) {
  log("TIFF write not implemented");
  return 0;
}

//------------------------------------------------------------------------------
static toff_t mem_file_seek(thandle_t handle, toff_t offset, int whence) {
  MemFile* file = (MemFile*)handle;
  char* new_cur = file->cur;
  switch (whence) {
    case SEEK_SET:
      new_cur = file->beg + offset;
      break;
    case SEEK_CUR:
      new_cur = file->cur + offset;
      break;
    case SEEK_END:
      new_cur = file->end + offset;
      break;
  }

  if (new_cur > file->end) {
    return -1;
  }
  file->cur = new_cur;
  return (toff_t)(new_cur - file->beg);
}

//------------------------------------------------------------------------------
static int mem_file_close(thandle_t handle) {
  return 0;
}

//------------------------------------------------------------------------------
static toff_t mem_file_size(thandle_t handle) {
  MemFile* file = (MemFile*)handle;
  return (toff_t)(file->end - file->beg);
}

//------------------------------------------------------------------------------
static int mem_file_map(thandle_t handle, tdata_t* buf, toff_t* size) {
  MemFile* file = (MemFile*)handle;
  *buf = (tdata_t*)file->beg;
  *size = (toff_t)(file->end - file->beg);
  return 0;
}

//------------------------------------------------------------------------------
static void mem_file_unmap(thandle_t handle, tdata_t buf, toff_t size) {
}


//------------------------------------------------------------------------------
static bool load_texture(BLGrTex** __restrict tex, const TexMeta* meta, void* buffer, int bufferSize, const char* __restrict filename) {
//  MemFile file;
//  file.beg  = (char *)buffer;
//  file.cur  = file.beg;
//  file.end  = file.beg + bufferSize;
//
//  TIFF* tiff = TIFFClientOpen(filename,
//                              "r",
//                              &file,
//                              &mem_file_read,
//                              &mem_file_write,
//                              &mem_file_seek,
//                              &mem_file_close,
//                              &mem_file_size,
//                              &mem_file_map,
//                              &mem_file_unmap);
//  if (!tiff) {
//    return false;
//  }
//
//  uint32_t width, height;
//  TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
//  TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
//
//  uint32_t* __restrict raw_surface = (uint32_t*)malloc(width * height * sizeof(uint32_t));
//  if (!TIFFReadRGBAImageOriented(tiff, width, height, raw_surface)) {
//    free(raw_surface);
//    TIFFClose(tiff);
//    return false;
//  }
//  TIFFClose(tiff);
//  
//  // alloc space for the compressed image
//  uint32 dxt_bytes = squish::GetStorageRequirements(width, height, squish::kDxt5);
//  int tex_file_buf_size = sizeof(BLGrTexFileHeader) + dxt_bytes;
//  void* tex_file_buf = bl_alloc(tex_file_buf_size, 16);
//  BLGrTexFileHeader* header = (BLGrTexFileHeader*)tex_file_buf;
//  header->magic     = BL_TO_BIG_ENDIAN(BL_GR_TEX_FILE_MAGIC);
//  header->version   = BL_TO_BIG_ENDIAN(BL_GR_TEX_FILE_VERSION);
//  header->endian    = BL_TO_BIG_ENDIAN(BL_GR_TEX_FILE_ENDIAN_CURRENT);
//  header->width     = width;
//  header->height    = height;
//  header->levels    = 1;
//  header->format    = BL_GR_TEX_FILE_FORMAT_DXT5;
//  header->reserved  = 0;
//
//  squish::u8* __restrict dxt_surface = (squish::u8*)(header + 1);
//  squish::CompressImage((squish::u8*)raw_surface, width, height, dxt_surface, squish::kDxt5);
//  free(raw_surface);
//  
//  *tex = bl_gr_tex_load((const char*)tex_file_buf, tex_file_buf_size);

  
  // alloc space for the compressed image
  int tex_file_buf_size = sizeof(BLGrTexFileHeader) + bufferSize;
  void* tex_file_buf = bl_alloc(tex_file_buf_size, 16);
  BLGrTexFileHeader* header = (BLGrTexFileHeader*)tex_file_buf;
  header->magic     = BL_TO_BIG_ENDIAN(BL_GR_TEX_FILE_MAGIC);
  header->version   = BL_TO_BIG_ENDIAN(BL_GR_TEX_FILE_VERSION);
  header->endian    = BL_TO_BIG_ENDIAN(BL_GR_TEX_FILE_ENDIAN_CURRENT);
  header->width     = meta->width;
  header->height    = meta->height;
  header->levels    = 1;
  header->format    = BL_GR_TEX_FILE_FORMAT_DXT5;
  header->reserved  = 0;
  memcpy(header + 1, buffer, bufferSize);
  *tex = bl_gr_tex_load((const char*)tex_file_buf, tex_file_buf_size);
  bl_free(tex_file_buf);
  if (*tex) {
    return true;
  }
  else {
//    bl_free(tex_file_buf);
    return true;
  }
}


//
// GLLayer
//

@interface GLLayer : CAOpenGLLayer {
  Plugin* m_plugin;
  bool m_visible;
  float m_width;
  float m_height;

  BLGrTex* m_tex;

  NSString* m_texFilename;
  uint32_t m_texFileId;

  void* m_bulk;
  int m_bulkSize;
  bool m_texChanged;
  
  struct event_base* m_ev_base;
  BayeuxClient* m_pubsub;
  
  TexMeta m_tex_meta;
}

- (Plugin*) getPlugin;
- (void) setTexFilename:(const char*)filename
                 fileId:(uint32_t)fileId;
- (void) setTexBulk:(const void*)data
               size:(int)size;
- (void) setTexMeta:(const TexMeta*)meta;
@end

@implementation GLLayer

//------------------------------------------------------------------------------
static void url_get(GLLayer* layer, EURLRequestType req_type, const char* format, ...) {
  URLRequestContext* ctx = (URLRequestContext*)malloc(sizeof(URLRequestContext));
  ctx->type = req_type;
  ctx->layer = [layer retain];
  
  const char* base_uri = BASEURI;
  char fmt[256];
  snprintf(fmt, sizeof(fmt), "%s%s", base_uri, format);
  fmt[sizeof(fmt) - 1] = 0;
  
  char uri[256];
  va_list args;
  va_start(args, format);
  vsnprintf(uri, sizeof(uri), fmt, args);
  va_end(args);
  
  url_get([layer getPlugin], uri, ctx);
}

//------------------------------------------------------------------------------
static void on_evt_session(const char* msg, void* ctx) {
  GLLayer* layer = (GLLayer*)ctx;
  log("/session: %s", msg);
  
  json_object* json = json_tokener_parse(msg);
//  log("session: %s", json_object_to_json_string(json));
  
  // get updated selection
  json_object* jsonTexFilename = json_object_object_get(json, "selection");
  const char * texFilename = json_object_get_string(jsonTexFilename);
  json_object* jsonTexFileId = json_object_object_get(json, "selection_id");
  const char * texFileIdStr = json_object_get_string(jsonTexFileId);
  uint32_t texFileId = (uint32_t)strtoul(texFileIdStr, NULL, 16);
  
  [layer setTexFilename:texFilename fileId:texFileId];
  
  json_object_put(json);
}

//------------------------------------------------------------------------------
static void on_url_texture_bulk(GLLayer* layer, const char* data, int data_size) {
  [layer setTexBulk:data size:data_size];
}

//------------------------------------------------------------------------------
static void on_url_texture_meta(GLLayer* layer, const char* data, int data_size) {
  json_object* msg = json_tokener_parse(data);
  log("texture_meta: %s", json_object_to_json_string(msg));

  json_object* json_metadata  = json_object_object_get(msg, "metadata");
  json_object* json_target    = json_object_object_get(json_metadata, "target");
  json_object* json_default   = json_object_object_get(json_target, "default");
  json_object* json_height    = json_object_object_get(json_default, "height");
  json_object* json_width     = json_object_object_get(json_default, "width");
  json_object* json_file_ids  = json_object_object_get(msg, "file_ids");
  json_object* json_osx_x64   = json_object_object_get(json_file_ids, "osx_x64");
  json_object* json_file_id   = json_object_object_get(json_osx_x64, "$oid");
  
  TexMeta meta;
  meta.height   = json_object_get_int(json_height);
  meta.width    = json_object_get_int(json_width);
  meta.file_id  = json_object_get_string(json_file_id);
  
  [layer setTexMeta:&meta];

  json_object_put(msg);
}

//------------------------------------------------------------------------------
- (id) init {
  if ((self = [super init]) != NULL) {
    self.opaque = YES;
    self.asynchronous = YES;
    self.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
    self.needsDisplayOnBoundsChange = YES;

    m_tex = NULL;
    m_texFilename = NULL;
    m_texFileId = 0;
    m_texChanged = false;

    m_bulk = NULL;
    m_bulkSize = 0;
    
    m_ev_base = NULL;
    m_pubsub = NULL;

//    sleep(15000);
  }
  return self;
}

//------------------------------------------------------------------------------
- (void) dealloc {
  if (m_pubsub) {
    bayeux_client_destroy(m_pubsub);
    m_pubsub = NULL;
  }
  
  [super dealloc];
}

//------------------------------------------------------------------------------
- (void) drawInCGLContext:(CGLContextObj)ctx
              pixelFormat:(CGLPixelFormatObj)pf
             forLayerTime:(CFTimeInterval)t
              displayTime:(const CVTimeStamp *)ts {
  // dispatch events
  if (m_ev_base) {
    event_base_loop(m_ev_base, EVLOOP_NONBLOCK);
  }

  CGLSetCurrentContext(ctx);

  glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
  glClearDepth(1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, (GLsizei)m_width, (GLsizei)m_height);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, m_width, m_height, 0, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);

  static float angle = 0.0f;
  glRotatef(0.4f * sin(angle), 0.0f, 0.0f, 1.0f);
  angle += 0.1f;

  if (m_bulk && m_texChanged) {
    if (m_tex) {
      bl_gr_tex_unload(m_tex);
      m_tex = NULL;
    }
    if (!load_texture(&m_tex, &m_tex_meta, m_bulk, m_bulkSize, [m_texFilename UTF8String])) {
      m_tex = NULL;
    }
    else {
      m_texChanged = false;
    }
  }
  
  bl_gr_process_commands();

  if (m_tex) {
    glEnable(GL_TEXTURE_2D);
    bl_gr_tex_bind(m_tex);
  }

  glBegin(GL_TRIANGLE_STRIP);
  {
    const float x = 10.0f;
    const float y = 10.0f;
    const float w = m_width - 20.0f;
    const float h = m_height - 20.0f;

    // top right
    glColor3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x+w, y, 0.5f);

    // top left
    glColor3f(0.0, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x, y, 0.5f);

    // bottom right
    glColor3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x+w, y+h, 0.5f);

    // bottom left
    glColor3f(1.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x, y+h, 0.5f);
  }
  glEnd();

  if (m_tex) {
    bl_gr_tex_unbind(m_tex);
    glDisable(GL_TEXTURE_2D);
  }

  check_gl();
  issue_invalidate_rect(m_plugin, 0, 0, m_width, m_height);
  [super drawInCGLContext:ctx pixelFormat:pf forLayerTime:t displayTime:ts];
}

//------------------------------------------------------------------------------
- (BOOL) canDrawInCGLContext:(CGLContextObj)ctx
                 pixelFormat:(CGLPixelFormatObj)pf
                forLayerTime:(CFTimeInterval)t
                 displayTime:(const CVTimeStamp*)ts {
  return m_visible;
}

//------------------------------------------------------------------------------
- (Plugin*) getPlugin {
  return m_plugin;
}

//------------------------------------------------------------------------------
- (void) setTexFilename:(const char*)filename
                 fileId:(uint32_t)fileId {
  NSString* texFilename = [[NSString alloc] initWithUTF8String:filename];
  if ((m_texFilename == NULL) || ![texFilename isEqualToString:m_texFilename]) {
    m_texFilename = texFilename;
    
    m_texFileId = fileId;

    url_get(self, URL_REQUEST_TYPE_TEXTURE_META, "/assets/texture/%08x", m_texFileId);
  }
}

//------------------------------------------------------------------------------
- (void) setTexBulk:(const void*)data
               size:(int)size {
  free(m_bulk);
  m_bulk = NULL;
  m_bulkSize = 0;
  if (size > 0) {
    m_bulk = malloc(size);
    memcpy(m_bulk, data, size);
    m_bulkSize = size;
  }
  m_texChanged = true;
}

//------------------------------------------------------------------------------
- (void) setTexMeta:(const TexMeta *)meta {
  m_tex_meta.height   = meta->height;
  m_tex_meta.width    = meta->width;
  m_tex_meta.file_id  = strdup(meta->file_id);
  
  // clear the bulk data so it doesn't get loaded with this new metadata
  [self setTexBulk:NULL size:0];
  
  // fetch the bulk data
  url_get(self, URL_REQUEST_TYPE_TEXTURE_BULK, "/bulk/%s", m_tex_meta.file_id);
}

//------------------------------------------------------------------------------
- (void) setPlugin:(Plugin*)val {
  m_plugin = val;
}

//------------------------------------------------------------------------------
- (void) setEventBase:(struct event_base*)ev_base {
  m_ev_base = ev_base;
  if (m_pubsub) {
    bayeux_client_destroy(m_pubsub);
    m_pubsub = NULL;
  }
  if (m_ev_base) {
    BayeuxClientOpts opts;
    bayeux_client_opts_defaults(&opts);
    opts.ev_base = m_ev_base;
    m_pubsub = bayeux_client_create("http://localhost:5000/faye", &opts);
    
    bayeux_client_subscribe(m_pubsub, "/session", &on_evt_session, self);
  }
}

//------------------------------------------------------------------------------
- (void) setVisible:(BOOL)val {
  m_visible = val;
}

//------------------------------------------------------------------------------
- (void) setWidth:(float)width
           height:(float)height {
  m_width = width;
  m_height = height;
}

@end


//
// exported functions
//

//------------------------------------------------------------------------------
void* core_anim_layer_create(Plugin* plugin, struct event_base* ev_base) {
  GLLayer* layer = [[GLLayer layer] retain];
  [layer setPlugin:plugin];
  [layer setEventBase:ev_base];

  NSDictionary * actions = [NSDictionary dictionaryWithObjectsAndKeys:
                            [NSNull null], @"contents",
                            nil];
  [layer setActions:actions];
  return layer;
}

//------------------------------------------------------------------------------
void core_anim_layer_destroy(void* layer) {
  GLLayer* gl_layer = (GLLayer*)layer;
  [gl_layer release];
}

//------------------------------------------------------------------------------
void core_anim_layer_set_visible(void* layer, bool visible) {
  GLLayer* gl_layer = (GLLayer*)layer;
  [gl_layer setVisible:visible];
}

//------------------------------------------------------------------------------
void core_anim_layer_render(void* layer) {
  GLLayer* gl_layer = static_cast<GLLayer*>(layer);
  [gl_layer setNeedsDisplay];
}

//------------------------------------------------------------------------------
void core_anim_layer_set_dims(void* layer, float width, float height) {
  GLLayer* gl_layer = static_cast<GLLayer*>(layer);
  [gl_layer setWidth:width height:height];
}

//------------------------------------------------------------------------------
void core_anim_layer_url_ready(void* layer, const char* data, int data_size, void* context) {
  GLLayer* gl_layer = static_cast<GLLayer*>(layer);
  URLRequestContext* ctx = (URLRequestContext*)context;

  switch (ctx->type) {
    case URL_REQUEST_TYPE_TEXTURE_BULK:
      on_url_texture_bulk(gl_layer, data, data_size);
      break;
      
    case URL_REQUEST_TYPE_TEXTURE_META:
      on_url_texture_meta(gl_layer, data, data_size);
      break;
      
    default:
      log("unknown request type: %d", ctx->type);
      break;
  }
  
  [gl_layer release];
  free(ctx);
}

