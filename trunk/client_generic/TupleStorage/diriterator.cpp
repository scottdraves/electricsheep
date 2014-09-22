#include <sys/stat.h>
#include	<string>
#include	<stdlib.h>

#ifdef WIN32
#include	<direct.h>
#include	<io.h>
#include	<sys/locking.h>
#include	<sys/utime.h>
#else
#include	<unistd.h>
#include	<dirent.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<utime.h>
#include	<dirent.h>
#endif


#include	"diriterator.h"

using namespace std;

namespace	TupleStorage
{

/*
	CDirectoryIterator().

*/
CDirectoryIterator::CDirectoryIterator( const std::string &_path )
{
#ifdef WIN32
	m_pDirData = (dir_data *)malloc( sizeof( dir_data ) );
	m_pDirData->hFile = 0L;
	if( _path.size() <= MAX_DIR_LENGTH )
		sprintf( m_pDirData->pattern, "%s/*", _path.c_str() );
#else
	m_pDirData = opendir( _path.c_str() );
#endif

	m_Directory = _path;
}

/*
	~CDirectoryIterator().

*/
CDirectoryIterator::~CDirectoryIterator()
{
#ifdef	WIN32
	if( m_pDirData->hFile )
	{
		_findclose( m_pDirData->hFile );
		free( m_pDirData );
	}
#else
	if( m_pDirData )
		closedir( m_pDirData );
#endif

	m_pDirData = NULL;
}

#ifdef WIN32
	#ifndef	S_ISDIR
		#define	S_ISDIR( _mode )  ( (_mode) & _S_IFDIR )
	#endif
#endif

/*
	isDirectory().

*/
bool CDirectoryIterator::isDirectory( const std::string &_object )
{
	struct stat info;

	std::string	url = m_Directory + _object;
	if( _object == "." || _object == ".." )
		return( false );

	if( stat( url.c_str(), &info ) )
		return( false );

	if( S_ISDIR( info.st_mode ) )
		return( true );

	return( false );
}

/*
	Next().

*/
bool CDirectoryIterator::Next( std::string &_object )
{
#ifdef WIN32
	struct _finddata_t c_file;
	if( m_pDirData->hFile == 0L )
	{
		//	First entry.
		if( (m_pDirData->hFile = (long)_findfirst( m_pDirData->pattern, &c_file ) ) == -1L )
		{
			return( false );
		}
		else
		{
			_object = c_file.name;
			return( true );
		}
	}
	else
	{	//	Next entry.
		if( _findnext( m_pDirData->hFile, &c_file ) == -1L )
			return( false );
		else
		{
			_object = c_file.name;
			return( true );
		}
	}
#else
	struct dirent *entry;
	if( ( entry = readdir( m_pDirData )) != NULL )
	{
		_object = entry->d_name;
		return( true );
	}
	else
		return( false );
#endif
}

};
