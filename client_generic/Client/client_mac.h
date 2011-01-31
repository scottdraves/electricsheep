#ifndef CLIENT_MAC_H_INCLUDED
#define CLIENT_MAC_H_INCLUDED

#ifndef MAC
#error	This file is not supposed to be used for this platform...
#endif

#include <string>
#include <SystemConfiguration/SystemConfiguration.h>
#include "base.h"
#include "MathBase.h"
#include "Exception.h"
#include "Log.h"
#include "Player.h"
#include "SimplePlaylist.h"
#include "lua_playlist.h"
#include "Timer.h"
#include "storage.h"
#include "Settings.h"
#include "Splash.h"

#include "dlfcn.h"
#include "libgen.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

#include "ESScreensaver.h"

/*
	CElectricSheep_Mac().
	Mac specific client code.
*/
class	CElectricSheep_Mac : public CElectricSheep
{	
	std::vector<CGLContextObj> m_glContextList;
	bool m_bIsPreview;
	UInt8 m_proxyHost[256];
	UInt8 m_proxyUser[32];
	UInt8 m_proxyPass[32];
	Boolean m_proxyEnabled;
	std::string m_verStr;
	
	typedef Boolean (*get_proxy_for_serverT)( const UInt8 *server, UInt8 *host, const UInt32 host_len, UInt8* user, const UInt32 user_len, UInt8 *pass, const UInt32 pass_len );
		
	public:
			CElectricSheep_Mac() : CElectricSheep()
			{
				printf( "CElectricSheep_Mac()\n" );
				
				m_bIsPreview = false;
				
				*m_proxyHost = 0;
				*m_proxyUser = 0;
				*m_proxyPass = 0;
				
				FSRef foundRef;
				
				UInt8 path[1024];
				
				OSErr err = FSFindFolder(kUserDomain, kApplicationSupportFolderType, false, &foundRef);
				
				if (err == noErr)
				{
					CFURLRef appSupportURL = CFURLCreateFromFSRef(kCFAllocatorDefault, &foundRef);
					if (appSupportURL != NULL)
					{
						if (CFURLGetFileSystemRepresentation ( appSupportURL, true, path, sizeof(path) - 1))
						{							
							m_AppData = (char *)path;
							
							m_AppData += "/ElectricSheep/";
						}
						
						CFRelease(appSupportURL);
					}
				}
				
				CFBundleRef bundle = dlbundle_ex();
				
				if (bundle != NULL)
				{
					CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);
					
					if (resourcesURL != NULL)
					{
						if (CFURLGetFileSystemRepresentation ( resourcesURL, true, path, sizeof(path) - 1))
						{						
							m_WorkingDir = (char *)path;
							
							m_WorkingDir += "/";
						}
						
						CFRelease(resourcesURL);
					}
					
					CFStringRef cfver = (CFStringRef)CFBundleGetValueForInfoDictionaryKey( bundle, CFSTR("CFBundleVersion") );

					char verstr[64];
					
					verstr[0] = '\0';
					
					CFStringGetCString(cfver, verstr, sizeof(verstr) - 1, kCFStringEncodingASCII);
					
					m_verStr.assign(verstr);
					
					CFRelease(bundle);
				}
				
				GetClientProxy();
			}
	
			virtual ~CElectricSheep_Mac()
			{
			}
			
			CFStringRef GetBundleVersion()
			{
				CFBundleRef bundle = dlbundle_ex();
				
				if (bundle == NULL)
					return NULL;
				
				CFStringRef verStr = (CFStringRef)CFBundleGetValueForInfoDictionaryKey( bundle, CFSTR("CFBundleVersion") );
				
				CFRelease(bundle);
				
				return (CFStringRef)verStr;
			}
			
			static Boolean GetProxyForServer104( const UInt8 *server, UInt8 *host, const UInt32 host_len, UInt8* user, const UInt32 user_len, UInt8 *pass, const UInt32 pass_len )
			{
				Boolean             result;
				CFDictionaryRef     proxyDict;
				CFNumberRef         enableNum;
				int                 enable;
				CFStringRef         hostStr;
				CFNumberRef         portNum;
				
				if (user && user_len > 0)
					*user = 0;

				if (pass && pass_len > 0)
					*pass = 0;
									
				// Get the dictionary.
				
				proxyDict = SCDynamicStoreCopyProxies(NULL);
				result = (proxyDict != NULL);

				// Get the enable flag.  This isn't a CFBoolean, but a CFNumber.
				
				if (result) {
					enableNum = (CFNumberRef) CFDictionaryGetValue(proxyDict,
							kSCPropNetProxiesHTTPEnable);

					result = (enableNum != NULL)
							&& (CFGetTypeID(enableNum) == CFNumberGetTypeID());
				}
				if (result) {
					result = CFNumberGetValue(enableNum, kCFNumberIntType,
								&enable) && (enable != 0);
				}
				
				// Get the proxy host.  DNS names must be in ASCII.  If you 
				// put a non-ASCII character  in the "Secure Web Proxy"
				// field in the Network preferences panel, the CFStringGetCString
				// function will fail and this function will return false.
				
				if (result) {
					hostStr = (CFStringRef) CFDictionaryGetValue(proxyDict,
								kSCPropNetProxiesHTTPProxy);

					result = (hostStr != NULL)
						&& (CFGetTypeID(hostStr) == CFStringGetTypeID());
				}
				
				// Get the proxy port.
				
				if (result) {
					portNum = (CFNumberRef) CFDictionaryGetValue(proxyDict,
							kSCPropNetProxiesHTTPPort);

					result = (portNum != NULL)
						&& (CFGetTypeID(portNum) == CFNumberGetTypeID());
				}
				if (result) {
					CFStringRef fullHost = CFStringCreateWithFormat( NULL, NULL, CFSTR("http://%@:%@"), hostStr, portNum);
					
					if (fullHost != NULL)
					{
						result = CFStringGetCString(fullHost, (char*)host,
							(CFIndex) host_len, kCFStringEncodingUTF8);
							
						CFRelease(fullHost);
					}
				}

				// Clean up.
				
				if (proxyDict != NULL) {
					CFRelease(proxyDict);
				}
				
				if ( ! result ) {
					*host = 0;
				}
				
				return result;
			}
	
			void GetClientProxy(void)
			{
				CFBundleRef proximusBundle = NULL;
				get_proxy_for_serverT get_proxy_for_server = GetProxyForServer104;
				
				CFBundleRef bundle = dlbundle_ex();
				
				if (bundle != NULL)
				{
					CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);
					
					if (resourcesURL != NULL)
					{
						CFURLRef proximusURL = CFURLCreateCopyAppendingPathComponent( NULL, resourcesURL, CFSTR("proximus.bundle"), true );
						
						if (proximusURL != NULL)
						{
							proximusBundle = CFBundleCreate( NULL, proximusURL);
							
							if (proximusBundle != NULL)
							{
								void *func_ptr = CFBundleGetFunctionPointerForName( proximusBundle, CFSTR("get_proxy_for_server105") );

								if (func_ptr)
									get_proxy_for_server = (get_proxy_for_serverT)func_ptr;
							}
						
							CFRelease(proximusURL);
						}
						
						CFRelease(resourcesURL);
					}
					
					CFRelease(bundle);
				}
				
				if (get_proxy_for_server)
					m_proxyEnabled = get_proxy_for_server( (const UInt8*)CLIENT_SERVER, m_proxyHost, sizeof(m_proxyHost) - 1, m_proxyUser, sizeof(m_proxyUser) - 1, m_proxyPass, sizeof(m_proxyPass) - 1 );
					
				if (proximusBundle != NULL)
					CFRelease(proximusBundle);
			}
			
			void AddGLContext( CGLContextObj _glContext )
			{					
				if ( g_Player().Display() == NULL )
				{
					m_glContextList.push_back( _glContext );
				}
				else
				{
					g_Player().AddDisplay( _glContext );
				}
			}
		
			void SetIsPreview( bool _isPreview )
			{
				m_bIsPreview = _isPreview;
			}	
			
			void SetUpConfig( void )
			{
				InitStorage();
			}	
	
			//
			virtual const bool	Startup()
			{
				using namespace DisplayOutput;

				printf( "Startup()\n" );

				InitStorage();
				
				AttachLog();
				
				//if m_proxyHost is set, the proxy resolver found one. If not, we should not override preferences.
				if (*m_proxyHost)
				{ 
					g_Settings()->Set( "settings.content.use_proxy", (bool)( m_proxyEnabled != 0 ) );
					
					g_Settings()->Set( "settings.content.proxy", std::string( (char*)m_proxyHost ) );
					
					//for now the resolver doesn't support user/password combination, so we should not touch it so user can set it in prefs.
					//g_Settings()->Set( "settings.content.proxy_username", std::string( (char*)m_proxyUser ) );
					//g_Settings()->Set( "settings.content.proxy_password", std::string( (char*)m_proxyPass ) );
				}

				std::string tmp = "Working dir: " + m_WorkingDir;
				g_Log->Info( tmp.c_str() );
				
				std::vector<CGLContextObj>::const_iterator it = m_glContextList.begin();
				
				for ( ; it != m_glContextList.end(); it++ )
				{
					g_Player().AddDisplay(*it);
				}
					
				m_glContextList.clear();
																
				return CElectricSheep::Startup();
			}


			//
			const bool Update()
			{
				using namespace DisplayOutput;
				
				g_Player().Framerate( m_CurrentFps );

				if( !CElectricSheep::Update() )
					return false;
				
				//	Update display events.
				g_Player().Display()->Update();

				HandleEvents();
				
				return true;
			}
			
			virtual std::string GetVersion()
			{
				return m_verStr;
			}
			
			virtual int GetACLineStatus()
			{
				CFTypeRef blob = IOPSCopyPowerSourcesInfo();
				CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
				
				CFDictionaryRef pSource = NULL;
				
				CFIndex srcCnt = CFArrayGetCount(sources);
				
				if ( srcCnt == 0 )
				{
					CFRelease(blob);
					CFRelease(sources);
					return -1;	// Could not retrieve battery information.  System may not have a battery.
				}

				bool charging = false;
				
				for (CFIndex i = 0; i < srcCnt; i++)
				{
					pSource = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, i));
					CFStringRef cfStateStr = (CFStringRef)CFDictionaryGetValue(pSource, CFSTR(kIOPSPowerSourceStateKey));
										
					if ( cfStateStr != NULL && CFEqual( cfStateStr, CFSTR( kIOPSACPowerValue ) ) )
					{
						charging = true;
						break;
					}
				}
				
				CFRelease(blob);
				CFRelease(sources);
				
				return (charging ? 1 : 0);
			}
};

#endif // CLIENT_H_INCLUDED
