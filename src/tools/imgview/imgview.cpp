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

#include <npapi.h>
#include <npfunctions.h>
#include <json/json.h>
#include "imgview_osx.h"

//
// types
//

struct Plugin {
  NPP       npp;
  NPWindow  window;
  void*     ca_layer;
  bool      is_invalidating_core_anim_model;
};

struct Stream {
  char*     beg;
  char*     cur;
  char*     end;
  
  void*     context;
};


//
// local vars
//

static NPNetscapeFuncs * s_browser;


//
// forward declarations
//

extern "C" NPError NP_GetEntryPoints(NPPluginFuncs* funcs);
extern "C" NPError NP_Initialize(NPNetscapeFuncs* funcs);
extern "C" NPError NP_Shutdown();


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
static void render(NPP npp, NPCocoaEvent* __restrict evt) {
  Plugin* __restrict plugin = (Plugin*)npp->pdata;
  core_anim_layer_render(plugin->ca_layer);
}


//
// exported functions
//

//------------------------------------------------------------------------------
void issue_invalidate_rect(Plugin* __restrict plugin, float x, float y, float w, float h) {
  if (plugin->is_invalidating_core_anim_model) {
    NPRect rect;
    rect.left = (uint16_t)x;
    rect.top = (uint16_t)y;
    rect.right = (uint16_t)w;
    rect.bottom = (uint16_t)h;
    s_browser->invalidaterect(plugin->npp, &rect);
  }
}

//------------------------------------------------------------------------------
void url_get(Plugin* __restrict plugin, const char* __restrict url, void* context) {
//  log("url_get: %s", url);
  Stream* __restrict s = (Stream*)malloc(sizeof(Stream));
  memset(s, 0, sizeof(Stream));
  s->context = context;
  
  NPError err = s_browser->geturlnotify(plugin->npp, url, NULL, s);
  if (err != NPERR_NO_ERROR) {
    log("request failed: GET %s", url);
  }
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_ClearSiteData(const char* site, uint64_t flags, uint64_t maxAge) {
  log(__FUNCTION__);
  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NP_GetEntryPoints(NPPluginFuncs* funcs) {
  if (!funcs) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }
  if (funcs->size == 0) {
    funcs->size = sizeof(NPPluginFuncs);
  }
  else if (funcs->size < sizeof(NPPluginFuncs)) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }

  funcs->version            = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
  funcs->size               = sizeof(NPPluginFuncs);
  funcs->newp               = NPP_New;
  funcs->destroy            = NPP_Destroy;
  funcs->setwindow          = NPP_SetWindow;
  funcs->newstream          = NPP_NewStream;
  funcs->destroystream      = NPP_DestroyStream;
  funcs->asfile             = NPP_StreamAsFile;
  funcs->writeready         = NPP_WriteReady;
  funcs->write              = NPP_Write;
  funcs->print              = NPP_Print;
  funcs->event              = NPP_HandleEvent;
  funcs->urlnotify          = NPP_URLNotify;
  funcs->javaClass          = 0;
  funcs->getvalue           = NPP_GetValue;
  funcs->setvalue           = NPP_SetValue;
  funcs->gotfocus           = NPP_GotFocus;
  funcs->lostfocus          = NPP_LostFocus;
  funcs->urlredirectnotify  = NPP_URLRedirectNotify;
  funcs->clearsitedata      = NPP_ClearSiteData;
  funcs->getsiteswithdata   = NPP_GetSitesWithData;

  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" char** NPP_GetSitesWithData() {
  log(__FUNCTION__);
  char** sites = (char **)s_browser->memalloc(sizeof(char*) * 1);
  sites[0] = NULL;
  return sites;
}

//------------------------------------------------------------------------------
extern "C" NPBool NPP_GotFocus(NPP instance, NPFocusDirection direction) {
  log(__FUNCTION__);
  return false;
}

//------------------------------------------------------------------------------
extern "C" NPError NP_Initialize(NPNetscapeFuncs* funcs) {
  log(__FUNCTION__);

  if (!funcs) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }
  if ((funcs->version >> 8) > NP_VERSION_MAJOR) {
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }

  s_browser = funcs;

  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" void NPP_LostFocus(NPP instance) {
  log(__FUNCTION__);
}

//------------------------------------------------------------------------------
extern "C" NPError NP_Shutdown() {
  log(__FUNCTION__);
  s_browser = NULL;

  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_Destroy(NPP instance, NPSavedData** save) {
  log(__FUNCTION__);
  if (!instance || !instance->pdata) {
    log("bad instance");
    return NPERR_INVALID_INSTANCE_ERROR;
  }

  Plugin* plugin = (Plugin*)instance->pdata;
  if (plugin) {
    if (plugin->ca_layer) {
      core_anim_layer_destroy(plugin->ca_layer);
    }
    free(plugin);
    instance->pdata = NULL;
  }

  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason) {
//  log(__FUNCTION__);
  Stream* __restrict s = (Stream*)stream->notifyData;
  if (!s) {
    // didn't create this stream
    return NPERR_NO_ERROR;
  }

  // free stream unless successful, otherwise let urlnotify free it
  if (reason != NPRES_DONE) {
    free(s->beg);
    free(s);
  }
  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_GetValue(NPP instance, NPPVariable variable, void* value) {
  log("%s var=%d", __FUNCTION__, variable);

  Plugin* plugin = (Plugin*)instance->pdata;

  if (variable == NPPVpluginCoreAnimationLayer) {
    if (!plugin->ca_layer) {
      return NPERR_GENERIC_ERROR;
    }
    *(void**)value = plugin->ca_layer;
    return NPERR_NO_ERROR;
  }

  return NPERR_GENERIC_ERROR;
}

//------------------------------------------------------------------------------
extern "C" int16_t NPP_HandleEvent(NPP instance, void* event) {
  if (!instance || !instance->pdata) {
    return 0;
  }

  NPCocoaEvent* evt = (NPCocoaEvent*)event;
  if (!evt) {
    return 0;
  }

  switch (evt->type) {
    case NPCocoaEventDrawRect:
      log("%s: NPCocoaEventDrawRect", __FUNCTION__);
      render(instance, evt);
      return 1;

    case NPCocoaEventMouseDown:
      log("%s: NPCocoaEventMouseDown", __FUNCTION__);
      break;

    case NPCocoaEventMouseUp:
      log("%s: NPCocoaEventMouseUp", __FUNCTION__);
      break;

    case NPCocoaEventMouseMoved:
//      log("%s: NPCocoaEventMouseMoved", __FUNCTION__);
      break;

    case NPCocoaEventMouseEntered:
//      log("%s: NPCocoaEventMouseEntered", __FUNCTION__);
      break;

    case NPCocoaEventMouseExited:
//      log("%s: NPCocoaEventMouseExited", __FUNCTION__);
      break;

    case NPCocoaEventMouseDragged:
      log("%s: NPCocoaEventMouseDragged", __FUNCTION__);
      break;

    case NPCocoaEventKeyDown:
      log("%s: NPCocoaEventKeyDown", __FUNCTION__);
      break;

    case NPCocoaEventKeyUp:
      log("%s: NPCocoaEventKeyUp", __FUNCTION__);
      break;

    case NPCocoaEventFlagsChanged:
      log("%s: NPCocoaEventFlagsChanged", __FUNCTION__);
      break;

    case NPCocoaEventFocusChanged:
      log("%s: NPCocoaEventFocusChanged", __FUNCTION__);
      break;

    case NPCocoaEventWindowFocusChanged:
      log("%s: NPCocoaEventWindowFocusChanged", __FUNCTION__);
      break;

    case NPCocoaEventScrollWheel:
      log("%s: NPCocoaEventScrollWheel", __FUNCTION__);
      break;

    case NPCocoaEventTextInput:
      log("%s: NPCocoaEventTextInput", __FUNCTION__);
      break;

    default:
      log("%s: <unknown>", __FUNCTION__);
      break;
  }

  return 0;
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved) {
  log(__FUNCTION__);
  if (!instance) {
    log("bad instance");
    return NPERR_INVALID_INSTANCE_ERROR;
  }

  Plugin* plugin = (Plugin*)malloc(sizeof(Plugin));
  if (!plugin) {
    log("out of memory");
    return NPERR_OUT_OF_MEMORY_ERROR;
  }

  memset(plugin, 0, sizeof(Plugin));
  plugin->npp = instance;
  instance->pdata = plugin;

  // query browser caps
  NPError err;
  NPBool supports_core_animation;
  NPBool supports_cocoa;
  NPBool supports_invalidating_core_animation;
  err = s_browser->getvalue(instance, NPNVsupportsCoreAnimationBool, &supports_core_animation);
  if (err != NPERR_NO_ERROR) {
    supports_core_animation = false;
  }
  err = s_browser->getvalue(instance, NPNVsupportsCocoaBool, &supports_cocoa);
  if (err != NPERR_NO_ERROR) {
    supports_cocoa = false;
  }
  err = s_browser->getvalue(instance, NPNVsupportsInvalidatingCoreAnimationBool, &supports_invalidating_core_animation);
  if (err != NPERR_NO_ERROR) {
    supports_invalidating_core_animation = false;
  }
//  log("invalidating core anim? %s", supports_invalidating_core_animation ? "yes" : "no");

  // validate browser caps
  if (!(supports_core_animation || supports_invalidating_core_animation) || !supports_cocoa) {
    free(plugin);
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }
  plugin->is_invalidating_core_anim_model = supports_invalidating_core_animation;

  // negotiate drawing and event models
  if (supports_invalidating_core_animation) {
    err = s_browser->setvalue(instance, NPPVpluginDrawingModel, (void*)NPDrawingModelInvalidatingCoreAnimation);
    if (err != NPERR_NO_ERROR) {
      log("Failed to set InvalidatingCoreAnimation drawing model.");
      free(plugin);
      return NPERR_INCOMPATIBLE_VERSION_ERROR;
    }
  }
  else {
    err = s_browser->setvalue(instance, NPPVpluginDrawingModel, (void*)NPDrawingModelCoreAnimation);
    if (err != NPERR_NO_ERROR) {
      log("Failed to set CoreAnimation drawing model.");
      free(plugin);
      return NPERR_INCOMPATIBLE_VERSION_ERROR;
    }
  }
  err = s_browser->setvalue(instance, NPPVpluginEventModel, (void*)NPEventModelCocoa);
  if (err != NPERR_NO_ERROR) {
    log("Failed to set Cocoa event model.");
    free(plugin);
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }

  // create the core animation layer
  plugin->ca_layer = core_anim_layer_create(plugin);
  
  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) {
//  log("%s end=%d mime=%s", __FUNCTION__, stream->end, type);
//  log("headers\n%s", stream->headers);
  
  Stream* __restrict s = (Stream*)stream->notifyData;
  if (!s) {
    // did not request this stream
    return NPERR_NO_ERROR;
  }

  s->beg = (char*)malloc(stream->end);
  s->cur = s->beg;
  s->end = s->beg + stream->end;
  
  *stype = NP_NORMAL;
  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" void NPP_Print(NPP instance, NPPrint* platformPrint) {
  log(__FUNCTION__);
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_SetValue(NPP instance, NPNVariable variable, void* value) {
  log(__FUNCTION__);
  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_SetWindow(NPP instance, NPWindow* window) {
//  log(__FUNCTION__);
  if (!instance || !instance->pdata) {
    return NPERR_INVALID_INSTANCE_ERROR;
  }
  Plugin* plugin = (Plugin*)instance->pdata;
  plugin->window = *window;
//  log("  window: %08x", window->window);
//  log("  x: %d", window->x);
//  log("  y: %d", window->y);
//  log("  w: %d", window->width);
//  log("  h: %d", window->height);
//  log("  clip: l=%d, t=%d, r=%d, b=%d", window->clipRect.left, window->clipRect.top, window->clipRect.right, window->clipRect.bottom);
  core_anim_layer_set_dims(plugin->ca_layer, window->width, window->height);

  // use a zero-sized clip rect to determine if the window is visible
  uint16_t clip_w, clip_h;
  clip_w = window->clipRect.right - window->clipRect.left;
  clip_h = window->clipRect.bottom - window->clipRect.top;
  if (clip_w == 0 || clip_h == 0) {
    core_anim_layer_set_visible(plugin->ca_layer, false);
  }
  else {
    core_anim_layer_set_visible(plugin->ca_layer, true);
  }
  
  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname) {
  log(__FUNCTION__);
}

//------------------------------------------------------------------------------
extern "C" void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {
//  log(__FUNCTION__);

  Stream* __restrict s = (Stream*)notifyData;
  if (!s) {
    // didn't create this stream
    return;
  }

  Plugin* __restrict plugin = (Plugin*)instance->pdata;
  if (!plugin) {
    return;
  }

  core_anim_layer_url_ready(plugin->ca_layer, s->beg, s->cur - s->beg, s->context);

  free(s->beg);
  free(s);
}

//------------------------------------------------------------------------------
extern "C" void NPP_URLRedirectNotify(NPP instance, const char* url, int32_t status, void* notifyData) {
  log(__FUNCTION__);
}

//------------------------------------------------------------------------------
extern "C" int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer) {
//  log("%s off=%d, len=%d", __FUNCTION__, offset, len);
  Stream* __restrict s = (Stream*)stream->notifyData;
  if (!s) {
    // didn't create this stream
    return -1;
  }
  
  if (offset != (int32_t)(s->cur - s->beg)) {
    log("  bad offset");
    return 0;
  }
  if (len > (int32_t)(s->end - s->cur)) {
    log("  bad len");
    return 0;
  }
  
  memcpy(s->cur, buffer, len);
  s->cur += len;
  return len;
}

//------------------------------------------------------------------------------
extern "C" int32_t NPP_WriteReady(NPP instance, NPStream* stream) {
//  log(__FUNCTION__);
  Stream* __restrict s = (Stream*)stream->notifyData;
  if (!s) {
    // didn't create this stream
    return -1;
  }

  return (int32_t)(s->end - s->cur);
}

