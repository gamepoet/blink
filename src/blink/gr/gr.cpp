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

#include <blink/gr.h>
#include <QuartzCore/QuartzCore.h>
#include <OpenGL/glu.h>
#include <vector>


//
// constants
//

//#define BL_GR_TEX_FILE_MAGIC            0x42544558u // BTEX
//#define BL_GR_TEX_FILE_VERSION          0x00000000u
//#define BL_GR_TEX_FILE_ENDIAN_BIG       0x00000000u
//#define BL_GR_TEX_FILE_ENDIAN_LITTLE    0x00000001u
//#define BL_GR_TEX_FILE_HEIGHT_MAX       (1 << 12)
//#define BL_GR_TEX_FILE_WIDTH_MAX        (1 << 12)
//#define BL_GR_TEX_FILE_LEVELS_MAX       (1 << 4)
//
//#define BL_GR_TEX_FILE_FORMAT_DXT5      0
//#define BL_GR_TEX_FILE_FORMAT_DXT1      1
//// ...
//#define BL_GR_TEX_FILE_FORMAT_MAX       2
//
//#ifdef BL_BIG_ENDIAN
//# define BL_GR_TEX_FILE_ENDIAN_CURRENT  BL_GR_TEX_FILE_ENDIAN_BIG
//#else
//# define BL_GR_TEX_FILE_ENDIAN_CURRENT  BL_GR_TEX_FILE_ENDIAN_LITTLE
//#endif



//
// types
//

struct BLGrTex {
  uint32_t  height  : 12; // height - 1 (since 0 is not valid)
  uint32_t  width   : 12; // width - 1 (since 0 is not valid)
  uint32_t  levels  : 4;  // levels - 1 (since 0 is not valid)
  uint32_t  dxt5    : 1;  // dxt5 vs dxt1
  uint32_t  unused  : 3;

  GLuint    pbo_id;
  GLuint    tex_id;
};

struct BLGrCmdTexUpload {
  BLGrTex*  tex;
  void*     surface;
};


//
// vars
//

static std::vector<BLGrCmdTexUpload*> s_cmd_tex_uploads;


//
// functions
//

//------------------------------------------------------------------------------
static void check_gl() {
  GLenum err = glGetError();
  while (err != GL_NO_ERROR) {
    bl_log_error("GL ERROR: 0x%08x: %s\n", err, gluErrorString(err));
    err = glGetError();
  }
}

//------------------------------------------------------------------------------
static int get_storage_requirements(int width, int height, bool dxt5) {
  int block_count = ((width + 3) / 4) * ((height + 3) / 4);
  int block_size  = dxt5 ? 16 : 8;
  return block_count * block_size;
}

//------------------------------------------------------------------------------
static BLGrTex* tex_alloc() {
  BLGrTex* tex = (BLGrTex*)bl_alloc(sizeof(BLGrTex), 16);
  tex->unused = 0;
  return tex;
}

//------------------------------------------------------------------------------
static void tex_free(BLGrTex* tex) {
  if (tex) {
    bl_free(tex);
  }
}


//
// exported functions
//

//------------------------------------------------------------------------------
void bl_gr_lib_initialize() {
}

//------------------------------------------------------------------------------
void bl_gr_lib_finalize() {
}

//------------------------------------------------------------------------------
void bl_gr_process_commands() {
  // process pending texture uploads
  for (std::vector<BLGrCmdTexUpload*>::iterator itr = s_cmd_tex_uploads.begin(), end = s_cmd_tex_uploads.end(); itr != end; ++itr) {
    BLGrCmdTexUpload* cmd = *itr;
    BLGrTex* tex = cmd->tex;

    // get some gl handles
    glGenTextures(1, &tex->tex_id);
    glGenBuffers(1, &tex->pbo_id);
    check_gl();

    // bind the texture and pbo
    glBindTexture(GL_TEXTURE_2D, tex->tex_id);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex->pbo_id);
    check_gl();

    // upload the texture data to the pbo
    int height, width, surface_size;
    height        = tex->height + 1;
    width         = tex->width + 1;
    surface_size  = get_storage_requirements(width, height, tex->dxt5);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, surface_size, cmd->surface, GL_STATIC_DRAW);
    check_gl();

    // setup tex params
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    check_gl();

    // specify the texture
    GLenum gl_format = tex->dxt5 ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, surface_size, NULL);
    check_gl();

    bl_free(cmd->surface);
    bl_free(cmd);
  }
  s_cmd_tex_uploads.clear();
}

//------------------------------------------------------------------------------
void bl_gr_tex_bind(BLGrTex* tex) {
  BL_ASSERT(tex);
  glBindTexture(GL_TEXTURE_2D, tex->tex_id);
}

//------------------------------------------------------------------------------
void bl_gr_tex_unbind(BLGrTex* tex) {
  BL_ASSERT(tex);
  glBindTexture(GL_TEXTURE_2D, 0);
}

//------------------------------------------------------------------------------
BLGrTex* bl_gr_tex_load(const char* buf, unsigned int buf_size) {
  if (buf_size < sizeof(BLGrTexFileHeader)) {
    bl_log_error("tex data invalid: short read on file header");
    return NULL;
  }
  const BLGrTexFileHeader* header = (const BLGrTexFileHeader*)buf;
  buf += sizeof(BLGrTexFileHeader);
  buf_size -= sizeof(BLGrTexFileHeader);


  int surface_size;
  void* surface;
  BLGrTex* tex;
  BLGrCmdTexUpload* cmd;

#define ERR_CHECK(cond, msg, ...)                           \
  if (!(cond)) {                                            \
    bl_log_error("tex data invalid: " msg, ## __VA_ARGS__); \
    goto error;                                             \
  }                                                         \
  ((void)0)

  // validate header
  uint32_t expected_magic   = BL_FROM_BIG_ENDIAN(BL_GR_TEX_FILE_MAGIC);
  uint32_t expected_version = BL_FROM_BIG_ENDIAN(BL_GR_TEX_FILE_VERSION);
  uint32_t expected_endian  = BL_FROM_BIG_ENDIAN(BL_GR_TEX_FILE_ENDIAN_CURRENT);
  ERR_CHECK(header->magic == expected_magic,              "invalid magic (expected %08x, got %08x)", expected_version, header->magic);
  ERR_CHECK(header->version == expected_version,          "invalid version (expected %08x, got %08x)", expected_version, header->version);
  ERR_CHECK(header->endian == expected_endian,            "invalid endian (expected %08x, got %08x)", expected_endian, header->endian);
  ERR_CHECK(header->height <= BL_GR_TEX_FILE_HEIGHT_MAX,  "height too big (max %d, got %d)", BL_GR_TEX_FILE_HEIGHT_MAX, header->height);
  ERR_CHECK(header->width <= BL_GR_TEX_FILE_WIDTH_MAX,    "width too big (max %d, got %d)", BL_GR_TEX_FILE_WIDTH_MAX, header->width);
  ERR_CHECK(header->levels <= BL_GR_TEX_FILE_LEVELS_MAX,  "levels too big (max %d, got %d)", BL_GR_TEX_FILE_LEVELS_MAX, header->levels);
  ERR_CHECK(header->format < BL_GR_TEX_FILE_FORMAT_MAX,   "unknown format type (got %08x)", header->format);
  ERR_CHECK(header->reserved == 0,                        "invalid reserved data (expected 00000000, got %08x)", header->reserved);

  BL_ASSERT_MSG(header->levels == 1, "mip levels not yet supported");

  // alloc the temporary surface storage for upload
  surface_size = get_storage_requirements(header->width, header->height, header->format == BL_GR_TEX_FILE_FORMAT_DXT5);
  ERR_CHECK(buf_size == surface_size, "file size mismatch");

#undef ERR_CHECK

  // alloc the surface and copy into it
  surface = bl_alloc(surface_size, 16);
  memcpy(surface, buf, surface_size);

  // alloc the texture handle
  tex = tex_alloc();
  tex->height   = header->height - 1;
  tex->width    = header->width - 1;
  tex->levels   = header->levels - 1;
  tex->dxt5     = header->format == BL_GR_TEX_FILE_FORMAT_DXT5 ? 1 : 0;

  // schedule an upload
  // TODO: this should really come from a ring buffer
  cmd = (BLGrCmdTexUpload*)bl_alloc(sizeof(BLGrCmdTexUpload), 16);
  cmd->tex      = tex;
  cmd->surface  = surface;
  s_cmd_tex_uploads.push_back(cmd);

  return tex;

error:
  return NULL;
}

//------------------------------------------------------------------------------
void bl_gr_tex_unload(BLGrTex* tex) {
  BL_ASSERT(tex);
  // TODO: unload from gfx
}
