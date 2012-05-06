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


//
// GLLayer
//

@interface GLLayer : CAOpenGLLayer {
  bool m_visible;
}
@end

@implementation GLLayer

//------------------------------------------------------------------------------
- (id) init {
  log(__FUNCTION__);
  if ((self = [super init]) != NULL) {
    self.asynchronous = YES;
  }
  return self;
}

//------------------------------------------------------------------------------
- (void) dealloc {
  log(__FUNCTION__);
  [super dealloc];
}

//------------------------------------------------------------------------------
- (void) drawInCGLContext:(CGLContextObj)ctx
              pixelFormat:(CGLPixelFormatObj)pf
             forLayerTime:(CFTimeInterval)t
              displayTime:(const CVTimeStamp *)ts {
//  log(__FUNCTION__);
  CGLSetCurrentContext(ctx);

  glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  [super drawInCGLContext:ctx pixelFormat:pf forLayerTime:t displayTime:ts];
}

//------------------------------------------------------------------------------
- (BOOL) canDrawInCGLContext:(CGLContextObj)ctx
                 pixelFormat:(CGLPixelFormatObj)pf
                forLayerTime:(CFTimeInterval)t
                 displayTime:(const CVTimeStamp*)ts {
//  log(__FUNCTION__);
  return m_visible;
}

//------------------------------------------------------------------------------
- (void) setVisible:(BOOL)val {
  m_visible = val;
}

@end


//
// exported functions
//

//------------------------------------------------------------------------------
void* core_anim_layer_create() {
  log(__FUNCTION__);
  GLLayer* layer = [[GLLayer layer] retain];
  layer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
  layer.needsDisplayOnBoundsChange = YES;
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
  log(__FUNCTION__);
  GLLayer* gl_layer = static_cast<GLLayer*>(layer);
  [gl_layer setNeedsDisplay];
}
