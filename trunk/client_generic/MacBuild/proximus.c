/*
 *  proximus.c
 *  ElectricSheep
 */

#include <CoreServices/CoreServices.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <CoreFoundation/CoreFoundation.h>

#define kPrivateRunLoopMode CFSTR("org.electricsheep.proxy.autoconfigloop")

static void CFQRelease(CFTypeRef cf)
	// Trivial wrapper for CFRelease that treats NULL as a no-op.
{
	if (cf != NULL) {
		CFRelease(cf);
	}
}

static void ResultCallback(void * client, CFArrayRef proxies, CFErrorRef error)
	// Callback for CFNetworkExecuteProxyAutoConfigurationURL.  client is a 
	// pointer to a CFTypeRef.  This stashes either error or proxies in that 
	// location.
{
	CFTypeRef *		resultPtr;
	
	
	resultPtr = (CFTypeRef *) client;
	
	if (error != NULL) {
		*resultPtr = CFRetain(error);
	} else {
		*resultPtr = CFRetain(proxies);
	}
	CFRunLoopStop(CFRunLoopGetCurrent());
}

Boolean get_proxy_for_server105( const UInt8 *server, UInt8 *host, const UInt32 host_len, UInt8* user, const UInt32 user_len, UInt8* pass, const UInt32 pass_len )
{
	OSStatus		err;
	CFURLRef		url;
	CFDictionaryRef proxySettings;
	CFArrayRef		proxies;
	Boolean			enabled = false;
	CFTypeRef		res;

	
	url = NULL;
	proxySettings = NULL;
	proxies = NULL;
	res = NULL;
	
	CFStringRef urlStr = CFStringCreateWithFormat( NULL, NULL, CFSTR("http://%s"), server);
	
	// Create a URL from the argument C string.
	
	err = noErr;
	url = CFURLCreateWithString( NULL, urlStr, NULL );
	if (url == NULL) {
		err = coreFoundationUnknownErr;
	}
	
	CFRelease(urlStr);
	
	// Get the default proxies dictionary from CF.
	
	if (err == noErr) {
		proxySettings = SCDynamicStoreCopyProxies(NULL);
		if (proxySettings == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Call CFNetworkCopyProxiesForURL and print the results.
	
	if (err == noErr) {
		proxies = CFNetworkCopyProxiesForURL(url, proxySettings);
		if (proxies == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr && proxies != NULL && CFArrayGetCount(proxies) > 0) {
		CFDictionaryRef bestProxy = CFArrayGetValueAtIndex(proxies, 0);
		
		if (bestProxy != NULL)
		{
			CFStringRef proxyType = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyTypeKey);
			
			if ( proxyType != NULL && !CFEqual(proxyType, kCFProxyTypeNone) )
			{
				if (CFEqual(proxyType, kCFProxyTypeAutoConfigurationURL))
				{
					CFURLRef scriptURL = (CFURLRef) CFDictionaryGetValue(bestProxy, kCFProxyAutoConfigurationURLKey);
					
					if ( scriptURL != NULL )
					{
						CFStreamClientContext	context = { 0, &res, NULL, NULL, NULL };
						
						// Work around <rdar://problem/5530166>.  This dummy call to 
						// CFNetworkCopyProxiesForURL initialise some state within CFNetwork 
						// that is required by CFNetworkCopyProxiesForAutoConfigurationScript.
						
						CFRelease(CFNetworkCopyProxiesForURL(url, NULL));
						
						CFRunLoopSourceRef	rls = CFNetworkExecuteProxyAutoConfigurationURL(scriptURL, url, ResultCallback, &context);
						if (rls == NULL) {
							err = coreFoundationUnknownErr;
						}
						
						if ( err == noErr )
						{							
							CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
									
							CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e10, false);
									
							CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
							
							CFQRelease(rls);
													
							if ( res && CFGetTypeID(res) == CFArrayGetTypeID() )
							{
								bestProxy = CFArrayGetValueAtIndex(res, 0);
								if (bestProxy != NULL)
									 proxyType = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyTypeKey);
							}
						}
					}
					
				}
				
				if (bestProxy != NULL && proxyType != NULL && CFEqual(proxyType, kCFProxyTypeHTTP))
				{
					CFStringRef hostStr = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyHostNameKey);

					if ( hostStr != NULL ) {
						CFNumberRef portNum = (CFNumberRef) CFDictionaryGetValue(bestProxy, kCFProxyPortNumberKey );
						
						if ( portNum != NULL )
						{
							CFStringRef fullHost = CFStringCreateWithFormat( NULL, NULL, CFSTR("http://%@:%@"), hostStr, portNum);
							
							if ( fullHost != NULL )
							{
								CFStringGetCString(fullHost, (char*)host,
									(CFIndex) host_len, kCFStringEncodingUTF8);
									
								enabled = true;
									
								CFRelease(fullHost);
							}
						}

					}
                    
                    CFStringRef userNameStr = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyUsernameKey);
                    
					if ( userNameStr != NULL ) {
                        CFStringGetCString(userNameStr, (char*)user, (CFIndex) user_len, kCFStringEncodingUTF8);
					}
                    
                    CFStringRef passwordStr = (CFStringRef) CFDictionaryGetValue(bestProxy, kCFProxyPasswordKey);
                    
					if ( passwordStr != NULL ) {
                        CFStringGetCString(passwordStr, (char*)pass, (CFIndex) pass_len, kCFStringEncodingUTF8);
					}

				}
			}
		}
	}
	
	// Clean up.
	
	CFQRelease(proxies);
	CFQRelease(proxySettings);
	CFQRelease(url);
	CFQRelease(res);
	
	return enabled;
}
