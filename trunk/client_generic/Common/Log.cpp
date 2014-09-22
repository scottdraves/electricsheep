#include	<stdarg.h>
#include	<inttypes.h>
#include	<string>
#include	<time.h>
#include	<sstream>

#include	"base.h"
#include	"Log.h"

//	Lua.
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
};

namespace	Base
{

/*
	CLog().

*/
CLog::CLog() : m_bActive( false ), m_pFile(NULL)
{
}


/*
	~CLog().

*/
CLog::~CLog()
{
	//	Mark singleton as shutdown, to track unwanted access after this point.
	SingletonActive( false );
}

/*
	Startup().

*/
bool	CLog::Startup()
{
	//if( (
		//m_pStdout = freopen( "debug.txt", "a+", stdout ); 
			//) == NULL )
		//exit(-1);
	m_pState = NULL;
	m_pFile = NULL;
	return( true );
}

/*
	Shutdown().

*/
bool	CLog::Shutdown( void )
{
	Info( "Logger shutting down..." );
	Info( "dummy" );
	Detach();

	//if( m_pStdout )
        //fclose( m_pStdout );

	return( true );
}

/*
*/
void	CLog::Attach( const std::string &_location, const uint32 /*_level*/ )
{
	time_t	curTime;
	time( &curTime );

	char	timeStamp[32] = { 0 };
	strftime( timeStamp, sizeof(timeStamp), "%Y_%m_%d", localtime( &curTime ) );

    std::stringstream s;
	s << _location << timeStamp << ".log";
	std::string f = s.str();
    if( (m_pFile = freopen( f.c_str(), "a+", stdout ) )== NULL )
	{
#ifdef WIN32
		MessageBoxA(NULL, "Unable to create log file, exiting. Please check file permissions for electricsheep folder.", "Log error", MB_OK);
#endif
        exit( -1 );
	}

/*
	m_pState = new Base::Script::CLuaState();
	m_pState->Init( "" );

	//	Load embedded scripts.
	if( luaL_loadbuffer( m_pState->GetState(),(const char*)luaLogging, sizeof(luaLogging), "logging.lua" ) == 0 )
		lua_pcall( m_pState->GetState(), 0, 0, 0 );

	//
	m_pState->Execute( "require 'os'" );
	m_pState->Execute( "require 'io'" );
	m_pState->Execute( "require 'table'" );
	m_pState->Execute( "require 'string'" );
	m_pState->Execute( "function createLogFile( _location )	logfile = logging.file( _location .. '%s.log', '%y-%m-%d' )	end" );
	m_pState->Execute( "function g_Log( _level, _file, _func, _line, _timestamp, _msg )	logfile:log( _level, _timestamp .. '->['.._level..']:'.._func .. '('..tostring(_line)..'): '.. tostring(_msg) ) end" );
	m_pState->Pop( Script::Call( m_pState->GetState(), "createLogFile", "s", _location.c_str() ) );
*/
	m_bActive = true;

}

/*
*/
void	CLog::Detach( void )
{
	m_bActive = false;
	SAFE_DELETE( m_pState );

	if( m_pFile )
        fclose( m_pFile );
}

/*
	SetFile().
	Store file that did the logging.
*/
void	CLog::SetInfo( const char *_pFileStr, const uint32 _line, const char *_pFunc )
{
	//boost::mutex::scoped_lock locker( m_Lock );

	std::string tmp = _pFileStr;
	size_t offs = tmp.find_last_of( "/\\", tmp.size() );

	m_File = tmp.substr( offs+1, tmp.size() );
	m_Function = std::string( _pFunc );
	m_Line = _line;
}

/*
*/

char CLog::s_MessageSpam[ m_MaxMessageLength ] = {0};
char CLog::s_MessageType[ m_MaxMessageLength ] = {0};
size_t CLog::s_MessageSpamCount = 0;

void	CLog::Log( const char *_pType, /*const char *_file, const uint32 _line, const char *_pFunc,*/ const char *_pStr )
{
	//boost::mutex::scoped_lock locker( m_Lock );

	if (m_pFile == NULL)
		return;

	time_t	curTime;
	time( &curTime );

	char	timeStamp[32] = { 0 };
	strftime( timeStamp, sizeof(timeStamp), "%H:%M:%S", localtime( &curTime ) );

	/* log spam start*/
	if (strcmp(_pStr, s_MessageSpam) == 0)
	{
		++s_MessageSpamCount;
		return;
	} else
	{
		if (s_MessageSpamCount > 0)
		{
			if( m_bActive )
			{
				fprintf( m_pFile, "[%s-%s]: '%s' x%lu\n", s_MessageType, timeStamp, s_MessageSpam, s_MessageSpamCount );
				fflush( m_pFile );
			}
			else
			{
				fprintf( stdout, "[%s-%s]: '%s' x%lu\n", s_MessageType, timeStamp, s_MessageSpam, s_MessageSpamCount );
				fflush( stdout );
			}
		} else
		{
			if( !m_bActive )
			{
				//	Not active/attached, dump to stdout.
				//fprintf( stdout, "[%s]: %s - %s[%s(%d)]: '%s'\n", _pType, timeStamp, _file, _pFunc, m_Line, _pStr );
				fprintf( stdout, "[%s-%s]: '%s'\n", s_MessageType, timeStamp, s_MessageSpam );
				fflush( stdout );
			}
			else
			{
				//fprintf( m_pFile, "[%s]: %s - %s[%s(%d)]: '%s'\n", _pType, timeStamp, _file, _pFunc, m_Line, _pStr );
				fprintf( m_pFile, "[%s-%s]: '%s'\n", s_MessageType, timeStamp, s_MessageSpam );
				fflush( m_pFile );
			}
		}

		s_MessageSpamCount = 0;
		memcpy(s_MessageSpam, _pStr, m_MaxMessageLength);
		strcpy(s_MessageType, _pType);
		return;
	}
	/* log spam end */

	if( !m_bActive )
	{
		//	Not active/attached, dump to stdout.
		//fprintf( stdout, "[%s]: %s - %s[%s(%d)]: '%s'\n", _pType, timeStamp, _file, _pFunc, m_Line, _pStr );
		fprintf( stdout, "[%s-%s]: '%s'\n", _pType, timeStamp, _pStr );
		fflush( stdout );
	}
	else
	{
		//fprintf( m_pFile, "[%s]: %s - %s[%s(%d)]: '%s'\n", _pType, timeStamp, _file, _pFunc, m_Line, _pStr );
		fprintf( m_pFile, "[%s-%s]: '%s'\n", _pType, timeStamp, _pStr );
		fflush( m_pFile );

		/*lua_State *pState = m_pState->GetState();

		uint32	stackSizeIn = lua_gettop( pState );
		lua_getglobal( pState, "g_Log" );

		lua_pushstring( pState, _pType );
		lua_pushstring( pState, _file );
		lua_pushstring( pState, _pFunc );
		lua_pushinteger( pState, _line );
		lua_pushstring( pState, timeStamp );
		lua_pushstring( pState, _pStr );

		luaL_checkstack( pState, 1, "Too many arguments" );

		int status = lua_pcall( pState, 6, 0, 0 );
		if( status != 0 )
		{
			if( !lua_isnil( pState, -1 ) )
			fprintf( stdout, "LUA ERROR: %s\n", lua_tostring( pState, -1 ) );

			fprintf( stdout, "[%s]: %s - %s[%s(%d)]: '%s'\n", _pType, timeStamp, _file, _pFunc, m_Line, _pStr );
			fflush( stdout );

			//	Detach.
			Detach();
		}*/
	}
}

#define	grabvarargs	\
	va_list	ArgPtr;	\
	char	pTempStr[ m_MaxMessageLength ];	\
	va_start( ArgPtr, _pFmt );	\
	vsnprintf( pTempStr, m_MaxMessageLength, _pFmt, ArgPtr );	\
	va_end( ArgPtr );

//	Def our loggers.
void	CLog::Debug( const char *_pFmt, ... )	{	grabvarargs	Log( "DEBUG", /*m_File.c_str(), m_Line, m_Function.c_str(),*/ pTempStr );	}
void	CLog::Info( const char *_pFmt, ... )	{	grabvarargs	Log( "INFO", /*m_File.c_str(), m_Line, m_Function.c_str(),*/ pTempStr );	}
void	CLog::Warning( const char *_pFmt, ... )	{	grabvarargs	Log( "WARN", /*m_File.c_str(), m_Line, m_Function.c_str(),*/ pTempStr );	}
void	CLog::Error( const char *_pFmt, ... )	{	grabvarargs	Log( "ERROR", /*m_File.c_str(), m_Line, m_Function.c_str(),*/ pTempStr );	}
void	CLog::Fatal( const char *_pFmt, ... )	{	grabvarargs	Log( "FATAL", /*m_File.c_str(), m_Line, m_Function.c_str(),*/ pTempStr );	}

};
