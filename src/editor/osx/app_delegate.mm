#include <thread>
#include "../application.h"
#import "app_delegate.h"

static char s_app_thread_memory[sizeof(std::thread)];
static std::thread* s_app_thread;
static bool s_app_thread_quit;

static void app_thread_proc() {
  pthread_setname_np("bl_app");
  application_init();
  while (!s_app_thread_quit) {
    application_update();
  }
  application_shutdown();
}

static void app_thread_init() {
  s_app_thread_quit = false;
  s_app_thread = new (s_app_thread_memory) std::thread(&app_thread_proc);
}

static void app_thread_shutdown() {
  s_app_thread_quit = true;
  s_app_thread->join();
  s_app_thread->~thread();
  s_app_thread = nullptr;
}

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
  app_thread_init();
}

- (void)applicationWillTerminate:(NSNotification*)aNotification {
  app_thread_shutdown();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
  return true;
}

@end
