#import "ESScreensaverView.h"
#import "ESScreensaver.h"
#import <OpenGL/OpenGL.h>
#include "dlfcn.h"
#include "libgen.h"

NSOpenGLContext *glContext = NULL;

bool bStarted = false;

@implementation ESScreensaverView

- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview
{	
#ifdef SCREEN_SAVER
	self = [super initWithFrame:frame isPreview:isPreview];
#else
	self = [super initWithFrame:frame];
#endif

	m_updater = NULL;
	
	m_isFullScreen = !isPreview;
	
	m_isPreview = isPreview;

#ifdef SCREEN_SAVER
	//if (isPreview)
#endif
	{
		CFBundleRef bndl = dlbundle_ex();
		NSBundle *nsbndl;
		
		if (bndl != NULL)
		{
			NSURL* url = (NSURL*)CFBundleCopyBundleURL(bndl);
			
			nsbndl = [NSBundle bundleWithPath:[url path]];

			m_updater = [SUUpdater updaterForBundle:nsbndl];
			
			[m_updater setDelegate:self];
			
			if (m_updater && [m_updater automaticallyChecksForUpdates])
			{
				[m_updater checkForUpdateInformation];
			}
		}
	}
	
	if (self)
    {
        // So we modify the setup routines just a little bit to get our
        // new OpenGL screensaver working.
		
		// Create the new frame
        NSRect newFrame = frame;
        // Slam the origin values
        newFrame.origin.x = 0.0;
        newFrame.origin.y = 0.0;
		
		theRect = newFrame;
        // Now alloc and init the view, right from within the screen saver's initWithFrame:

        // If the view is valid, we continue
        //if(glView)
        {
            // Make sure we autoresize
            [self setAutoresizesSubviews:YES];
            // So if our view is valid...
			
			glView = NULL;
            
            // Do some setup on our context and view
            //[glView prepareOpenGL];
            // Then we set our animation loop timer
            //[self setAnimationTimeInterval:1/60.0];
#ifdef SCREEN_SAVER
			[self setAnimationTimeInterval:-1];
#endif
            // Since our BasicOpenGLView class does it's setup in awakeFromNib, we call that here.
            // Note that this could be any method you want to use as the setup routine for your view.
            //[glView awakeFromNib];
        }
        //else // Log an error if we fail here
           // NSLog(@"Error: Electric Sheep Screen Saver failed to initialize NSOpenGLView!");
    }
    // Finally return our newly-initialized self
    return self;
}

- (void)startAnimation
{	
	/*NSMutableArray *displays = [NSMutableArray arrayWithCapacity:5];
	
	[displays addObject:[NSScreen mainScreen]];
	
	UInt32 cnt = [[NSScreen screens] count];
	
	for (int i=0; i<cnt; i++)
	{
		NSScreen *scr = [[NSScreen screens] objectAtIndex:i];
		
		 if (scr !=  [NSScreen mainScreen])
			[displays addObject:scr];
	}
	
	ESScreensaver_InitClientStorage();
	
	SInt32 scr = ESScreensaver_GetIntSetting("settings.player.screen", 0);
	
	ESScreensaver_DeinitClientStorage();
	
	if (scr >= cnt)
		scr = cnt - 1;
			
	//main screen only for now?
	if (!m_isPreview && [[self window] screen] != [displays objectAtIndex:scr])
	{
       return;
	}
	else
	{*/
        if (glView == NULL)
		{
			/*NSRect newRect;
			
			newRect.size.height = theRect.size.height;
			
			newRect.size.width = 800.0 * ( theRect.size.height / 592.0 );
			
			newRect.origin.x = theRect.origin.x + ( theRect.size.width - newRect.size.width ) / 2.0;
			
			newRect.origin.y = theRect.origin.y;
			
			theRect = newRect;*/
			
			glView = [[ESOpenGLView alloc] initWithFrame:theRect]; 
			
			if(glView)
			{
				glContext = [glView openGLContext];

				// We make it a subview of the screensaver view
				[self addSubview:glView];
				
				[glView release];
			}
		}
	//}

	if (glView != NULL && [glView openGLContext])
	{
		ESScreenSaver_AddGLContext( (CGLContextObj)[[glView openGLContext] CGLContextObj] );
	}
	
	int32 width = theRect.size.width;
	int32 height = theRect.size.height;
	
#ifdef SCREEN_SAVER
	m_isHidden = NO;
#endif
	
	if ( !bStarted )
	{

		if (!ESScreensaver_Start( m_isPreview, width, height ))
			return;
			
		bStarted = true;

		[self _beginThread];
	}
#ifdef SCREEN_SAVER
	[super startAnimation];
#endif
}

- (void)stopAnimation
{	
#ifdef SCREEN_SAVER
	[NSCursor unhide];
#endif
	if ( bStarted )
	{
		[self _endThread];
	
		ESScreensaver_Stop();
	
		ESScreensaver_Deinit();
		
		bStarted = false;
	}

#ifdef SCREEN_SAVER
	if (m_isHidden)
		[NSCursor unhide];
	m_isHidden = NO;
	[super stopAnimation];	
#endif
}

- (void)animateOneFrame
{	
	//[self setAnimationTimeInterval:-1];
	
	//[animationLock unlock];
	
	//ESScreensaver_DoFrame();
	
	//[glView setNeedsDisplay:YES];
}

- (void)_animationThread:(ESScreensaverView *)source 
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (animationLock != NULL)
		[animationLock lock];
		
	while (!m_isStopped && !ESScreensaver_Stopped() && ESScreensaver_DoFrame())
	{
#ifdef SCREEN_SAVER
		if (!m_isPreview && CGCursorIsVisible())
		{
			[NSCursor hide];
			m_isHidden = YES;
		}
#endif		
		//if (m_isStopped)
			//break;
		
		//if (glView != NULL)
			//[glView setNeedsDisplay:YES];
	}
	
	if (animationLock != NULL)
		[animationLock unlock];
	
	[pool release];
}

- (void)_beginThread
{
	animationLock = [[NSLock alloc] init];
	
	//[animationLock lock];
		
	m_isStopped = NO;
	
	[NSThread detachNewThreadSelector:@selector(_animationThread:) toTarget:self withObject:self];
}

- (void)_endThread
{
	m_isStopped = YES;
	
	[animationLock lock];
	[animationLock unlock];
	[animationLock release];
	
	animationLock = NULL;
}

- (void)windowDidResize
{
	[glView setFrame: [self frame]];
	
	ESScreensaver_ForceWidthAndHeight( [self frame].size.width, [self frame].size.height );
}


- (BOOL)hasConfigureSheet
{
    return YES;
}

- (NSWindow*)configureSheet
{
    if (!m_config) {
        m_config = [[ESConfiguration alloc] initWithWindowNibName:@"ElectricSheep" updater:m_updater];
    }
    
    return [m_config window];
}

- (void)flagsChanged:(NSEvent *)ev
{
	if ([ev keyCode] == 63) //FN Key
		return;
	
	[super flagsChanged:ev];
}

// keyDown
// Capture Up/Down for rating animations
// If there is no animation, or the computer cannot access the electricsheep server, UP and DOWN
// act just like in a normal screensaver (they stop it) - initially I thought it should just ignore
// the vote and not end playback (for consistency - UP/DOWN would never stop it) but I think it is
// appropriate that if you can't vote, the default event behavior used

//keycodes based on - http://www.filewatcher.com/p/BasiliskII-0.9.1.tgz.276457/share/BasiliskII/keycodes.html
- (void) keyDown:(NSEvent *) ev
{
    BOOL handled = NO;
	
    NSString *characters = [ev charactersIgnoringModifiers];
    unsigned int characterIndex, characterCount = [characters length];
    
    for (characterIndex = 0; characterIndex < characterCount; characterIndex++) {
		unichar c = [characters characterAtIndex:characterIndex];
		switch (c) {
			case NSRightArrowFunctionKey:
				ESScreensaver_AppendKeyEvent( 0x7C );
				handled = YES;
				break;
				
			case NSLeftArrowFunctionKey:
				ESScreensaver_AppendKeyEvent( 0x7B );
				handled = YES;
				break;
				
			case NSUpArrowFunctionKey:
				ESScreensaver_AppendKeyEvent( 0x7E );
				handled = YES;
				break;

			case NSDownArrowFunctionKey:
				ESScreensaver_AppendKeyEvent( 0x7D );
				handled = YES;
				break;
								
			case NSF1FunctionKey:
				ESScreensaver_AppendKeyEvent( 0x7A );
				handled = YES;
				break;
				
			case NSF2FunctionKey:
				ESScreensaver_AppendKeyEvent( 0x78 );
				handled = YES;
				break;

			case NSF3FunctionKey:
				ESScreensaver_AppendKeyEvent( 0x63 );
				handled = YES;
				break;

			case NSF4FunctionKey:
				ESScreensaver_AppendKeyEvent( 0x76 );
				handled = YES;
				break;

			case NSF8FunctionKey:
				ESScreensaver_AppendKeyEvent( 0x64 );
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

// Called immediately before relaunching.
- (void)updaterWillRelaunchApplication:(SUUpdater *)updater
{
	if (m_config != NULL)
		[NSApp endSheet:[m_config window]];
}

- (void)doUpdate:(NSTimer*)timer
{
	SUAppcastItem* update = [timer userInfo];
	
	if (!m_isFullScreen)
		[m_updater checkForUpdatesInBackground];
	else 
		ESScreensaver_SetUpdateAvailable([[update displayVersionString] UTF8String]);
}

// Sent when a valid update is found by the update driver.
- (void)updater:(SUUpdater *)updater didFindValidUpdate:(SUAppcastItem *)update
{
	[NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(doUpdate:) userInfo:update repeats:NO];
}

- (BOOL)fullscreen
{
	return m_isFullScreen;
}

- (void)setFullScreen:(BOOL)fullscreen
{
	m_isFullScreen = fullscreen;
}


@end
