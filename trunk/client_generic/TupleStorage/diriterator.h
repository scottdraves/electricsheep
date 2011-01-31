#ifndef	_DIRITERATOR_H
#define _DIRITERATOR_H

namespace	TupleStorage
{

#ifdef WIN32

#define MAX_DIR_LENGTH 1023

typedef struct dir_data
{
	long	hFile;
	char	pattern[ MAX_DIR_LENGTH + 1 ];
}	dir_data;

#endif

/*
*/
class	CDirectoryIterator
{

#ifdef	WIN32
	dir_data	*m_pDirData;
#else
	DIR			*m_pDirData;
#endif

	std::string	m_Directory;

	public:
			CDirectoryIterator( const std::string &_path );
			~CDirectoryIterator();

			//
			const bool Next( std::string &_object );
			const bool isDirectory( const std::string &_object );

//			POOLED( CDirectoryIterator, Memory::CLinkPool );
};

};

#endif
