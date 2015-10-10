#include <include/cef_app.h>
#include "../bl_cef_app.h"
#include "../bl_cef_client.h"
#include "bl_editor_app_delegate.h"

@implementation BLEditorAppDelegate

//@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  CefMainArgs args;
  CefRefPtr<BLCefApp> app(new BLCefApp);

  CefSettings settings;
  CefInitialize(args, settings, app.get(), NULL);

  CefWindowInfo info;
  CefBrowserSettings browser_settings;
  CefRefPtr<CefClient> client(new BLCefClient);

  NSView* view = [[self window] contentView];
  info.SetAsChild(view, 0, 0, [view frame].size.width, [view frame].size.height);
  CefBrowserHost::CreateBrowser(info, client.get(), "http://google.com", browser_settings, NULL);

  CefRunMessageLoop();
  CefShutdown();
}

- (void)applicationWillTerminate:(NSNotification*)notification {
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
  return YES;
}

@end
