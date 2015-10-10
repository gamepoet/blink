#import <Cocoa/Cocoa.h>

/*
#include <stdio.h>
#include <include/capi/cef_app_capi.h>
// receives notifications from the application
@interface EditorAppDelegate : NSObject<NSApplicationDelegate>
- (void)createApplication:(id)object;
- (void)tryToTerminateApplication:(NSApplication*)app;

@implementation EditorAppDelegate

- (void)createApplication:(id)object {
  printf("createApplication\n");
  [NSApplication sharedApplication];
  [NSBundle loadNibNamed]:@"MainMenu" owner:NSApp];

  // create the app
  [[NSApplication sharedApplication] setDelegate:self];
}

- (void)tryToTerminateApplication:(NSApplication*)app {
  printf("tryToTerminateApplication\n");
}

//- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
//  return NSTerminateNow;
//}


int main(int argc, char** argv) {
  cef_main_args_t args;
  args.argc = argc;
  args.argv = argv;

  cef_app_t app = { 0 };
  app.base.size = sizeof(cef_app_t);

  cef_settings_t settings = { 0 };
  settings.size = sizeof(cef_settings_t);

  printf("initialize\n");
  int ret = cef_initialize(&args, &settings, &app, nullptr);
  if (!ret) {
    return 1;
  }

  // create the app delegate
  NSObject* delegate = [[EditorAppDelegate alloc] init];
  [delegate performSelectorOnMainThread:@selector(createApplication:)
                             withObject:nil
                          waitUntilDone:NO];

  // blocks until cef_quit_message_loop() is called
  printf("run_message_loop\n");
  cef_run_message_loop();

  printf("shutdown\n");
  cef_shutdown();

  // cleanup
  [delegate release];

  return 0;
}
*/

int main(int argc, char** argv) {
  return NSApplicationMain(argc, (const char**)argv);
}
