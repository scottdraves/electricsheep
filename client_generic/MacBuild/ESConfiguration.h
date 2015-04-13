#import <Cocoa/Cocoa.h>
#import "Sparkle/Sparkle.h"

@interface ESConfiguration : NSWindowController
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6 
 <NSTextFieldDelegate> 
#endif
{
    IBOutlet NSMatrix* displayMode;
	
    IBOutlet NSTextField* playerFPS;
    IBOutlet NSTextField* displayFPS;
    IBOutlet NSTextField* loopIterations;
	IBOutlet NSPopUpButton* display;
	IBOutlet NSPopUpButton* multiDisplayMode;
	IBOutlet NSButton* seamlessPlayback;
	IBOutlet NSButton* synchronizeVBL;
	IBOutlet NSButton* blackoutMonitors;
	IBOutlet NSSlider* playEvenly;
	IBOutlet NSButton* showAttribution;
	IBOutlet NSButton* autoUpdates;
	IBOutlet NSButton* negVoteKills;
	IBOutlet NSButton* calculateTransitions;
    IBOutlet NSButton* preserveAR;
			
    IBOutlet NSTextField* drupalLogin;
    IBOutlet NSSecureTextField* drupalPassword;
    IBOutlet NSButton* useProxy;
    IBOutlet NSTextField* proxyHost;
    IBOutlet NSTextField* proxyLogin;
    IBOutlet NSSecureTextField* proxyPassword;

	IBOutlet NSMatrix* cacheType;
	IBOutlet NSFormCell* cacheSize;
	IBOutlet NSTextField* contentFldr;	
	IBOutlet NSButton* enableDownload;
	IBOutlet NSButton* enableRendering;
	//IBOutlet NSButton* allCores;
	IBOutlet NSButton* saveFrames;
	IBOutlet NSButton* debugLog;
	IBOutlet NSButton* silentMode;
	
	IBOutlet NSPopUpButton* playbackMixingMode;
	IBOutlet NSMatrix* goldCacheType;
	IBOutlet NSFormCell* goldCacheSize;

	
	IBOutlet NSTextField* aboutText;
	
	IBOutlet NSTextField* membershipText;
		
	IBOutlet NSTextField* version;	
	
	IBOutlet NSTextField* flockSizeText;
	
	IBOutlet NSTextField* goldFlockSizeText;
	
	IBOutlet NSTextField* totalFlockSizeText;
	
	IBOutlet NSTextField* loginTestStatusText;
	
	IBOutlet NSImageView* loginStatusImage;
	
	IBOutlet NSButton* signInButton;
	
	NSString *m_origNickname;
	NSString *m_origPassword;
	
	NSString *m_roleString;
	
	NSMutableData *m_httpData;
	
	NSImage* redImage;
	NSImage* yellowImage;
	NSImage* greenImage;
	
	SUUpdater *m_updater;
	
	NSTimer *m_checkTimer;
		
	BOOL m_checkingLogin;
	
}


- (IBAction)ok:(id)sender;
- (IBAction)cancel:(id)sender;
- (IBAction)goToCreateAccountPage:(id)sender;
- (IBAction)goToLearnMorePage:(id)sender;
- (IBAction)goToHelpPage:(id)sender;
- (IBAction)chooseContentFolder:(id)sender;
- (IBAction)doManualUpdate:(id)sender;
- (IBAction)doSignIn:(id)sender;

- (ESConfiguration*)initWithWindowNibName:(NSString*)nibName updater:(SUUpdater*)updater;

- (void)htmlifyEditFields;
- (void)fixFlockSize;
- (NSString*)md5Password;
- (void)updateMembershipText:(NSString*)role;

- (void)awakeFromNib;
- (void)loadSettings;
- (void)saveSettings;
- (SUUpdater*) updater;

- (void)dealloc;

@end
