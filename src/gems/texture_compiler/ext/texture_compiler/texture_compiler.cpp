/*
 Copyright (c) 2012, Ben Scott.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include <ruby.h>
#include <tiffio.h>
#include <squish.h>

/*
 * constants
 */

typedef enum _EImgFormat {
  IMG_FORMAT_UNKNOWN = 0,
  IMG_FORMAT_DXT1,
  IMG_FORMAT_DXT5,
  IMG_FORMAT_RGBA8888,
} EImgFormat;

typedef struct _Image {
  EImgFormat        format;
  unsigned int      height;
  unsigned int      width;
  VALUE             surface_str;
} Image;

static void image_free(void* data);

/*
 * local vars
 */

static ID     s_id_read;
static ID     s_id_seek;
static ID     s_id_tell;

static VALUE  s_class_Image;
static VALUE  s_module_IO;
static VALUE  s_SEEK_CUR;
static VALUE  s_SEEK_END;
static VALUE  s_SEEK_SET;


/*
 * local functions
 */

/*----------------------------------------------------------------------------*/
static tsize_t io_read(thandle_t handle, tdata_t buf, tsize_t size) {
  VALUE* fh = (VALUE*)handle;
  VALUE result = rb_funcall(*fh, s_id_read, 1, INT2NUM(size));
  if (NIL_P(result)) {
    rb_warn("  read failed");
    return 0;
  }

  long len    = RSTRING_LEN(result);
  char* data  = RSTRING_PTR(result);
  memcpy(buf, data, len);

  return (tsize_t)len;
}

/*----------------------------------------------------------------------------*/
static tsize_t io_write(thandle_t handle, tdata_t buf, tsize_t size) {
//  log("TIFF write not implemented");
  return 0;
}

/*----------------------------------------------------------------------------*/
static toff_t io_seek(thandle_t handle, toff_t offset, int whence) {
  VALUE* fh = (VALUE*)handle;

  VALUE amount = INT2NUM(offset);
  VALUE whence_val;
  switch (whence) {
    case SEEK_SET:
      whence_val = s_SEEK_SET;
      break;
    case SEEK_CUR:
      whence_val = s_SEEK_CUR;
      break;
    case SEEK_END:
      whence_val = s_SEEK_END;
      break;
  }

  rb_funcall(*fh, s_id_seek, 2, amount, whence_val);
  VALUE pos = rb_funcall(*fh, s_id_tell, 0);
  return NUM2INT(pos);
}

/*----------------------------------------------------------------------------*/
static int io_close(thandle_t handle) {
  return 0;
}

/*----------------------------------------------------------------------------*/
static toff_t io_size(thandle_t handle) {
  VALUE* fh = (VALUE*)handle;

  VALUE loc = rb_funcall(*fh, s_id_tell, 0);
  VALUE size = rb_funcall(*fh, s_id_seek, 2, NUM2INT(0), s_SEEK_END);
  rb_funcall(*fh, s_id_seek, 2, loc, s_SEEK_SET);

  return (toff_t)INT2NUM(loc);
}

/*----------------------------------------------------------------------------*/
static int io_map(thandle_t handle, tdata_t* buf, toff_t* size) {
  *buf = NULL;
  *size = 0;
  return 0;
}

/*----------------------------------------------------------------------------*/
static void io_unmap(thandle_t handle, tdata_t buf, toff_t size) {
}


/*
 * Image functions
 */

/*----------------------------------------------------------------------------*/
static void image_free(void* data) {
  Image* img = (Image*)data;
  free(img);
}

/*----------------------------------------------------------------------------*/
static VALUE image_load(VALUE klass, VALUE fh) {
  Image* img = ALLOC(Image);
  img->format       = IMG_FORMAT_UNKNOWN;
  img->height       = 0;
  img->width        = 0;
  img->surface_str  = Qnil;
  VALUE img_val = Data_Wrap_Struct(klass, NULL, &image_free, img);

  TIFF* tiff = TIFFClientOpen("buffer", "r", &fh, &io_read, &io_write, &io_seek, &io_close, &io_size, &io_map, &io_unmap);
  if (!tiff) {
    return Qnil;
  }

  uint32_t width, height;
  TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
  img->width = width;
  img->height = height;

  // alloc space for the raw surface (as a ruby string)
  VALUE surface_str;
  surface_str = rb_str_new("", 0);
  rb_str_resize(surface_str, width * height * sizeof(uint32_t));

  // load the image from disk
  uint32_t* __restrict surface_ptr;
  surface_ptr = (uint32_t*)RSTRING_PTR(surface_str);
  if (!TIFFReadRGBAImageOriented(tiff, width, height, surface_ptr, ORIENTATION_BOTLEFT, 0)) {
    TIFFClose(tiff);
    return Qnil;
  }
  TIFFClose(tiff);

  img->surface_str  = surface_str;
  img->format       = IMG_FORMAT_RGBA8888;

  return img_val;
}

/*----------------------------------------------------------------------------*/
static VALUE image_surface(VALUE self) {
  Image* img;
  Data_Get_Struct(self, Image, img);
  return img->surface_str;
}

/*----------------------------------------------------------------------------*/
static VALUE image_to_dxt(VALUE self) {
  Image* src_img;
  Data_Get_Struct(self, Image, src_img);

  // alloc a new image struct
  Image* dst_img        = ALLOC(Image);
  dst_img->format       = IMG_FORMAT_DXT5;
  dst_img->height       = src_img->height;
  dst_img->width        = src_img->width;
  dst_img->surface_str  = Qnil;
  VALUE dst_img_val     = Data_Wrap_Struct(s_class_Image, NULL, &image_free, dst_img);

  // alloc space for the compressed surface
  uint32_t dxt_bytes = squish::GetStorageRequirements(src_img->width, src_img->height, squish::kDxt5);
  VALUE dxt_surface_str = rb_str_new("", 0);
  rb_str_resize(dxt_surface_str, dxt_bytes);

  // do the compression
  squish::u8* __restrict dxt_surface_ptr = (squish::u8*)RSTRING_PTR(dxt_surface_str);
  squish::CompressImage(
    (squish::u8*)RSTRING_PTR(src_img->surface_str),
    src_img->width,
    src_img->height,
    dxt_surface_ptr,
    squish::kDxt5
    );
  dst_img->surface_str  = dxt_surface_str;

  return dst_img_val;
}

/*----------------------------------------------------------------------------*/
static VALUE image_format(VALUE self) {
  Image* img;
  Data_Get_Struct(self, Image, img);
  return INT2NUM(img->format);
}

/*----------------------------------------------------------------------------*/
static VALUE image_height(VALUE self) {
  Image* img;
  Data_Get_Struct(self, Image, img);
  return INT2NUM(img->height);
}

/*----------------------------------------------------------------------------*/
static VALUE image_width(VALUE self) {
  Image* img;
  Data_Get_Struct(self, Image, img);
  return INT2NUM(img->width);
}


/*
 * exported functions
 */

/*----------------------------------------------------------------------------*/
extern "C" void Init_texture_compiler() {
  // lookup globals
  s_id_read       = rb_intern("read");
  s_id_seek       = rb_intern("seek");
  s_id_tell       = rb_intern("tell");
  s_module_IO     = rb_const_get(rb_cObject, rb_intern("IO"));

  s_SEEK_CUR      = rb_const_get(s_module_IO, rb_intern("SEEK_CUR"));
  s_SEEK_END      = rb_const_get(s_module_IO, rb_intern("SEEK_END"));
  s_SEEK_SET      = rb_const_get(s_module_IO, rb_intern("SEEK_SET"));

  /* TextureCompiler module */
  VALUE module_texture_compiler = rb_define_module("TextureCompiler");

  /* Image class */
  s_class_Image = rb_define_class_under(module_texture_compiler, "Image", rb_cObject);
  rb_define_singleton_method(s_class_Image, "load", (VALUE(*)(...))&image_load, 1);
  rb_define_method(s_class_Image, "surface", (VALUE(*)(...))&image_surface, 0);
  rb_define_method(s_class_Image, "to_dxt", (VALUE(*)(...))&image_to_dxt, 0);
  rb_define_method(s_class_Image, "format", (VALUE(*)(...))&image_format, 0);
  rb_define_method(s_class_Image, "height", (VALUE(*)(...))&image_width, 0);
  rb_define_method(s_class_Image, "width", (VALUE(*)(...))&image_width, 0);

  rb_define_const(s_class_Image, "FORMAT_UNKNOWN",  INT2NUM(IMG_FORMAT_UNKNOWN));
  rb_define_const(s_class_Image, "FORMAT_DXT1",     INT2NUM(IMG_FORMAT_DXT1));
  rb_define_const(s_class_Image, "FORMAT_DXT5",     INT2NUM(IMG_FORMAT_DXT5));
  rb_define_const(s_class_Image, "FORMAT_RGBA8888", INT2NUM(IMG_FORMAT_RGBA8888));
}
