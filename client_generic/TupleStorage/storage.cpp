#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/stat.h>
	#include <errno.h>
	#include <stddef.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <dirent.h>
#endif

#include	<vector>
#include	<string>

#include	"Log.h"
#include	"storage.h"
#include	"diriterator.h"

using namespace std;

namespace	TupleStorage
{

/*
	CreateDir().

*/
bool IStorageInterface::CreateDir( const std::string &_sPath )
{
#ifdef	WIN32
	if( CreateDirectoryA( _sPath.c_str(), NULL ) != 0 )
	{
		const DWORD result=GetLastError();
		if( result == ERROR_ALREADY_EXISTS )
			return( true );	//	This error is ok.
		else
			return( false );
	}
#else
	if( mkdir( _sPath.c_str(), (S_IRWXU | S_IRWXG | S_IRWXO ) ) != 0 )
	{
		if( errno == EEXIST )
			return( true );	//	This error is ok.
	}
#endif

	return( true );
}


/*
	RemoveDir().

*/
bool IStorageInterface::RemoveDir( const std::string &_sPath )
{
#ifdef	WIN32
	BOOL result = RemoveDirectoryA( _sPath.c_str() );
	if( !result )
	{
		g_Log->Error( "%d...", GetLastError() );
		return( false );
	}

#else
	if( rmdir( _sPath.c_str() ) != 0 )
	{
		g_Log->Error( "%d...", errno );
		return( false );
	}
#endif

	return( true );
}

/*
	CreateFullDirectory().

*/
bool IStorageInterface::CreateFullDirectory( const std::string &_sPath )
{
	//	Will be false once we're out of the "../../"'s, then we will actually create.
	bool dotdot = true;

	for( unsigned int C=0; C<_sPath.size(); C++ )
	{
		if( _sPath[C] == '/' || _sPath[C] == '\\' )
		{
			if( dotdot == false )
			{
				std::string subPath = _sPath.substr( 0, C );
				if( !CreateDir( subPath ) )
				{
					return( false );
				}
				//	Path created.
			}
		}
		else
		{
			if( _sPath[C] != '.' )
				dotdot = false;
		}
	}

	return true;
}

/*
	DirectoryEmpty().

*/
bool	IStorageInterface::DirectoryEmpty( const std::string &_sPath )
{
	CDirectoryIterator	iterator( _sPath );

	std::string tmp;

	for( ;; )
	{
		if( !iterator.Next( tmp ) )
			return( false );

		if( tmp != "." && tmp != ".." )
		{
			if( iterator.isDirectory( tmp ) )
				return( true );
		}
	}

	return( false );
}


/*
	RemoveFullDirectory().

*/
bool IStorageInterface::RemoveFullDirectory( const std::string &_sPath, const bool _bSubdirectories )
{
	std::string	path = _sPath;

	size_t len = path.size();
	if( path[ len - 1 ] != '/' )
		path.append( "/" );

	CDirectoryIterator	*pIterator = new CDirectoryIterator( path );

	std::vector<std::string> dirlist;
	std::vector<std::string> filelist;

	std::string tmp;

	for( ;; )
	{
		if( !pIterator->Next( tmp ) )
			break;

		if( tmp != "." && tmp != ".." )
		{
			std::string	fullPath = path + tmp;

			if( pIterator->isDirectory( tmp ) && _bSubdirectories == true )
				dirlist.push_back( fullPath );
			else
				filelist.push_back( fullPath.c_str() );
		}
	}

	delete pIterator;

	std::vector<std::string>::iterator i;
	for( i=filelist.begin(); i != filelist.end(); i++ )
		remove( i->c_str() );

	filelist.clear();

	for( i=dirlist.begin(); i != dirlist.end(); i++ )
		RemoveFullDirectory( i->c_str(), _bSubdirectories );

	dirlist.clear();

	//	Remove ourselves.
	RemoveDir( path );

	return( true );
}


/*
	IoHierarchyHelper().

*/
bool IStorageInterface::IoHierarchyHelper( const std::string &_uniformPath, std::string &_retPath, std::string &_retName )
{
	std::string path = _uniformPath;

	//	Convert '.' to '/'
	for( size_t c=0; c<path.size(); c++ )
	{
		if( path[c] == '.' )
			path[c] = '/';
	}

	//	Make sure there is a triling "/"
	size_t len = path.size();
	if( path[ len - 1 ] != '/' )
		path.append( "/" );

	//	Find actual nodename.
	std::string temp = path.substr( 0, path.size()-1 );
	size_t offs = temp.find_last_of( "/\\", temp.size() );

	//	Todo: handle any errors here...

	_retPath = path;
	_retName = temp.substr( offs+1, temp.size() );

	return( true );
}

};
