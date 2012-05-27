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
#include <json/json.h>
#include <squish.h>
#include <tiffio.h>

extern void issue_invalidate_rect(Plugin* __restrict plugin, float x, float y, float w, float h);
extern void url_get(Plugin* __restrict plugin, const char * __restrict url, void* context);


struct MemFile {
  char* beg;
  char* cur;
  char* end;
};

struct Tex {
  uint32_t  height  : 12; // height - 1 (since 0 is not valid)
  uint32_t  width   : 12; // width - 1 (since 0 is not valid)
  uint32_t  levels  : 4;  // levels - 1 (since 0 is not valid)
  uint32_t  dxt5    : 1;  // dxt5 vs dxt1
  uint32_t  unused  : 3;

  void*     surface;
  GLuint    pbo_id;
  GLuint    tex_id;
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
static bool load_texture(Tex* __restrict tex, const char* __restrict filename) {
  FILE* __restrict fh = fopen(filename, "rb");
  if (!fh) {
    return false;
  }
  fseek(fh, 0, SEEK_END);
  long size = ftell(fh);
  fseek(fh, 0, SEEK_SET);
  char* __restrict buf = (char*)malloc(size);
  long bytes_read = fread(buf, 1, size, fh);
  if (bytes_read != size) {
    free(buf);
    fclose(fh);
    return false;
  }
  fclose(fh);

  MemFile file;
  file.beg = buf;
  file.cur = buf;
  file.end = buf + size;
  TIFF* tiff = TIFFClientOpen(filename,
                              "r",
                              &file,
                              &mem_file_read,
                              &mem_file_write,
                              &mem_file_seek,
                              &mem_file_close,
                              &mem_file_size,
                              &mem_file_map,
                              &mem_file_unmap);
  if (!tiff) {
    free(buf);
    return false;
  }

  uint32_t width, height;
  TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);

  uint32_t* __restrict raw_surface = (uint32_t*)malloc(width * height * sizeof(uint32_t));
  if (!TIFFReadRGBAImageOriented(tiff, width, height, raw_surface)) {
    free(raw_surface);
    TIFFClose(tiff);
    free(buf);
    return false;
  }
  TIFFClose(tiff);
  free(buf);

  // alloc space for the compressed image
  uint32 dxt_bytes = squish::GetStorageRequirements(width, height, squish::kDxt5);
  squish::u8* __restrict dxt_surface = (squish::u8*)malloc(dxt_bytes);
  squish::CompressImage((squish::u8*)raw_surface, width, height, dxt_surface, squish::kDxt5);
  free(raw_surface);

  tex->height   = height - 1;
  tex->width    = width - 1;
  tex->levels   = 1 - 1;
  tex->dxt5     = 1;
  tex->unused   = 0;
  tex->surface  = dxt_surface;
  tex->pbo_id   = 0;
  tex->tex_id   = 0;

  return true;
}


//
// GLLayer
//

@interface GLLayer : CAOpenGLLayer {
  Plugin* m_plugin;
  bool m_visible;
  float m_width;
  float m_height;

  Tex m_tex;

  NSString * m_texFilename;
  bool m_texFilenameChanged;
  
  NSTimer * m_timer;
}
@end

@implementation GLLayer

//------------------------------------------------------------------------------
- (id) init {
  if ((self = [super init]) != NULL) {
    self.opaque = YES;
    self.asynchronous = YES;
    self.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
    self.needsDisplayOnBoundsChange = YES;
    
    m_timer = [NSTimer timerWithTimeInterval:1.0f target:self selector:@selector(syncSession:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:m_timer forMode:NSDefaultRunLoopMode];
    
    m_texFilename = NULL;
    m_texFilenameChanged = false;
    
//    sleep(15000);
  }
  return self;
}

//------------------------------------------------------------------------------
- (void) syncSession:(NSTimer*)timer {
  url_get(m_plugin, "http://localhost:9292/session", self);
}

//------------------------------------------------------------------------------
- (void) dealloc {
  [super dealloc];
}

//------------------------------------------------------------------------------
- (void) drawInCGLContext:(CGLContextObj)ctx
              pixelFormat:(CGLPixelFormatObj)pf
             forLayerTime:(CFTimeInterval)t
              displayTime:(const CVTimeStamp *)ts {
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
  
  if (!m_tex.surface || m_texFilenameChanged) {
    if (m_tex.surface) {
      free(m_tex.surface);
      m_tex.surface = NULL;
    }
    char path[256];
    snprintf(path, 256, "/Users/bscott/dev/blink/src/tools/imgview/%s", [m_texFilename UTF8String]);
    if (!load_texture(&m_tex, path)) {
      m_tex.surface = NULL;
    }
    else {
      uint32_t w = m_tex.width + 1;
      uint32_t h = m_tex.height + 1;
      int surface_size = squish::GetStorageRequirements(w, h, m_tex.dxt5 ? squish::kDxt5 : squish::kDxt1);
      glGenTextures(1, &m_tex.tex_id);
      glGenBuffers(1, &m_tex.pbo_id);
      check_gl();

      // bind the texture and pbo
      glBindTexture(GL_TEXTURE_2D, m_tex.tex_id);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_tex.pbo_id);
      check_gl();

      // upload texture data to the pbo
      glBufferData(GL_PIXEL_UNPACK_BUFFER, surface_size, m_tex.surface, GL_STATIC_DRAW);
      check_gl();
      
      // setup tex params
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      check_gl();
      
      glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, w, h, 0, surface_size, NULL);
      check_gl();
      
      m_texFilenameChanged = false;
    }
  }

  if (m_tex.tex_id) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_tex.tex_id);
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
  
  if (m_tex.tex_id) {
    glBindTexture(GL_TEXTURE_2D, 0);
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
- (void) setTexFilename:(const char*)filename {
  NSString* texFilename = [[NSString alloc] initWithUTF8String:filename];
  if ((m_texFilename == NULL) || ![texFilename isEqualToString:m_texFilename]) {
    m_texFilename = texFilename;
    m_texFilenameChanged = true;
  }
}

//------------------------------------------------------------------------------
- (void) setPlugin:(Plugin*)val {
  m_plugin = val;
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
void* core_anim_layer_create(Plugin* plugin) {
  GLLayer* layer = [[GLLayer layer] retain];
  [layer setPlugin:plugin];

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
  
  // parse json string
  json_object* json = json_tokener_parse(data);
  log("%s", json_object_to_json_string(json));
  
  // get updated selection
  json_object* jsonTexFilename = json_object_object_get(json, "selection");
  const char * texFilename = json_object_get_string(jsonTexFilename);
  
  [gl_layer setTexFilename:texFilename];
  
  json_object_put(json);
}

