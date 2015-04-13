/*
	LUASTATE.CPP
	Author: Stef.

*/
//#include	<io.h>
#include	<limits.h>
#include	<stdint.h>
#include	<string.h>
#ifdef	WIN32
#include	<io.h>
#endif

#include	"Log.h"
#include	"base.h"
#include	"Exception.h"
#include	"LuaState.h"

//	Lua.
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
};

#ifdef LINUX_GNU
#include <limits.h>
#define MAX_PATH PATH_MAX
#endif

#ifndef	MAX_PATH
#define	MAX_PATH 4096
#endif

namespace	Base	{
namespace	Script	{

/*
	luaInstallation().

*/
static const char *luaInstallation = "								\
	function SetInstallation( _luaPath )							\
		print( 'SetInstallation: ' .. tostring(_luaPath) )			\
		require 'string'											\
		LUA_PATH = _luaPath .. '/?.lua;'							\
		LUA_PATH = LUA_PATH .. _luaPath .. '/?/?.lua;'				\
		LUA_CPATH = _luaPath .. '/?.dll;'							\
		LUA_CPATH = LUA_CPATH .. _luaPath .. '/?/?.dll;'			\
		package.path = LUA_PATH										\
		package.cpath = LUA_CPATH									\
	end";

/*
	addInstallation().

*/
static const char *addInstallation = "								\
	function AddInstallation( _package )							\
		print( 'AddInstallation: ' .. tostring(_package) )			\
		require 'string'											\
		LUA_PATH = LUA_PATH .. _package .. '/?.lua;'				\
		LUA_PATH = LUA_PATH .. _package .. '/?/?.lua;'				\
		LUA_CPATH = LUA_CPATH .. _package .. '/?.dll;'				\
		LUA_CPATH = LUA_CPATH .. _package .. '/?/?.dll;'			\
		package.path = LUA_PATH										\
		package.cpath = LUA_CPATH									\
	end";

//
static int	LuaAlert( ::lua_State *_pState )
{
	ASSERT( _pState != NULL );
	const char *pMsg = lua_tostring( _pState, -1 );
	//ThrowStr( pMsg );
	g_Log->Error( "Lua Alert: %s", pMsg );
	return( 0 );
}


/*
	CLuaState().

*/
CLuaState::CLuaState()
{
}

/*
	~CLuaState().

*/
CLuaState::~CLuaState()
{
	if( m_pState )
		lua_close( m_pState );
}

//
void	CLuaState::DumpStack()
{
	g_Log->Debug( "stack: %d\n", lua_gettop( m_pState ) );
}

/*
	registerLib().
	Pre-register library, so it's up to the script to require libraries.
*/
void	CLuaState::registerLib( const char *_pName, lua_CFunction _func )
{
	lua_getglobal( m_pState, "package" );
	lua_getfield( m_pState, -1, "preload" );	//	'package.preload'
	lua_pushcfunction( m_pState, _func );
	lua_setfield( m_pState, -2, _pName );	//	'package.preload[ _pName ] = f'
	lua_pop( m_pState, 2 );					//	Pop package & preload tables.
}

/*
	AddPackagePath().

*/
static int c_AddPackagePath( lua_State *_pState )
{
	const char *packagePath = luaL_checkstring( _pState, 1 );

	bool bRelative = true;

	if( lua_isboolean( _pState, 2 ) )
		if( lua_toboolean( _pState, 2 ) == 0 )
			bRelative = false;

	char  dirBuf[ MAX_PATH + 1];
#if defined(WIN32) && defined(_MSC_VER)
	std::string dir;
	if (GetCurrentDirectoryA(sizeof(dirBuf) - 1, dirBuf))
		dir = dirBuf;
#else
	std::string dir = getcwd( &dirBuf[0], MAX_PATH + 1 );
#endif

	if( bRelative )
		dir += packagePath;
	else
		dir = packagePath;

	Call( _pState, "AddInstallation", "s", (const char *)dir.c_str() );

	return( 0 );
}

/*
	Init().

*/
void	CLuaState::Init( const std::string &_basePath )
{
	g_Log->Info( "Lua base bath: %s", _basePath.c_str() );

	//	Create state.
	m_pState = luaL_newstate();

	g_Log->Info( "stack: %d\n", lua_gettop( m_pState ) );

	lua_gc( m_pState, LUA_GCSTOP, 0 );

	lua_cpcall( m_pState, luaopen_base, NULL );		//	Base library.
	lua_cpcall( m_pState, luaopen_package, NULL );	//	Package library. (we need this to be able to 'require' from lua)

#ifdef LUA_JIT
	lua_cpcall( m_pState, luaopen_jit, NULL );
#endif

#if defined(AMD64) || defined(__LP64__)
	Execute( "_PLATFORM = 'x64'" );
#else
	Execute( "_PLATFORM = 'x86'" );
#endif

	//	The following will be pre-registered, and only loaded explicitly from the script.
	registerLib( "io", luaopen_io );
	registerLib( "os", luaopen_os );
	registerLib( "table", luaopen_table );
	registerLib( "string", luaopen_string );
	registerLib( "math", luaopen_math );
	registerLib( "debug", luaopen_debug );

	lua_register( m_pState, "_ALERT", &LuaAlert );

	//
	lua_pushcfunction( m_pState, c_AddPackagePath );
	lua_setglobal( m_pState, "c_AddPackagePath" );

	//
	std::string cmd = luaInstallation;
	Execute( luaInstallation );

	//
	cmd = addInstallation;
	Execute( addInstallation );

	Call( m_pState, "SetInstallation", "s", (const char *)_basePath.c_str() );

	//	Start collector again.
	lua_gc( m_pState, LUA_GCRESTART, 0 );
}

/*
*/
static int traceback( lua_State *_pLuaState )
{
	lua_getfield( _pLuaState, LUA_GLOBALSINDEX, "debug" );
	if (!lua_istable( _pLuaState, -1 ) )
	{
		lua_pop( _pLuaState, 1 );
		return 1;
	}

	lua_getfield( _pLuaState, -1, "traceback" );

	if( !lua_isfunction( _pLuaState, -1 ) )
	{
		lua_pop( _pLuaState, 2 );
		return 1;
	}

	lua_pushvalue( _pLuaState, 1 );		//	Pass error message.
	lua_pushinteger( _pLuaState, 2 );	//	Skip this function and traceback.
	lua_call( _pLuaState, 2, 1 );		//	Call debug.traceback.

	return 1;
}

/*
*/
static int docall( lua_State *_pLuaState, int narg, int clear )
{
	int status;
	int base = lua_gettop( _pLuaState ) - narg;		//	Function index.

	lua_pushcfunction( _pLuaState, traceback );		//	Push traceback function.
	lua_insert( _pLuaState, base );					//	Put it under chunk and args.

	status = lua_pcall( _pLuaState, narg, (clear ? 0 : LUA_MULTRET ), base );

	lua_remove( _pLuaState, base );					//	Remove traceback function.

	//	Force a complete garbage collection in case of errors */
	if(	status != 0 )
		lua_gc( _pLuaState, LUA_GCCOLLECT, 0 );

	return( status );
}


/*
	reportLua().

*/
static bool reportLua( lua_State *_pLuaState, const int32 _status )
{
	if( _status && !lua_isnil( _pLuaState, -1 ) )
	{
		const char *pMsg = lua_tostring( _pLuaState, -1 );

		if( !pMsg )
			pMsg = "(error object is not a string)";

		g_Log->Error( "%s", pMsg );
		//ThrowArgs( ("CLuaState(): %s", pMsg) );

		lua_pop( _pLuaState, 1 );

		return false;
	}

	return true;
}

/*
	Execute().
	Execute lua code.
*/
bool	CLuaState::Execute( const std::string &_command )
{
	int status = luaL_loadstring( m_pState, _command.c_str() );
	if( status == 0 )
		status = docall( m_pState, 0, 1 );

	return reportLua( m_pState, status );
}


//
/*static void	LuaCall( lua_State *L, int narg, int clear )
{
	int status;
	lua_checkstack( L, LUA_MINSTACK );
	int top  = lua_gettop(L);
	int base = top - narg;  // function index
	lua_pushliteral(L, "_TRACEBACK");
	lua_rawget(L, LUA_GLOBALSINDEX);  // get traceback function
	lua_insert(L, base);  // put it under chunk and args
	//status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
	status = lua_pcall(L, narg, clear, base );
	lua_remove(L, base);  // remove traceback function
	//status = LuaError(L, status, top - (narg + 1));
	reportLua( L, status );

	lua_settop( L, top );
}*/

/*
	Run().

*/
bool	CLuaState::Run( const std::string &_script )
{
	int status = luaL_loadfile( m_pState, _script.c_str() );
	if( status == 0 )
		status = lua_pcall( m_pState, 0, 0, 0 );

	reportLua( m_pState, status );

	if( status != 0 )
		return false;

	return( true );
}

/*
	Pop().

*/
void	CLuaState::Pop( const int32 _num )
{
	if( _num != 0 )
		lua_pop( m_pState, _num );
}

/*
	Call().

*/
int32 Call( lua_State *_pState, const char *_pFunc, const char *_pSig, ... )
{
	ASSERT( _pState != NULL );

	uint32	stackSizeIn = static_cast<uint32>(lua_gettop( _pState ));
	if( stackSizeIn != 0 )
	{
		g_Log->Error( "Lua stack not empty..." );
		return 0;
	}

	int delta = 0;

	va_list	pArg;

	//try
	{
		//	Number of arguments and results.
		int	narg, nres;

		//	Get function.
		lua_getglobal( _pState, _pFunc );

		//	Push arguments.
		narg = 0;

		char	*pBinarySource = NULL;
		int32	binaryLen = 0;

		//	Gimme args.
		va_start( pArg, _pSig );

		while( *_pSig )
		{
			//	Push arguments.
			switch( *_pSig++ )
			{
				case 'd':  //	Double argument.
						lua_pushnumber( _pState, va_arg( pArg, fp8 ) );
						break;

				case 'i':  //	Int argument.
						lua_pushinteger( _pState, va_arg( pArg, int32 ) );
						break;

				case 'b':  //	Bool argument.
						lua_pushboolean( _pState, va_arg( pArg, int32 ) != 0 );
						break;

				case 'x':	//	Binary string argument. Next argument MUST be an integer. (which will be skipped)
						pBinarySource = va_arg( pArg, char * );
						binaryLen = va_arg( pArg, int32 );
						lua_pushlstring( _pState, pBinarySource, static_cast<size_t>(binaryLen) );
						_pSig++;
						break;

				case 's':	//	String argument.
						lua_pushstring( _pState, va_arg( pArg, char * ) );
						break;

				case '>':
						goto endwhileLabel;

				default:
				{
					g_Log->Error( "Invalid option (%c)", *(_pSig - 1) );
					return 0;
				}
			}

			//printf( "stack: %d", lua_gettop( _pState ) );

			narg++;
			luaL_checkstack( _pState, 1, "Too many arguments" );

		}	endwhileLabel:

		//	Do the call.
		nres = (int)strlen( _pSig );  //	Number of expected results.

		//LuaCall( _pState, narg, nres );
		//int	status = lua_pcall( _pState, narg, nres, 0 );
		int status = docall( _pState, narg, 0 );

		reportLua( _pState, status );

		//	Retrieve results.
		nres = -nres;	//	Stack index of first result.

		while( *_pSig )	//	Get results.
		{
			switch( *_pSig++ )
			{
				case 'd':	//	Double result.
						if( !lua_isnumber( _pState, nres ) )
							g_Log->Error( "Wrong result type, expected double" );
						else
							*va_arg( pArg, fp8 *) = lua_tonumber( _pState, nres );

						break;


				case 'b':	//	Bool result.
						if( !lua_isboolean( _pState, nres ) )
							g_Log->Error( "Wrong result type, expected bool" );
						else
							*va_arg( pArg, bool *) = lua_toboolean( _pState, nres ) != 0;

						break;

				case 'i':	//	Int result.
						if( !lua_isnumber( _pState, nres ) )
							g_Log->Error( "Wrong result type, expected integer" );
						else
							*va_arg( pArg, int32 *) = (int32)lua_tonumber( _pState, nres );

						break;

				case 's':	//	String result.
						if( !lua_isstring( _pState, nres ) )
							g_Log->Error( "Wrong result type, expected string" );
						else
							*va_arg( pArg, const char **) = lua_tostring( _pState, nres );

						break;

				default:
				{
					g_Log->Error( "Invalid option (%c)", *(_pSig - 1) );
					break;
				}
			}

			nres++;
		}

		/*
			HACK!
			Pop delta from stack, keeping this function from raceing away!.
		*/
		delta = lua_gettop( _pState ) - static_cast<int>(stackSizeIn);
	}
	/*catch( Base::CException &_e )
	{
		va_end( pArg );

		//	Make sure to clean up stack in case of errors.
		delta = lua_gettop( _pState ) - stackSizeIn;
		if( delta != 0 )
			lua_pop( _pState, delta );

		g_Log->Warning( "%s", _e.Text().c_str() );
		//_e.ReportCatch();
		return 0;
	}*/

	va_end( pArg );

	return( delta );
}

};

};
