#ifndef	_STORAGE_H
#define _STORAGE_H

#include	"base.h"
#include	"SmartPtr.h"
#include	<string>

namespace	TupleStorage
{

/*
	IStorageInterface.

*/
class IStorageInterface
{
	private:
		bool	m_bDirty;

	protected:
		std::string		m_sRoot;

		void Dirty( bool _state )	{	m_bDirty = _state;	};
		bool Dirty()				{	return m_bDirty;	};

	public:
			IStorageInterface() : m_bDirty( false ), m_sRoot( "./.Reg" )	{};
			virtual ~IStorageInterface() {};

			std::string	Root()	{	return( m_sRoot );	};

			//
			virtual	bool	Initialise( const std::string &_sRoot, const std::string &_sWorkingDir, bool _bReadOnly = false) = PureVirtual;
			virtual bool	Finalise() = PureVirtual;

			//	Set values.
			virtual bool	Set( const std::string &_entry, const bool _val ) = PureVirtual;
			virtual bool	Set( const std::string &_entry, const int32 _val ) = PureVirtual;
			virtual bool	Set( const std::string &_entry, const fp8 _val ) = PureVirtual;
			virtual bool	Set( const std::string &_entry, const std::string &_str ) = PureVirtual;

			//	Get values.
			virtual bool	Get( const std::string &_entry, bool &_ret ) = PureVirtual;
			virtual bool	Get( const std::string &_entry, int32 &_ret ) = PureVirtual;
			virtual bool	Get( const std::string &_entry, fp8 &_ret ) = PureVirtual;
			virtual bool	Get( const std::string &_entry, std::string &_ret ) = PureVirtual;

			//	Remove node from storage.
			virtual	bool	Remove( const std::string &_entry ) = PureVirtual;

			//	Persist changes.
			virtual bool 	Commit() = PureVirtual;

			//	Helpers.
			static bool	IoHierarchyHelper( const std::string &_uniformPath, std::string &_retPath, std::string &_retName );
			static bool	CreateDir( const std::string &_sPath );
			static bool	RemoveDir( const std::string &_sPath );
			static bool	CreateFullDirectory( const std::string &_sPath );
			static bool	RemoveFullDirectory( const std::string &_sPath, const bool _bSubdirectories = false );
			static bool	DirectoryEmpty( const std::string &_sPath );

			//	Config.
			virtual bool	Config( const std::string &_url ) = PureVirtual;
};

MakeSmartPointers( IStorageInterface );

};

#endif
