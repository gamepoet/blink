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
#include <syslog.h>
#include "imgview_osx.h"

//
// types
//

struct Instance {
  NPP       npp;
  NPWindow  window;
  void*     ca_layer;
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
  Instance* __restrict obj = (Instance*)npp->pdata;
  core_anim_layer_render(obj->ca_layer);
}


//
// exported functions
//

//------------------------------------------------------------------------------
extern "C" NPError NPP_ClearSiteData(const char* site, uint64_t flags, uint64_t maxAge) {
  log(__FUNCTION__);
  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NP_GetEntryPoints(NPPluginFuncs* funcs) {
  log(__FUNCTION__);
//  sleep(100000);
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
//  sleep(100000);

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

  Instance* obj = (Instance*)instance->pdata;
  if (obj) {
    if (obj->ca_layer) {
      core_anim_layer_destroy(obj->ca_layer);
    }
    free(obj);
    instance->pdata = NULL;
  }

  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason) {
  log(__FUNCTION__);
  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_GetValue(NPP instance, NPPVariable variable, void* value) {
  log("%s var=%d", __FUNCTION__, variable);

  Instance* obj = (Instance*)instance->pdata;

  if (variable == NPPVpluginCoreAnimationLayer) {
    if (!obj->ca_layer) {
      return NPERR_GENERIC_ERROR;
    }
    *(void**)value = obj->ca_layer;
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

  Instance* obj = (Instance*)instance->pdata;

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
      core_anim_layer_set_visible(obj->ca_layer, evt->data.focus.hasFocus);
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

  Instance* obj = (Instance*)malloc(sizeof(Instance));
  if (!obj) {
    log("out of memory");
    return NPERR_OUT_OF_MEMORY_ERROR;
  }

  memset(obj, 0, sizeof(Instance));
  obj->npp = instance;
  instance->pdata = obj;

  // query browser caps
  NPError err;
  NPBool supports_core_animation;
  NPBool supports_cocoa;
  err = s_browser->getvalue(instance, NPNVsupportsCoreAnimationBool, &supports_core_animation);
  if (err != NPERR_NO_ERROR) {
    supports_core_animation = false;
  }
  err = s_browser->getvalue(instance, NPNVsupportsCocoaBool, &supports_cocoa);
  if (err!= NPERR_NO_ERROR) {
    supports_cocoa = false;
  }

  // validate browser caps
  if (!supports_core_animation || !supports_cocoa) {
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }

  // negotiate drawing and event models
  err = s_browser->setvalue(instance, NPPVpluginDrawingModel, (void*)NPDrawingModelCoreAnimation);
  if (err != NPERR_NO_ERROR) {
    log("Failed to set CoreGraphics drawing model.");
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }
  err = s_browser->setvalue(instance, NPPVpluginEventModel, (void*)NPEventModelCocoa);
  if (err != NPERR_NO_ERROR) {
    log("Failed to set Cocoa event model.");
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }

  // create the core animation layer
  obj->ca_layer = core_anim_layer_create();
  log("created core anim layer: %08x", obj->ca_layer);

  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) {
  log(__FUNCTION__);
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
  log(__FUNCTION__);
  if (!instance || !instance->pdata) {
    return NPERR_INVALID_INSTANCE_ERROR;
  }
  Instance* cur_instance = (Instance*)instance->pdata;
  cur_instance->window = *window;
  log("  window: %08x", window->window);
  log("  x: %d", window->x);
  log("  y: %d", window->y);
  log("  w: %d", window->width);
  log("  h: %d", window->height);
  log("  clip: l=%d, t=%d, r=%d, b=%d", window->clipRect.left, window->clipRect.top, window->clipRect.right, window->clipRect.bottom);
  return NPERR_NO_ERROR;
}

//------------------------------------------------------------------------------
extern "C" void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname) {
  log(__FUNCTION__);
}

//------------------------------------------------------------------------------
extern "C" void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {
  log(__FUNCTION__);
}

//------------------------------------------------------------------------------
extern "C" void NPP_URLRedirectNotify(NPP instance, const char* url, int32_t status, void* notifyData) {
  log(__FUNCTION__);
}

//------------------------------------------------------------------------------
extern "C" int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer) {
  log(__FUNCTION__);
  return 0;
}

//------------------------------------------------------------------------------
extern "C" int32_t NPP_WriteReady(NPP instance, NPStream* stream) {
  log(__FUNCTION__);
  return 0;
}
