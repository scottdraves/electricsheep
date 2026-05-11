#include <sstream>
#include <string>

#include	"base.h"
#include	"Log.h"
#include	"Exception.h"
#include	"luastorage.h"
#include	"clientversion.h"

#include	"boost/filesystem/path.hpp"
#include	"boost/filesystem/operations.hpp"
#include	"boost/filesystem.hpp"

using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::directory_iterator;

using namespace std;

//	Lua.
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

//	IUP stuff.
//#include "iup.h"
//#include "iupcontrols.h"
//#include "iuplua.h"
//#include "iupluacontrols.h"
};

namespace	TupleStorage
{


/*
	Set( [bool] ).

*/
bool	CStorageLua::Set( const std::string &_entry, const bool _val )
{
	assert( m_pState != NULL );
	std::stringstream s;

	if( _val == true )
		s << "g_Settings." << _entry << " = true";
	else
		s << "g_Settings." << _entry << " = false";

	m_pState->Execute( s.str().c_str() );
	Dirty( true );
	return( true );
}

/*
	Set( [integer] ).

*/
bool	CStorageLua::Set( const std::string &_entry, const int32 _val )
{
	assert( m_pState != NULL );
	std::stringstream s;
	s << "g_Settings." << _entry << " = " << _val;
	m_pState->Execute( s.str().c_str() );
	Dirty( true );
	return( true );
}

/*
	Set( [double] ).

*/
bool	CStorageLua::Set( const std::string &_entry, const fp8  _val )
{
	assert( m_pState != NULL );
	std::stringstream s;
	s << "g_Settings." << _entry << " =  [[" << _val << "]]";
	m_pState->Execute( s.str().c_str() );
	Dirty( true );
	return( true );
}

/*
	Set( [string] ).

*/
bool	CStorageLua::Set( const std::string &_entry, const std::string &_str )
{
	assert( m_pState != NULL );
	std::stringstream s;
	s << "g_Settings." << _entry << " =  [[" << _str << "]]";
	m_pState->Execute( s.str().c_str() );
	Dirty( true );
	return( true );
}

/*
	Get( &[bool] ).

*/
bool	CStorageLua::Get( const std::string &_entry, bool &_val )
{
	assert( m_pState != NULL );
	std::string s = "g_Settings." + _entry;
	int32 bSuccess = 0;
	bool ret = false;
	m_pState->Pop( Base::Script::Call( m_pState->GetState(), "g_GetSetting", "sb>ib", s.c_str(), 0, &bSuccess, &ret ) );
	_val = ret;
	return( bSuccess !=0 );
}

/*
	Get( &[integer] ).

*/
bool	CStorageLua::Get( const std::string &_entry, int32 &_val )
{
	assert( m_pState != NULL );
	std::string s = "g_Settings." + _entry;
	int32 ret = 0, bSuccess = 0;
	m_pState->Pop( Base::Script::Call( m_pState->GetState(), "g_GetSetting", "si>ii", s.c_str(), 0, &bSuccess, &ret ) );
	_val = ret;
	return( bSuccess !=0 );
}


/*
	Get( &[double] ).

*/
bool	CStorageLua::Get( const std::string &_entry, fp8 &_val )
{
	assert( m_pState != NULL );
	std::string s = "g_Settings." + _entry;
	int32 bSuccess = 0;
	char	*ret = NULL;
	m_pState->Pop( Base::Script::Call( m_pState->GetState(), "g_GetSetting", "ss>is", s.c_str(), "?", &bSuccess, &ret ) );
	if (bSuccess && ret != NULL)
	{
		std::stringstream tmp( ret );
		tmp >> _val;
	}

	return( bSuccess !=0 );
}


/*
	Get( &[string] ).

*/
bool	CStorageLua::Get( const std::string &_entry, std::string &_val )
{
	assert( m_pState != NULL );
	std::string s = "g_Settings." + _entry;
	std::string a = "?";

	int32	bSuccess = 0;
	char	*ret = NULL;
	int32	stackdelta = Base::Script::Call( m_pState->GetState(), "g_GetSetting", "ss>is", s.c_str(), "?", &bSuccess, &ret );

	if (bSuccess && ret != NULL)
		_val = std::string( ret );

	//	Clean up return string from stack.
	m_pState->Pop( stackdelta );

	return( bSuccess !=0 );
}

/*
	Remove().
	No use for this yet.
*/
bool	CStorageLua::Remove( const std::string &_url )
{
	assert( m_pState != NULL );

	//	This is never called, just added this log entry both to track usage, an to get rid of unused var warning...
	g_Log->Warning( "CStorageLua::Remove( %s )\n", _url.c_str() );

	Dirty( true );
	return( true );
}

/*
	Commit.

*/
bool	CStorageLua::Commit()
{
	assert( m_pState != NULL );

	if( Dirty() && !m_bReadOnly)
	{
		std::string cfgfile = std::string("/") + CLIENT_SETTINGS + ".cfg";

		g_Log->Info( "CLuaStorage::Commit()\n" );

		std::stringstream s; // write client version as string
		s << "g_Settings." << "settings.app.os_version" << " =  [[" << CLIENT_VERSION << "]]";
		m_pState->Execute( s.str().c_str() );
		
		time_t	curTime;
		time( &curTime );

		std::stringstream temp;
		std::string temptime = ctime( &curTime );
		temptime.erase(temptime.size() - 1);
		
		std::stringstream s2; // write client version as string
		s2 << "g_Settings." << "settings.app.date_time" << " =  [[" << temptime << "]]";
		m_pState->Execute( s2.str().c_str() );

		path tmpPath = m_sRoot;
		tmpPath /= cfgfile;
		std::string tmp = "table.save( g_Settings, [[" + tmpPath.string() + "]] )";
		m_pState->Execute( tmp );
		Dirty( false );

		g_Log->Info( "CLuaStorage::Commit() done\n" );
	}

	return( true );
}

//	Simple function to use the logger..
int CStorageLua::SettingsLogger( lua_State *_pState )
{
    assert( _pState !=NULL );
    const char *pString = luaL_checkstring( _pState, 1 );
    g_Log->Info( "%s", pString );
    return(0);
}

/*
*/
bool	CStorageLua::Initialise( const std::string &_sRoot, const std::string &_sWorkingDir, bool _bReadOnly )
{
	g_Log->Info( "CStorageLua::Initialize( %s, %s )\n", _sRoot.c_str(), _sWorkingDir.c_str() );

	m_bReadOnly = _bReadOnly;
    path tmpPath = _sRoot;
	m_sRoot = tmpPath.string();

#ifdef	WIN32
	//	For -some- reason, boost refuses to leave the trailing slash here, but does so in Linux...
	size_t len = m_sRoot.size();
	if( m_sRoot[ len - 1 ] != '\\' )
	{
		g_Log->Info( "Appending trailing slashes to m_sRoot..." );
		m_sRoot.append( "\\" );
	}
#endif


	//	Make sure directory is created.
	if( IStorageInterface::CreateFullDirectory( m_sRoot.c_str() ) == false )
	{
		g_Log->Error( "Unable to create structure %s...", m_sRoot.c_str() );
		return false;
	}

	m_pState = new Base::Script::CLuaState();

    tmpPath = _sWorkingDir + "Scripts";
	m_pState->Init( tmpPath.string().c_str() );

    tmpPath = _sWorkingDir + "Scripts/serialize.lua";
	if (m_pState->Run( tmpPath.string().c_str() ) == false)
		return false;

    //	Logging...
    lua_pushcfunction( m_pState->GetState(), CStorageLua::SettingsLogger );
    lua_setglobal( m_pState->GetState(), "g_Log" );

	tmpPath = m_sRoot + CLIENT_SETTINGS + ".cfg";
	m_pState->Execute( "require( 'table' ) g_Settings, err = table.load( [[" + tmpPath.string() + "]] ) if g_Settings == nil then g_Log( err ) g_Settings = AutoTable( {} ) end" );

	//	Store root.
	m_pState->Execute( "g_Root = [[" + m_sRoot + "]]" );

	static const char *getSettings =	"function g_GetSetting( _url, _default )\
											g_Log( 'g_GetSetting(' .. tostring(_url) .. ', ' .. tostring(_default) .. ')' )\
											local f = assert( loadstring( 'return ' .. _url ) )\
											if not f then\
												error( 'setting request failed to compile...' )\
												return 0, _default\
											end\
											local val = f()\
											if type(val) ~= type(_default) then\
												g_Log( 'returned type does not match, returning ' .. tostring(_default) )\
												return 0, _default\
											end\
											g_Log( 'g_GetSetting returned ' .. tostring(val) )\
											return 1, val\
										end";

	//	So we can use Base::Script::Call() instead of messing around with lua's C api...
	m_pState->Execute(	getSettings );

	Dirty( false );

	return( true );
}

/*
*/
bool	CStorageLua::Config( const std::string &_url )
{
#ifndef  WIN32
    (void)_url;
    return true;
#else

	//	Register IUP.
//	lua_cpcall( m_pState->GetState(), iuplua_open, NULL );
//	lua_cpcall( m_pState->GetState(), iupcontrolslua_open, NULL );

	//	Store client version.
	m_pState->Execute( std::string( "g_ClientVersion = '" ) + CLIENT_VERSION_PRETTY + "'" );
	m_pState->Execute( std::string( "g_HelpLink = 'http://electricsheep.org/client/" ) + CLIENT_VERSION + "'" );

	m_pState->Run( _url );

	//	Close IUP again.
//	IupControlsClose();
//	IupClose();

    return true;
#endif
}

/*
	Finalise().

*/
bool	CStorageLua::Finalise()
{
	g_Log->Info( "CStorageLua::Finalise()\n" );
	SAFE_DELETE( m_pState );
	return( true );
}

};
