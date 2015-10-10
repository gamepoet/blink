#pragma once

#include <include/cef_app.h>

class BLCefApp : public CefApp, public CefRenderProcessHandler {
public:

  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
    return this;
  }

  IMPLEMENT_REFCOUNTING(BLCefApp);
};
