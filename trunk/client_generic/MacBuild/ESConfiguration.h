#import <Cocoa/Cocoa.h>
#import "Sparkle/Sparkle.h"

@interface ESConfiguration : NSWindowController {
    IBOutlet NSMatrix* displayMode;	
	
    IBOutlet NSFormCell* playerFPS;
    IBOutlet NSFormCell* displayFPS;
    IBOutlet NSFormCell* loopIterations;
	IBOutlet NSPopUpButton* display;
	IBOutlet NSPopUpButton* multiDisplayMode;
	IBOutlet NSButton* seamlessPlayback;
	IBOutlet NSButton* synchronizeVBL;
	IBOutlet NSSlider* playEvenly;
	IBOutlet NSButton* showAttribution;
	IBOutlet NSButton* autoUpdates;
			
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
	
	IBOutlet NSTextField* version;	
	
	NSString *origNickname;
	NSString *origPassword;
	
	SUUpdater *m_updater;
}


- (IBAction)ok:(id)sender;
- (IBAction)cancel:(id)sender;
- (IBAction)goToHelpPage:(id)sender;
- (IBAction)chooseContentFolder:(id)sender;
- (IBAction)doManualUpdate:(id)sender;

- (ESConfiguration*)initWithWindowNibName:(NSString*)nibName updater:(SUUpdater*)updater;

- (void)awakeFromNib;
- (void)loadSettings;
- (void)saveSettings;
- (SUUpdater*) updater;

@end
