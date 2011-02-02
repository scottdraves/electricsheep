#import <ScreenSaver/ScreenSaver.h>
#import "ESOpenGLView.h"
#import "ESConfiguration.h"
#import "Sparkle/Sparkle.h"

@interface ESScreensaverView 
#ifdef SCREEN_SAVER
: ScreenSaverView 
#else
: NSView 
#endif
{
    // So what do you need to make an OpenGL screen saver? Just an NSOpenGLView (or subclass thereof)
    // So we'll put one in here.
    NSRect theRect;
	ESOpenGLView *glView;
	NSTimer *animationTimer;
	NSLock *animationLock;
	BOOL m_isStopped;
	
	BOOL m_isPreview;
	
	BOOL m_isFullScreen;
	
#ifdef SCREEN_SAVER
	BOOL m_isHidden;
#endif
	
	ESConfiguration* m_config;
	
	SUUpdater* m_updater;
}

- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview;
- (void)startAnimation;
- (void)stopAnimation;
- (void)animateOneFrame;

- (BOOL)hasConfigureSheet;
- (NSWindow*)configureSheet;
- (void)flagsChanged:(NSEvent *)ev;
- (void) keyDown:(NSEvent *) ev;

- (void)_beginThread;
- (void)_endThread;
- (void)_animationThread:(ESScreensaverView *)source;

- (void)windowDidResize;

- (void)updaterWillRelaunchApplication:(SUUpdater *)updater;
- (void)updater:(SUUpdater *)updater didFindValidUpdate:(SUAppcastItem *)update;

- (BOOL)fullscreen;
- (void)setFullScreen:(BOOL)fullscreen;

@end
