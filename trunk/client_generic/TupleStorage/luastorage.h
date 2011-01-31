#ifndef	_REGSTORAGE_H
#define _REGSTORAGE_H

#include	"storage.h"
#include	"LuaState.h"

namespace	TupleStorage
{

/*
	CStorageLua.

*/
class	CStorageLua : public IStorageInterface
{
	//	The lua state that will do all the work.
	Base::Script::CLuaState	*m_pState;
	bool	m_bReadOnly;

    static int SettingsLogger( lua_State *_pState );

	public:
			CStorageLua()	{};
			virtual ~CStorageLua()	{};

			//
			bool	Initialise( const std::string &_sRoot, const std::string &_sWorkingDir, bool _bReadOnly = false );
			bool	Finalise();

			//
			bool	Set( const std::string &_entry, const bool _val );
			bool	Set( const std::string &_entry, const int32 _val );
			bool	Set( const std::string &_entry, const fp8 _val );
			bool	Set( const std::string &_entry, const std::string &_str );

			//
			bool	Get( const std::string &_entry, bool &_ret );
			bool	Get( const std::string &_entry, int32 &_ret );
			bool	Get( const std::string &_entry, fp8 &_ret );
			bool	Get( const std::string &_entry, std::string &_ret );

			//
			bool	Remove( const std::string &_entry );

			//
			bool	Commit();

			bool	Config( const std::string &_url );
};

MakeSmartPointers( CStorageLua );

};

#endif
