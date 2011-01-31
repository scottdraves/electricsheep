#import "ESConfiguration.h"
#import "ESScreensaver.h"
#import "clientversion.h"
#import "md5.h"

@implementation ESConfiguration

- (IBAction)ok:(id)sender
{
	ESScreensaver_InitClientStorage();

    [self saveSettings];
	
	ESScreensaver_DeinitClientStorage();
	
	[NSApp endSheet:[self window]];
};

- (IBAction)cancel:(id)sender
{
	[NSApp endSheet:[self window]];
};

- (void)awakeFromNib  // was - (NSWindow *)window
{
	ESScreensaver_InitClientStorage();

	[self loadSettings];

	ESScreensaver_DeinitClientStorage();
}

- (NSString*)computeMD5:(NSString*)str
{
	unsigned char digest[16]; //md5 digest size is 16
	
	const char *cstr = [str UTF8String];
	
	if ( cstr == nil )
		return nil;
		
	md5_buffer( cstr, strlen(cstr), digest );
	
	NSMutableString *retStr = [NSMutableString stringWithCapacity:sizeof(digest)*2];
	
	for (int i = 0; i < sizeof(digest); i++)
	{
		char *hex_digits = "0123456789ABCDEF";
		
		[retStr appendFormat:@"%c%c", hex_digits[ digest[i] >> 4 ], hex_digits[ digest[i] & 0x0F ]];
	}
		
	return retStr;
}

- (void)loadSettings
{
	[playerFPS setIntValue: ESScreensaver_GetIntSetting("settings.player.player_fps", 23)];
	
	[displayFPS setIntValue: ESScreensaver_GetIntSetting("settings.player.display_fps", 60)];
	
	[loopIterations setIntValue: ESScreensaver_GetIntSetting("settings.player.LoopIterations", 2)];
	
	SInt32 dm = ESScreensaver_GetIntSetting("settings.player.DisplayMode", 2);
	
	[displayMode selectCellWithTag:dm];
	
	SInt32 scr = ESScreensaver_GetIntSetting("settings.player.screen", 0);
	
	SInt32 scrcnt = [[NSScreen screens] count];
	
	if ( scr >= scrcnt && scrcnt > 0 )
		scr = scrcnt - 1;
	
	while ( [display numberOfItems] > scrcnt )
		[display removeItemAtIndex:scrcnt];

	[display selectItemAtIndex:scr];
	
	SInt32 mdmode = ESScreensaver_GetIntSetting("settings.player.MultiDisplayMode", 0);
	
	[multiDisplayMode selectItemAtIndex:mdmode];
	
	[seamlessPlayback setState: ESScreensaver_GetBoolSetting("settings.player.SeamlessPlayback", false)];
	
	[synchronizeVBL setState: ESScreensaver_GetBoolSetting("settings.player.vbl_sync", false)];

	[playEvenly setIntValue: ESScreensaver_GetIntSetting("settings.player.PlayEvenly", 100)];
	
	[silentMode setState: ESScreensaver_GetBoolSetting("settings.player.silent_mode", true)];

	[showAttribution setState: ESScreensaver_GetBoolSetting("settings.app.attributionpng", true)];
	
	SUUpdater *upd = [self updater];

	if (upd)
		[autoUpdates setState:[upd automaticallyChecksForUpdates]];

	[useProxy setState: ESScreensaver_GetBoolSetting("settings.content.use_proxy", false)];
		
	[proxyHost setStringValue: (NSString*)ESScreensaver_GetStringSetting("settings.content.proxy", "")];
	
	origNickname = (NSString*)ESScreensaver_GetStringSetting("settings.generator.nickname", "");
	
	[origNickname retain];
	
	[drupalLogin setStringValue: origNickname];
	
	origPassword = (NSString*)ESScreensaver_GetStringSetting("settings.content.password_md5", "");
	
	[origPassword retain];

	[drupalPassword setStringValue: origPassword];
	
	[proxyLogin setStringValue: (NSString*)ESScreensaver_GetStringSetting("settings.content.proxy_username", "")];

	[proxyPassword setStringValue: (NSString*)ESScreensaver_GetStringSetting("settings.content.proxy_password", "")];
	
	bool unlimited_cache = ESScreensaver_GetBoolSetting("settings.content.unlimited_cache", false);
	
	SInt32 cache_size = ESScreensaver_GetIntSetting("settings.content.cache_size", 2000);

	if (cache_size == 0)
	{
		unlimited_cache = true;
		
		cache_size = 2000;
	}

	[cacheType selectCellWithTag: (unlimited_cache ? 0 : 1) ];
	
	[cacheSize setIntValue: cache_size];
	
	[enableDownload setState: ESScreensaver_GetBoolSetting("settings.content.download_mode", true)];

	[enableRendering setState: ESScreensaver_GetBoolSetting("settings.generator.enabled", true)];
	
	//[allCores setState: ESScreensaver_GetBoolSetting("settings.generator.all_cores", false)];

	[saveFrames setState: ESScreensaver_GetBoolSetting("settings.generator.save_frames", false)];

	[debugLog setState: ESScreensaver_GetBoolSetting("settings.app.log", false)];

	[contentFldr setStringValue: [(NSString*)ESScreensaver_GetStringSetting("settings.content.sheepdir", "") stringByAbbreviatingWithTildeInPath]];

	[version setStringValue:(NSString*)ESScreensaver_GetVersion()];

}

- (void)saveSettings
{
	ESScreensaver_SetIntSetting("settings.player.player_fps", [playerFPS intValue]);
	
	ESScreensaver_SetIntSetting("settings.player.display_fps", [displayFPS intValue]);

	ESScreensaver_SetIntSetting("settings.player.LoopIterations", [loopIterations intValue]);

	ESScreensaver_SetIntSetting("settings.player.DisplayMode", [[displayMode selectedCell] tag]);
	
	ESScreensaver_SetBoolSetting("settings.player.SeamlessPlayback", [seamlessPlayback state]);
	
	ESScreensaver_SetBoolSetting("settings.player.vbl_sync", [synchronizeVBL state]);

	ESScreensaver_SetIntSetting("settings.player.PlayEvenly", [playEvenly intValue]);

	ESScreensaver_SetIntSetting("settings.player.screen", [display indexOfSelectedItem]);
	
	ESScreensaver_SetIntSetting("settings.player.MultiDisplayMode", [multiDisplayMode indexOfSelectedItem]);

	ESScreensaver_SetBoolSetting("settings.player.silent_mode", [silentMode state]);
	
	ESScreensaver_SetBoolSetting("settings.app.attributionpng", [showAttribution state]);

	SUUpdater *upd = [self updater];

	if (upd)
		[upd setAutomaticallyChecksForUpdates:[autoUpdates state]];

	ESScreensaver_SetBoolSetting("settings.content.use_proxy", [useProxy state]);

	ESScreensaver_SetStringSetting("settings.content.proxy", [[proxyHost stringValue] UTF8String]);

	ESScreensaver_SetStringSetting("settings.content.sheepdir", [[[contentFldr stringValue] stringByStandardizingPath] UTF8String]);
	
	NSString *newNickname = [drupalLogin stringValue];
	
	NSString *newPassword = [drupalPassword stringValue];
	
	if (![newNickname isEqual:origNickname] || ![newPassword isEqual:origPassword])
	{
		ESScreensaver_SetStringSetting("settings.generator.nickname", [newNickname UTF8String]);

		NSString *md5Password = [self computeMD5:[NSString stringWithFormat:@"%@sh33p%@", newPassword, newNickname]];
		
		ESScreensaver_SetStringSetting("settings.content.password_md5", [md5Password UTF8String]);
	}

	ESScreensaver_SetStringSetting("settings.content.proxy_username", [[proxyLogin stringValue] UTF8String]);

	ESScreensaver_SetStringSetting("settings.content.proxy_password", [[proxyPassword stringValue] UTF8String]);
	
	bool unlimited_cache = ([[cacheType selectedCell] tag] == 0);
	
	ESScreensaver_SetBoolSetting("settings.content.unlimited_cache", unlimited_cache);

	SInt32 cache_size = [cacheSize intValue];
	
	ESScreensaver_SetIntSetting("settings.content.cache_size", cache_size);

	ESScreensaver_SetBoolSetting("settings.content.download_mode", [enableDownload state]);
	
	ESScreensaver_SetBoolSetting("settings.generator.enabled", [enableRendering state]);

	//ESScreensaver_SetBoolSetting("settings.generator.all_cores", [allCores state]);

	ESScreensaver_SetBoolSetting("settings.generator.save_frames", [saveFrames state]);

	ESScreensaver_SetBoolSetting("settings.app.log", [debugLog state]);
}


- (IBAction)goToHelpPage:(id)sender
{
	NSString *urlStr = [NSString stringWithFormat:@"http://electricsheep.org/client/%s", CLIENT_VERSION];
	
	NSURL *helpURL = [NSURL URLWithString:urlStr];
	
	[[NSWorkspace sharedWorkspace] openURL:helpURL];
}

- (IBAction)chooseContentFolder:(id)sender
{
    NSTextField *field = nil;
    
	field = contentFldr;

    if (field) {
        NSOpenPanel *openPanel = [NSOpenPanel openPanel];
        int result = NSOKButton;
        NSString *path = [[field stringValue] stringByExpandingTildeInPath];
					
        [openPanel setCanChooseFiles:NO];
        [openPanel setCanChooseDirectories:YES];
        [openPanel setCanCreateDirectories:YES];
        [openPanel setAllowsMultipleSelection:NO];
        [openPanel setDirectory:[path stringByStandardizingPath]];
        
        result = [openPanel runModalForDirectory:nil file:nil types:nil];
        if (result == NSOKButton) {
            [field setObjectValue:[[openPanel directory] stringByAbbreviatingWithTildeInPath]];
        }
    }
}

- (ESConfiguration*)initWithWindowNibName:(NSString*)nibName updater:(SUUpdater*)updater
{
	m_updater = updater;
	
	return [super initWithWindowNibName:nibName];
}

- (SUUpdater*) updater
{
	return m_updater;
}

- (IBAction)doManualUpdate:(id)sender
{
	//[NSApp endSheet:[self window]];
	
	SUUpdater *upd = [self updater];
				
	if (upd != NULL)
		[upd checkForUpdates:sender];
}

@end
