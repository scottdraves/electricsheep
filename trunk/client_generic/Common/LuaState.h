/*
   LUASTATE.H
   Author: Stef.

   Scriptstate baseclass.
*/
#ifndef	_LUA_STATE_H
#define	_LUA_STATE_H

#include	"base.h"

struct lua_State;
typedef int (*lua_CFunction) (lua_State *L);

namespace   Base	{
namespace   Script	{

//
int32	Call( lua_State *_pState, const char *_pFunc, const char *_pSig, ... );

/*
	CLuaState.
	Main lua state.
*/
class	CLuaState
{
	NO_CLASS_STANDARDS( CLuaState );

	protected:

		lua_State	*m_pState;

	public:
			CLuaState();
			~CLuaState();

			//
			void	Init( const std::string &_basePath );
			void	AddPackage( const std::string &_packagePath, const bool _bRelativeRuntimeDir = true );
			void	registerLib( const char *_pName, lua_CFunction _func );

			//	Execute lua code.
			bool	Execute( const std::string &_command );

			//	Run a lua script.
			bool	Run( const std::string &_script );

			//
			lua_State	*GetState( void )	{	return( m_pState );	};

			//
			void	Pop( const int32 _num );

			void	DumpStack();
};

};

};

#endif
