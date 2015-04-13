
#include "ESScreensaver.h"
#include "mgl.h"
#include "client.h"
#include "client_mac.h"
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#include "tinyxml.h"


CElectricSheep_Mac	gClient;

CFBundleRef CopyDLBundle_ex( void )
{
	
	CFBundleRef bundle = NULL;
	Dl_info info;
	
	if( dladdr( (const void *)__func__, &info ) ) {
		
		const char *bundle_path = dirname( (char *)info.dli_fname );
		
		do {
			if (bundle != NULL)
            {
                CFRelease(bundle);
                bundle = NULL;
            }
            
            CFURLRef bundleURL = CFURLCreateFromFileSystemRepresentation( kCFAllocatorDefault, (UInt8 *)bundle_path, (CFIndex)strlen( bundle_path ), true );
			
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
		
    //g_Player().Display()->SetContext((CGLContextObj)_glContext);
    g_Player().ForceWidthAndHeight(0, _width, _height );

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
	g_Player().ForceWidthAndHeight(0, _width, _height );
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

CFStringRef ESScreensaver_CopyGetRoot( void )
{	
	std::string root = g_Settings()->Get("settings.content.sheepdir", g_Settings()->Root() + "content");
	
	if (root.empty())
	{
		root = g_Settings()->Root() + "content";
		g_Settings()->Set("settings.content.sheepdir", root);
	}
	
	return CFStringCreateWithCString( NULL, root.c_str(), kCFStringEncodingUTF8 );
}


CFStringRef ESScreensaver_CopyGetStringSetting( const char *url, const char *defval )
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

double ESScreensaver_GetDoubleSetting( const char *url, const double defval )
{	
	return g_Settings()->Get(std::string(url), (fp8)defval );
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

void ESScreensaver_SetDoubleSetting( const char *url, const double val )
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

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

static uint64 GetFlockSizeBytes(const std::string& path, int sheeptype)
{
	std::string mpegpath(path);
	
	if (mpegpath.substr(mpegpath.size() - 1, 1) != "/")
		mpegpath += "/";
	uint64 retval = 0;

	try {
	boost::filesystem::path p(mpegpath.c_str());

	directory_iterator end_itr; // default construction yields past-the-end
	for ( directory_iterator itr( p );
			itr != end_itr;
			++itr )
	{
		if (!is_directory(itr->status()))
		{
			std::string fname(itr->path().filename().string());
			
			std::string ext(itr->path().extension().string());
			
			if ( ext == std::string(".avi"))
			{
				int generation;
				int id;
				int first;
				int last;

				if( 4 == sscanf( fname.c_str(), "%d=%d=%d=%d.avi", &generation, &id, &first, &last ) )
				{
					if ( (generation >= 10000 && sheeptype == 1) || (generation < 10000 && sheeptype == 0) )
					{
						struct stat sbuf;

						if (stat( (mpegpath + "/" +fname).c_str(), &sbuf ) == 0)
							retval += (uint64)sbuf.st_size;
					}
				}
			}
		}
		else
			retval += GetFlockSizeBytes(itr->path().string(), sheeptype);
	}

	}
	catch(boost::filesystem::filesystem_error& err)
	{
		g_Log->Error( "Path enumeration threw error: %s",  err.what() );
		return 0;
	}
	return retval;
}

size_t ESScreensaver_GetFlockSizeMBs(const char *mpegpath, int sheeptype)
{
	return GetFlockSizeBytes(mpegpath, sheeptype)/1024/1024;

}

CFStringRef ESScreensaver_CopyGetRoleFromXML(const char *xml)
{
	TiXmlDocument doc;
	if ( doc.Parse(xml, NULL, TIXML_ENCODING_UTF8 ) )
	{
		TiXmlHandle hDoc(&doc);
		TiXmlElement* listElement;
		const char *role = NULL;

		listElement=hDoc.FirstChild( "query" ).FirstChild( "redir" ).Element();

		if ( listElement != NULL )
			role = listElement->Attribute("role");

		if ( role != NULL )
		{
			return CFStringCreateWithCString(kCFAllocatorDefault, role, kCFStringEncodingUTF8);

		}
	}
	
	return NULL;
}
