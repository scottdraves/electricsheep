
#include "ESScreensaver.h"
#include "mgl.h"
#include "client.h"
#include "client_mac.h"
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>


CElectricSheep_Mac	gClient;

CFBundleRef dlbundle_ex( void )
{
	
	CFBundleRef bundle = NULL;
	Dl_info info;
	
	if( dladdr( (const void *)__func__, &info ) ) {
		
		const char *bundle_path = dirname( (char *)info.dli_fname );
		
		do {
			CFURLRef bundleURL = CFURLCreateFromFileSystemRepresentation( kCFAllocatorDefault, (UInt8 *)bundle_path, strlen( bundle_path ), true );
			
			bundle = CFBundleCreate( kCFAllocatorDefault, bundleURL );
			
			CFRelease(bundleURL);
			
			if (bundle == NULL)
				return NULL;
			
			if( CFBundleGetValueForInfoDictionaryKey( bundle, kCFBundleExecutableKey ) != NULL ) {
				break;
			}
			bundle_path = dirname( (char *)bundle_path );
		} while( strcmp( bundle_path, "." ) );
	}
	
	return( bundle );
}

void ESScreenSaver_AddGLContext( void *_glContext )
{
	gClient.AddGLContext( (CGLContextObj)_glContext );
}

bool ESScreensaver_Start( bool _bPreview, uint32 _width, uint32 _height )
{	
	if (g_Player().Display() == NULL)
	{
		DisplayOutput::CMacGL::SetDefaultWidthAndHeight( _width, _height );
		
		gClient.SetIsPreview( _bPreview );
				
		gClient.Startup();
		//we should stop the player, if it is started by default, just to be sure.
		//g_Player().Stop();
	}
		
	if (g_Player().Display() != NULL)
	{		
		//g_Player().Display()->SetContext((CGLContextObj)_glContext);
		g_Player().Display()->ForceWidthAndHeight( _width, _height );
	}

	//g_Player().SetGLContext((CGLContextObj)_glContext, _bPreview);
	
	//if (g_Player().Renderer() != NULL)
	//{
		//g_Player().Renderer()->Defaults();
	//}
	
	//if (g_Player().Stopped())
		//g_Player().Start();
		
	return true;
}

bool ESScreensaver_DoFrame( void )
{
	bool retval = true;
		
	if( gClient.Update() == false )
	{
		retval = false;
	}
	
	return retval;
}

void ESScreensaver_Stop( void )
{
	g_Player().Stop();
}

bool ESScreensaver_Stopped( void )
{
	return g_Player().Stopped();
}

void ESScreensaver_ForceWidthAndHeight( uint32 _width, uint32 _height )
{
	g_Player().Display()->ForceWidthAndHeight( _width, _height );
}

void ESScreensaver_Deinit( void )
{
	gClient.Shutdown();
}

void ESScreensaver_AppendKeyEvent( UInt32 keyCode )
{
	using namespace DisplayOutput;
		
	if ( g_Player().Display() != NULL )
	{
		CKeyEvent *spEvent = new CKeyEvent();
		spEvent->m_bPressed = true;
		
		switch( keyCode )
		{
			case 0x7B:	spEvent->m_Code = CKeyEvent::KEY_LEFT;	break;
			case 0x7C:	spEvent->m_Code = CKeyEvent::KEY_RIGHT;	break;
			case 0x7E:	spEvent->m_Code = CKeyEvent::KEY_UP;	break;
			case 0x7D:	spEvent->m_Code = CKeyEvent::KEY_DOWN;	break;
			case 0x31:	spEvent->m_Code = CKeyEvent::KEY_SPACE;	break;
			case 0x7A:	spEvent->m_Code = CKeyEvent::KEY_F1;	break;
			case 0x78:	spEvent->m_Code = CKeyEvent::KEY_F2;	break;
			case 0x63:	spEvent->m_Code = CKeyEvent::KEY_F3;	break;
			case 0x76:	spEvent->m_Code = CKeyEvent::KEY_F4;	break;
			case 0x64:	spEvent->m_Code = CKeyEvent::KEY_F8;	break;
			case 0x35:	spEvent->m_Code = CKeyEvent::KEY_Esc;	break;
		}
		
		spCEvent e = spEvent;
		g_Player().Display()->AppendEvent( e );
	}	
}

CFStringRef ESScreensaver_GetVersion( void )
{
	return gClient.GetBundleVersion();
}


void ESScreensaver_InitClientStorage( void )
{
	gClient.SetUpConfig();
}

CFStringRef ESScreensaver_GetStringSetting( const char *url, const char *defval )
{	
	std::string val = g_Settings()->Get( std::string(url), std::string(defval) );
	
	return CFStringCreateWithCString( NULL, val.c_str(), kCFStringEncodingUTF8 );
}

SInt32 ESScreensaver_GetIntSetting( const char *url, const SInt32 defval )
{	
	return g_Settings()->Get(std::string(url), (int32)defval );
}

bool ESScreensaver_GetBoolSetting( const char *url, const bool defval )
{	
	return g_Settings()->Get(std::string(url), (bool)defval );
}

void ESScreensaver_SetStringSetting( const char *url, const char *val )
{
	g_Settings()->Set(std::string(url), std::string(val));
}

void ESScreensaver_SetIntSetting( const char *url, const SInt32 val )
{
	g_Settings()->Set(std::string(url), (int32)val);
}

void ESScreensaver_SetBoolSetting( const char *url, const bool val )
{
	g_Settings()->Set(std::string(url), val);
}
	
void ESScreensaver_DeinitClientStorage( void )
{
	g_Settings()->Shutdown();
}

void ESScreensaver_SetUpdateAvailable( const char *verinfo)
{
	gClient.SetUpdateAvailable(verinfo);
}
