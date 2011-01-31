/*
	CException.
	Author:	Stef.

	Exception handler.
*/
#ifdef	WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <sstream>
#include <string>

#include "base.h"
#include "Exception.h"
#include "Log.h"

namespace	Base
{

std::string	CException::m_ArgString;

/*
	ReportCatch().

*/
void	CException::ReportCatch( void ) const
{
//	printf( "%s\n", Text().c_str() );
	g_Log->Error( "EXCEPTION: %s", Text().c_str() );
#ifdef	WIN32
#ifdef	DEBUG
	//MessageBox( NULL, Text().c_str(), "Exception", MB_OK | MB_ICONERROR );
#endif
#endif
}

/*
	Text().

*/
std::string	CException::Text( void ) const
{
	std::ostringstream os;
	os << "in " << m_File << "(" << m_Line << "): " << m_String;
	return( std::string( os.str() ) );
}

/*
	CollectArguments().

*/
void	CException::CollectArguments( const char *String, ... )
{
	static char s_StringWork[8192];

	va_list	ArgPtr;

	va_start( ArgPtr, String );
    vsprintf( s_StringWork, String, ArgPtr );
	va_end( ArgPtr );

	m_ArgString = s_StringWork;
}

};
