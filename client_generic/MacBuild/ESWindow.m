#import "ESWindow.h"
#import "ESScreensaver.h" 
#import "Carbon/Carbon.h"

@implementation ESController

- (void)launchHelp: (id)sender
{
	[[NSWorkspace sharedWorkspace] openFile:[[NSBundle mainBundle] pathForResource:@"Instructions" ofType:@"rtf"]];
}

@end


@implementation ESWindow

- (void)awakeFromNib  // was - (NSWindow *)window
{
	[self setDelegate:self];
	
	[[NSApplication sharedApplication] setDelegate:self];
	
	NSRect frame = [self contentRectForFrameRect: [self frame]];
	
	mInSheet = NO;
	
	mFullScreenWindow = nil;
	
	mIsFullScreen = NO;
	
	mOriginalWindow = nil;
	
	mBlackingWindows = nil;
	
	ESScreensaver_InitClientStorage();
	
	SInt32 pmm = ESScreensaver_GetIntSetting("settings.player.PlaybackMixingMode", 0);
	
	const char *mpegpath = [[(NSString*)ESScreensaver_GetStringSetting("settings.content.sheepdir", "") autorelease] UTF8String];
	
	if (pmm == 1 || mpegpath == NULL || *mpegpath == 0 || ESScreensaver_GetFlockSizeMBs(mpegpath, 1) == 0) //playing only free sheep???
	{
		frame.size.width = 800;
		frame.size.height = 592;
	}
	else {
		frame.size.width = 1280;
		frame.size.height = 720; 
	}

	ESScreensaver_DeinitClientStorage();
	
	[self setFrame: [self frameRectForContentRect:frame] display:NO];

	[self makeKeyWindow];

	mESView = [[ESScreensaverView alloc] initWithFrame: frame isPreview:NO];
	
	if ( mESView != nil )
	{	
		[self setContentView:mESView];
		
		[mESView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
		
		[mESView setAutoresizesSubviews:YES];
		
		[mESView setFullScreen:NO];
			
		[self makeFirstResponder: mESView];
				
		[mESView startAnimation];
	}
}

- (void)toggleFullScreen:(id)sender
{
	NSScreen *targetScreen = [self screen];

	if(!mIsFullScreen)
	{
		// get screen that will be used for the full-screen display, create a borderless window the same size as the current content frame
		NSView *content = [self contentView];
		
		SetSystemUIMode( kUIModeAllHidden, 0);

		mFullScreenWindow = [[ESWindow alloc] 
								initWithContentRect:[self contentRectForFrameRect:[self frame]]
								styleMask:NSBorderlessWindowMask
								backing:NSBackingStoreBuffered
								defer:NO
								screen:targetScreen];
								
		[mFullScreenWindow setTitle:[self title]];
		
		//[mFullScreenWindow setHidesOnDeactivate:YES];
		
		[mFullScreenWindow setOriginalWindow:self];
								
		[mFullScreenWindow setBackgroundColor:[NSColor blackColor]];

		// switch the content view from targetWindow to the newly created window
		[content retain];

		[self setContentView:nil];

		[mFullScreenWindow setContentView:content];
		
		[content setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
		
		[mFullScreenWindow makeFirstResponder: content];

		[content release];

		// hide old window
		[self orderOut:nil];

		// now that the content has changed to the fullScreenWindow, draw a new frame (for OpenGL purposes) and make new window key and in front, with me as a delegate
		[mFullScreenWindow makeKeyAndOrderFront:nil];
				
		[mFullScreenWindow setLevel:kCGNormalWindowLevel];
				
		// get fullScreenWindow to resize to full screen
		[mFullScreenWindow setFrame:[targetScreen frame] display:YES animate:YES];
		
		if (CGCursorIsVisible())
		{
			[NSCursor hide];
		}
		
		[self blackScreensExcept:targetScreen];
		
		[[NSNotificationCenter defaultCenter] addObserver:self
										 selector:@selector(fullscreenWindowMoved:)
											 name:NSWindowDidMoveNotification
										   object:mFullScreenWindow];


		// set fullScreen flag
		mIsFullScreen = YES;
	}
	else
	{
		[[NSNotificationCenter defaultCenter] removeObserver:self
														name:NSWindowDidMoveNotification
													  object:mFullScreenWindow];

		/*// release the captured screen
		NSNumber *screenNumber = [[[mFullScreenWindow screen] deviceDescription] objectForKey:@"NSScreenNumber"];
		CGDisplayRelease((screenNumber != nil) ? (CGDirectDisplayID)[screenNumber unsignedLongValue] : kCGDirectMainDisplay);*/

		// restore normal window, resize the fullScreenWindow to the content frame for the desktop window
		[mFullScreenWindow setFrame:[self contentRectForFrameRect:[self frame]] display:YES animate:YES];

		// switch the content view from the fullScreenWindow back to the ordinary window
		NSView *content = [mFullScreenWindow contentView];
		[content retain];

		[mFullScreenWindow setContentView:nil];

		[self setContentView:content];
		
		[self makeFirstResponder: content];

		[content release];

		// redraw and make the ordinary window front and key
		[self makeKeyAndOrderFront:nil];

		// release the fullscreen window
		[mFullScreenWindow release]; 
		
		mFullScreenWindow = nil;
		
		if (!CGCursorIsVisible())
		{
			[NSCursor unhide];
		}

		SetSystemUIMode( kUIModeNormal, 0);
		
		[self unblackScreens];

		// set fullscreen flag
		mIsFullScreen = NO;
	}
	
	[mESView setFullScreen:mIsFullScreen];

	[mESView windowDidResize];
}

/*
	Black out all screens except fullscreen screen
 */
- (void) blackScreensExcept:(NSScreen*)fullscreen
{
	if (mBlackingWindows != NULL)
		[mBlackingWindows release];
		
	mBlackingWindows = [[NSMutableArray alloc] initWithCapacity:[[NSScreen screens] count]];
	
	unsigned int i;
	NSWindow *win;
	NSRect fs_rect;
	for (i = 0; i < [[NSScreen screens] count]; i++) { 
		
		NSScreen *actScreen = [[NSScreen screens] objectAtIndex:i];
		
		if (actScreen == nil || fullscreen == actScreen)
			continue;
		
		// when blacking the main screen, hide the menu bar and dock
		if (actScreen == [NSScreen mainScreen])
			SetSystemUIMode( kUIModeAllSuppressed, 0);
		
		fs_rect = [actScreen frame];
		fs_rect.origin = NSZeroPoint;
		win = [[NSWindow alloc] initWithContentRect:fs_rect styleMask:NSBorderlessWindowMask 
											backing:NSBackingStoreBuffered defer:NO screen:[[NSScreen screens] objectAtIndex:i]];
		[win setBackgroundColor:[NSColor blackColor]];
		[win setLevel:NSScreenSaverWindowLevel];
		[win orderFront:nil];
		
		//if ([[AppController sharedController] animateInterface])
		[self fadeWindow:win withEffect:NSViewAnimationFadeInEffect];
		
		[mBlackingWindows addObject:win];
		[win release];
	}
	
}

/*
	Remove black out windows
 */
- (void) unblackScreens
{
	if (!mBlackingWindows)
		return;
	
	unsigned int i;
	for (i = 0; i < [mBlackingWindows count]; i++) {
		/*if (![[AppController sharedController] animateInterface])
			[[mBlackingWindows objectAtIndex:i] close];
		else*/
			[self fadeWindow:[mBlackingWindows objectAtIndex:i] withEffect:NSViewAnimationFadeOutEffect];
	}
	
	[mBlackingWindows release];
	mBlackingWindows = nil;
}

/*
	Animate window fading in/out
*/
- (void) fadeWindow:(NSWindow *)window withEffect:(NSString *)effect
{
	
	NSViewAnimation *anim;
	NSMutableDictionary *animInfo;
	
	animInfo = [NSMutableDictionary dictionaryWithCapacity:2];
	[animInfo setObject:window forKey:NSViewAnimationTargetKey];
	[animInfo setObject:effect forKey:NSViewAnimationEffectKey];
	
	anim = [[NSViewAnimation alloc] initWithViewAnimations:[NSArray arrayWithObject:animInfo]];
	[anim setAnimationBlockingMode:NSAnimationNonblockingThreaded];
	[anim setAnimationCurve:NSAnimationEaseIn];
	[anim setDuration:0.3];
	
	[anim startAnimation];
	[anim release];
}

- (void) fullscreenWindowMoved:(NSNotification *)notification
{
	// triggered when fullscreen window changes spaces
	NSRect screen_frame = [[mFullScreenWindow screen] frame];
	[mFullScreenWindow setFrame:screen_frame display:YES animate:NO];
}



- (void)windowWillClose:(NSNotification *)notification
{
	if ( mESView != nil )
	{
		[mESView stopAnimation];
	}
}

- (void)showPreferences:(id)sender
{
	if ( !mIsFullScreen && mESView && [mESView hasConfigureSheet] )
	{
		[mESView stopAnimation];
		
		[NSApp beginSheet: [mESView configureSheet]
            modalForWindow: self
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
			
		mInSheet = YES;
	}
}

- (void)didEndSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    [sheet orderOut:self];
	
	mInSheet = NO;
	
	if ( mESView != nil )
	{
		[mESView startAnimation];
		[mESView windowDidResize];
	}
}


- (void)windowDidResize:(NSNotification *)notification
{
	if ( mInSheet )
		return;

	if ( mESView != nil )
		[mESView windowDidResize];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
	return YES;
}

- (BOOL)canBecomeKeyWindow
{
	return YES;
}

- (ESWindow *)originalWindow
{
	return mOriginalWindow;
}

- (void)setOriginalWindow: (ESWindow*)window
{
	mOriginalWindow = window;
}

- (BOOL)isFullScreen
{
	return mIsFullScreen;
}

- (void)keyDown:(NSEvent *)ev;
{
    BOOL handled = NO;
	
    NSString *characters = [ev charactersIgnoringModifiers];
    unsigned int characterIndex, characterCount = [characters length];
    
    for (characterIndex = 0; characterIndex < characterCount; characterIndex++) {
		unichar c = [characters characterAtIndex:characterIndex];
		switch (c) {
			case 0x1B: //ESC key
				{
					ESWindow *wnd = [self originalWindow];
					if ( wnd != nil && [wnd isFullScreen] )
					{
						[wnd toggleFullScreen:self];
					}
				}
				handled = YES;
				break;
								
			default:
				break;
		}
    }
	
    // If we didn't handle the key press, send it to the parent class
    if (handled == NO)
		[super keyDown:ev];
}

@end
