#import <Cocoa/Cocoa.h>

#import "ESScreensaverView.h"

@interface ESController : NSObject {


}

- (void)launchHelp:(id)sender;

@end



@interface ESWindow : NSWindow 
#if defined(MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
<NSWindowDelegate, NSApplicationDelegate>
#endif
{
ESScreensaverView* mESView;

ESWindow *mFullScreenWindow;

ESWindow *mOriginalWindow;

BOOL mIsFullScreen;

BOOL mInSheet;

NSMutableArray *mBlackingWindows;

}

- (void)awakeFromNib;

//- (void)windowWillClose:(NSNotification *)notification;

- (void)showPreferences:(id)sender;

- (void)didEndSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo;

- (void)windowDidResize:(NSNotification *)notification;

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication;

- (BOOL)canBecomeKeyWindow;

- (ESWindow *)originalWindow;

- (void)setOriginalWindow:(ESWindow*)window;

- (BOOL)isFullScreen;

- (void)switchFullScreen:(id)sender;

- (void) blackScreensExcept:(NSScreen*)fullscreen;

- (void) unblackScreens;

- (void) fadeWindow:(NSWindow *)window withEffect:(NSString *)effect;

@end
