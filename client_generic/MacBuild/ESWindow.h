#import <Cocoa/Cocoa.h>

#import "ESScreensaverView.h"

@interface ESController : NSObject {


}

- (void)launchHelp:(id)sender;

@end



#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6 
@interface ESWindow : NSWindow <NSWindowDelegate, NSApplicationDelegate> {
#else
@interface ESWindow : NSWindow {
#endif
ESScreensaverView* mESView;

ESWindow *mFullScreenWindow;

ESWindow *mOriginalWindow;

BOOL mIsFullScreen;

BOOL mInSheet;

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

- (void)toggleFullScreen:(id)sender;


@end
