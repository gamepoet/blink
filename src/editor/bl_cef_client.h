#pragma once

#include <include/cef_client.h>

class BLCefClient : public CefClient {
public:
  IMPLEMENT_REFCOUNTING(BLCefClient);
};
