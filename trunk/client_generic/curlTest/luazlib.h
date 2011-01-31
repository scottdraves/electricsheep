// $Id: luazlib.h 1045 2006-05-19 23:48:17Z kaos $

#ifndef __LUA_ZLIB_H__
#define __LUA_ZLIB_H__

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};


#ifndef LUAZLIB_API
#define LUAZLIB_API	LUA_API
#endif

#define LUA_ZLIBNAME	"zlib"
LUAZLIB_API int luaopen_zlib( struct lua_State * L );

#endif //__LUA_ZLIB_H__
