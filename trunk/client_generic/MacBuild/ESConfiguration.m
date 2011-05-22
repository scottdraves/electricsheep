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
}

- (IBAction)cancel:(id)sender
{
	[NSApp endSheet:[self window]];
}

- (void)awakeFromNib  // was - (NSWindow *)window
{
	CFBundleRef bndl = dlbundle_ex();
	
	NSURL *imgUrl = (NSURL*)CFBundleCopyResourceURL(bndl, CFSTR("red.tif"), NULL, NULL);
	
	redImage = [[NSImage alloc] initWithContentsOfURL:imgUrl]; 
	
	[imgUrl release];
	
	imgUrl = (NSURL*)CFBundleCopyResourceURL(bndl, CFSTR("yellow.tif"), NULL, NULL);

	yellowImage = [[NSImage alloc] initWithContentsOfURL:imgUrl];
	
	[imgUrl release];
	
	imgUrl = (NSURL*)CFBundleCopyResourceURL(bndl, CFSTR("green.tif"), NULL, NULL);
	
	greenImage = [[NSImage alloc] initWithContentsOfURL:imgUrl];
	
	[imgUrl release];
	
	m_checkTimer = nil;
	
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


- (void)htmlifyEditFields
{
	// both are needed, otherwise hyperlink won't accept mousedown
    [aboutText setAllowsEditingTextAttributes: YES];
    [aboutText setSelectable: YES];

    NSAttributedString* string = [[NSMutableAttributedString alloc] initWithHTML:[[aboutText stringValue] dataUsingEncoding:NSUTF8StringEncoding] documentAttributes:NULL];

    // set the attributed string to the NSTextField
    [aboutText setAttributedStringValue: string];
	
	[string release];
}

- (void)fixFlockSize
{	 
	const char *mpegpath = [[[contentFldr stringValue] stringByStandardizingPath] UTF8String];
	 
	if (mpegpath != NULL && *mpegpath)
	{

		size_t flockSize = ESScreensaver_GetFlockSizeMBs(mpegpath, 0);
		
		size_t goldFlockSize = ESScreensaver_GetFlockSizeMBs(mpegpath, 1);
		
		size_t totalSize = flockSize + goldFlockSize;

		
		NSMutableString *str = [NSMutableString stringWithString:[flockSizeText stringValue]];

		[str replaceOccurrencesOfString:@"^1" withString:[NSString stringWithFormat:@"%ld", flockSize] options:0 range:NSMakeRange(0, [str length])];
		
		[flockSizeText setStringValue:str];
		
		
		str = [NSMutableString stringWithString:[goldFlockSizeText stringValue]];

		[str replaceOccurrencesOfString:@"^1" withString:[NSString stringWithFormat:@"%ld", goldFlockSize] options:0 range:NSMakeRange(0, [str length])];
		
		[goldFlockSizeText setStringValue:str]; 
		
		
		str = [NSMutableString stringWithString:[totalFlockSizeText stringValue]];

		[str replaceOccurrencesOfString:@"^1" withString:[NSString stringWithFormat:@"%ld", totalSize] options:0 range:NSMakeRange(0, [str length])];
		
		[totalFlockSizeText setStringValue:str];  
	}
}

- (void)setCheckingLogin:(Boolean)cl
{
	
}


- (void)connection:(NSURLConnection *)connection
  didFailWithError:(NSError *)error
{ 
	[loginStatusImage setImage:yellowImage];
	[loginTestStatusText setStringValue:@"The server is unreachable."];
	
	m_checkingLogin = NO;
	
	[signInButton setEnabled:YES];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	// do something with the data
	// receivedData is declared as a method instance elsewhere
	// NSLog(@"Succeeded! Received bytes of data");
	
	int len = [m_httpData length];
	
	if (len > 0)
	{
		char xml[1024];
	   
		[m_httpData getBytes:xml length:sizeof(xml) - 1];
	   
		if ([m_httpData length] < sizeof(xml))
			xml[ [m_httpData length] ] = 0;
		else
			xml[sizeof(xml) - 1] = 0;
			
		NSString *rolestr = (NSString*)ESScreensaver_GetRoleFromXML(xml);
		   
		if (![rolestr isEqual:@"error"] && ![rolestr isEqual:@"none"])
		{
			[loginStatusImage setImage:greenImage];
			[loginTestStatusText setStringValue:[NSString stringWithFormat:@"Logged in (role: %@).", rolestr ? rolestr : @"N/A"]];
		}
		else
		{
			[loginStatusImage setImage:redImage];
			[loginTestStatusText setStringValue:[NSString stringWithFormat:@"Login Failed."]];
		}

		[self updateMembershipText:rolestr];
	}
	else
	{
		[loginStatusImage setImage:yellowImage];
		[loginTestStatusText setStringValue:@"Incorrect response from the server."];
	}
	
	[m_httpData release];
	
	m_checkingLogin = NO;
	
	[signInButton setEnabled:YES];
}

- (void)updateMembershipText:(NSString*)role
{
	if ([role isEqual:@"error"] || [role isEqual:@"none"])
	{
		[membershipText setStringValue:@"Become a member for access to our private server with more sheep,\nhigher resolution sheep, and other interactive features.\n"];
	} else
	if ([role isEqual:@"registered"])
	{
		[membershipText setStringValue:@"Thank you for registering, you may become a member for access to\nour private server with more sheep, higher resolution sheep,\nand other interactive features."];
	} else
	if ([role isEqual:@"member"])
	{
		[membershipText setStringValue:@"Thank you for your membership, you may upgrade to Gold for higher\nresolution and other benefits."];
	} else
	if ([role isEqual:@"gold"])
	{
		[membershipText setStringValue:@"Thank you for registering, you may become a member for access to\nour private server with more sheep, higher resolution sheep,\nand other interactive features."];
	}						
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    // This method is called when the server has determined that it
    // has enough information to create the NSURLResponse.
 
    // It can be called multiple times, for example in the case of a
    // redirect, so each time we reset the data.
 
    // receivedData is an instance variable declared elsewhere.
    //[receivedData setLength:0];
	NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse*)response;
    
    if (![httpResponse isKindOfClass:[NSHTTPURLResponse class]]) {
        NSLog(@"Unknown response type: %@", response);
        return;
    }
		
	int _statusCode = [httpResponse statusCode];	

	
	if (_statusCode == 200)
	{
	}
	else 
	{
		[loginStatusImage setImage:redImage];
		[loginTestStatusText setStringValue:@"Login Failed :("];
	}

}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
    // Append the new data to receivedData.
    // receivedData is an instance variable declared elsewhere.
    [m_httpData appendData:data];
}


- (void)startTest:(NSTimer *)timer
{
	if (m_checkingLogin)
	{
		if (timer)
			[m_checkTimer setFireDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];
		return;
	}
	
	[signInButton setEnabled:NO];
		
	m_checkingLogin = YES;

	[m_checkTimer invalidate];
	m_checkTimer = nil;
	

	[loginStatusImage setImage:nil];

	[loginTestStatusText setStringValue:@"Testing Login..."];
	
	m_httpData = [[NSMutableData dataWithCapacity:10] retain];
			
	NSString *newNickname = [drupalLogin stringValue];
	
	NSString *md5_pass = [self md5Password];

	CFStringRef urlnickname = CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, (CFStringRef)newNickname, NULL, NULL, kCFStringEncodingUTF8);	
	CFStringRef urlpass = CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, (CFStringRef)md5_pass, NULL, NULL, kCFStringEncodingUTF8);
	CFStringRef urlver = CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, CFSTR(CLIENT_VERSION), NULL, NULL, kCFStringEncodingUTF8);
	
	NSString *urlstr = [NSString stringWithFormat:@"http://%s/query.php?q=redir&u=%@&p=%@&v=%@", REDIRECT_SERVER, urlnickname, urlpass, urlver ];
		
	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:urlstr]];
	
	CFRelease(urlnickname);
	CFRelease(urlpass);
	CFRelease(urlver);
	
	/*CFHTTPMessageRef dummyRequest =
		CFHTTPMessageCreateRequest(
			kCFAllocatorDefault,
			CFSTR("GET"),
			(CFURLRef)[request URL],
			kCFHTTPVersion1_1);
			
	CFHTTPMessageAddAuthentication(
		dummyRequest,
		nil,
		(CFStringRef)[drupalLogin stringValue],
		(CFStringRef)[drupalPassword stringValue],
		kCFHTTPAuthenticationSchemeBasic,
		FALSE);
		
	NSString *authorizationString =
		(NSString *)CFHTTPMessageCopyHeaderFieldValue(
			dummyRequest,
			CFSTR("Authorization"));
						
	CFRelease(dummyRequest);
	
	[request setValue:authorizationString forHTTPHeaderField:@"Authorization"];*/
	
	[NSURLConnection connectionWithRequest:request delegate:self];
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
	if (m_checkTimer != nil)
	{
		if ([m_checkTimer isValid])
		{
			[m_checkTimer setFireDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];
			return;
		}
		else
			[m_checkTimer release];
	}
			
	m_checkTimer = [NSTimer scheduledTimerWithTimeInterval:2.0 target:self selector:@selector(startTest:) userInfo:nil repeats:NO];
}


- (void)loadSettings
{
	[self htmlifyEditFields];
	
	[playerFPS setDoubleValue: ESScreensaver_GetDoubleSetting("settings.player.player_fps", 23.0)];
	
	[displayFPS setDoubleValue: ESScreensaver_GetDoubleSetting("settings.player.display_fps", 60.0)];
	
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
	
	[negVoteKills setState: ESScreensaver_GetBoolSetting("settings.content.negvotedeletes", true)];
	
	SUUpdater *upd = [self updater];

	if (upd)
		[autoUpdates setState:[upd automaticallyChecksForUpdates]];

	[useProxy setState: ESScreensaver_GetBoolSetting("settings.content.use_proxy", false)];
			
	[proxyHost setStringValue: [(NSString*)ESScreensaver_GetStringSetting("settings.content.proxy", "") autorelease]];
		
	m_origNickname = (NSString*)ESScreensaver_GetStringSetting("settings.generator.nickname", "");
	
	//[m_origNickname retain];
	
	[drupalLogin setStringValue: m_origNickname];
	
	[drupalLogin setDelegate:self];
	
	m_origPassword = (NSString*)ESScreensaver_GetStringSetting("settings.content.password_md5", "");
	
	//[m_origPassword retain];

	[drupalPassword setStringValue: m_origPassword];
	
	[drupalPassword setDelegate:self];
	
	[proxyLogin setStringValue: [(NSString*)ESScreensaver_GetStringSetting("settings.content.proxy_username", "") autorelease]];

	[proxyPassword setStringValue: [(NSString*)ESScreensaver_GetStringSetting("settings.content.proxy_password", "") autorelease]];
	
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

	[contentFldr setStringValue: [[(NSString*)ESScreensaver_GetStringSetting("settings.content.sheepdir", "") autorelease] stringByAbbreviatingWithTildeInPath]];
	
	
	SInt32 pmm = ESScreensaver_GetIntSetting("settings.player.PlaybackMixingMode", 0);
	
	[playbackMixingMode selectItemAtIndex:pmm];
	
	bool unlimited_cache_gold = ESScreensaver_GetBoolSetting("settings.content.unlimited_cache_gold", false);
	
	SInt32 cache_size_gold = ESScreensaver_GetIntSetting("settings.content.cache_size_gold", 2000);

	if (cache_size_gold == 0)
	{
		unlimited_cache_gold = true;
		
		cache_size_gold = 2000;
	}

	[goldCacheType selectCellWithTag: (unlimited_cache_gold ? 0 : 1) ];
	
	[goldCacheSize setIntValue: cache_size_gold];
	

	[version setStringValue:(NSString*)ESScreensaver_GetVersion()];
		
	[self fixFlockSize];
	
	[self startTest:nil];

}

- (void)saveSettings
{
	double player_fps = [playerFPS doubleValue];
	
	if (player_fps <= .1)
		player_fps = .1;
	
	ESScreensaver_SetDoubleSetting("settings.player.player_fps", player_fps);
	
	double display_fps = [displayFPS doubleValue];
	
	if (display_fps <= .1)
		display_fps = .1;
	
	ESScreensaver_SetDoubleSetting("settings.player.display_fps", display_fps);

	ESScreensaver_SetIntSetting("settings.player.LoopIterations", [loopIterations intValue]);

	ESScreensaver_SetIntSetting("settings.player.DisplayMode", [[displayMode selectedCell] tag]);
	
	ESScreensaver_SetBoolSetting("settings.player.SeamlessPlayback", [seamlessPlayback state]);
	
	ESScreensaver_SetBoolSetting("settings.player.vbl_sync", [synchronizeVBL state]);

	ESScreensaver_SetIntSetting("settings.player.PlayEvenly", [playEvenly intValue]);

	ESScreensaver_SetIntSetting("settings.player.screen", [display indexOfSelectedItem]);
	
	ESScreensaver_SetIntSetting("settings.player.MultiDisplayMode", [multiDisplayMode indexOfSelectedItem]);

	ESScreensaver_SetBoolSetting("settings.player.silent_mode", [silentMode state]);
	
	ESScreensaver_SetBoolSetting("settings.app.attributionpng", [showAttribution state]);
	
	ESScreensaver_SetBoolSetting("settings.content.negvotedeletes", [negVoteKills state]);

	SUUpdater *upd = [self updater];

	if (upd)
		[upd setAutomaticallyChecksForUpdates:[autoUpdates state]];

	ESScreensaver_SetBoolSetting("settings.content.use_proxy", [useProxy state]);

	ESScreensaver_SetStringSetting("settings.content.proxy", [[proxyHost stringValue] UTF8String]);

	ESScreensaver_SetStringSetting("settings.content.sheepdir", [[[contentFldr stringValue] stringByStandardizingPath] UTF8String]);
	
	NSString *newNickname = [drupalLogin stringValue];
	
	NSString *newPassword = [drupalPassword stringValue];
	
	if (![newNickname isEqual:m_origNickname] || ![newPassword isEqual:m_origPassword])
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
	
	
	ESScreensaver_SetIntSetting("settings.player.PlaybackMixingMode", [playbackMixingMode indexOfSelectedItem]);
		
	bool unlimited_cache_gold = ([[goldCacheType selectedCell] tag] == 0);
	
	ESScreensaver_SetBoolSetting("settings.content.unlimited_cache_gold", unlimited_cache_gold);

	SInt32 cache_size_gold = [goldCacheSize intValue];
	
	ESScreensaver_SetIntSetting("settings.content.cache_size_gold", cache_size_gold);
}


- (IBAction)goToCreateAccountPage:(id)sender
{
	NSURL *helpURL = [NSURL URLWithString: @"http://community.electricsheep.org/user/register"];
	
	[[NSWorkspace sharedWorkspace] openURL:helpURL];
}

- (NSString*)md5Password
{
	NSString *newNickname = [drupalLogin stringValue];
	NSString *newPassword = [drupalPassword stringValue];

	if (![newPassword isEqual:m_origPassword])
	{
		return [self computeMD5:[NSString stringWithFormat:@"%@sh33p%@", newPassword, newNickname]];
	}
	else {
		return newPassword;
	}
}

- (IBAction)goToLearnMorePage:(id)sender
{
	NSString *newNickname = [drupalLogin stringValue];
	
	NSString *md5_pass = [self md5Password];

	CFStringRef urlnickname = CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, (CFStringRef)newNickname, NULL, NULL, kCFStringEncodingUTF8);	
	CFStringRef urlpass = CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, (CFStringRef)md5_pass, NULL, NULL, kCFStringEncodingUTF8);
	CFStringRef urlver = CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, CFSTR(CLIENT_VERSION), NULL, NULL, kCFStringEncodingUTF8);
	
	NSString *urlstr = [NSString stringWithFormat:@"http://electricsheep.org/account/?u=%@&p=%@", urlnickname, urlpass, urlver ];

	NSURL *helpURL = [NSURL URLWithString: urlstr];
	
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
	
	[self fixFlockSize];
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

- (IBAction)doSignIn:(id)sender
{
	[self startTest:nil];
}

@end
