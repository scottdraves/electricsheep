#include <CoreFoundation/CoreFoundation.h>
#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif
	CFBundleRef dlbundle_ex( void );
	void ESScreenSaver_AddGLContext( void *_glContext );
	
	bool ESScreensaver_Start( bool _bPreview, uint32 _width, uint32 _height );
	bool ESScreensaver_DoFrame( void );
	void ESScreensaver_Stop( void );
	bool ESScreensaver_Stopped( void );
	void ESScreensaver_ForceWidthAndHeight( uint32 _width, uint32 _height );
	void ESScreensaver_Deinit( void );
	
	void ESScreensaver_AppendKeyEvent( UInt32 keyCode );
	
	CFStringRef ESScreensaver_GetVersion( void );
	
	CFStringRef ESScreensaver_GetStringSetting( const char *url, const char *defval );
	SInt32 ESScreensaver_GetIntSetting( const char *url, const SInt32 defval );
	bool ESScreensaver_GetBoolSetting( const char *url, const bool defval );
	
	void ESScreensaver_SetStringSetting( const char *url, const char *val );
	void ESScreensaver_SetIntSetting( const char *url, const SInt32 val );
	void ESScreensaver_SetBoolSetting( const char *url, const bool val );
	
	void ESScreensaver_SaveSettings( void );
	
	void ESScreensaver_InitClientStorage( void );
	void ESScreensaver_DeinitClientStorage( void );
	
	void ESScreensaver_SetUpdateAvailable( const char *verinfo );
	
	size_t ESScreensaver_GetFlockSizeMBs(const char *mpegpath, int sheeptype);
	
	CFStringRef ESScreensaver_GetRoleFromXML(const char *xml);
	
#ifdef __cplusplus
}
#endif