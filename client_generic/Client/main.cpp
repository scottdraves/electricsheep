#include <string>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#if defined(WIN32) && !defined(_MSC_VER)
#include <dirent.h>
#endif
#include <cstring>
#ifdef WIN32
#include <process.h>
#include <windows.h>
#endif
#include <float.h>
#include <signal.h>

#include "client.h"

#ifdef	WIN32
class ExceptionHandler
{
    public:

    ExceptionHandler()
    {
        LoadLibraryA( "exchndl.dll" );
    }
};

static ExceptionHandler gExceptionHandler;	//  global instance of class

	#include	"client_win32.h"
	typedef CElectricSheep_Win32	CElectricSheepClient;
#else
#ifdef MAC
#include    <OpenGL/gl.h>
#include    <GLUT/glut.h>
#include	"client_mac.h"
typedef CElectricSheep_Mac	CElectricSheepClient;
#else
#include    <GL/gl.h>
#include    <GL/glut.h>
#include	"client_linux.h"
typedef CElectricSheep_Linux	CElectricSheepClient;
#endif
#endif

//
#ifdef	WIN32
int32 APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
#else
int32 main( int argc, char *argv[] )
{
	glutInit( &argc, argv );
#endif

    //	Start log (unattached).
    g_Log->Startup();

	CElectricSheepClient	client;

#ifdef	WIN32
	boost::filesystem::path::default_name_check( boost::filesystem::native );
    ULONG lowfragmentationheap = 2;
    HeapSetInformation(GetProcessHeap(), HeapCompatibilityInformation, &lowfragmentationheap, sizeof(lowfragmentationheap));
#endif

	if( client.Startup() )
		client.Run();

//    g_Log->Info( "Raising access violation...\n" );
//    asm( "movl $0, %eax" );
//    asm( "movl $1, (%eax)" );

//    __asm("int3");

	client.Shutdown();

	g_Log->Shutdown();

	return 0;
}
