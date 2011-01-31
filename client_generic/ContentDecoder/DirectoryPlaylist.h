#ifndef DIRECTIRYPLAYLIST_H_INCLUDED
#define DIRECTIRYPLAYLIST_H_INCLUDED

#include <set>
#include "playlist.h"
#include <boost/filesystem/operations.hpp>

using boost::filesystem::directory_iterator;

/**
	CDirectoryPlaylist.
	Scans a directory for avi files and loops them.
*/
class	CDirectoryPlaylist : public ContentDecoder::CLoopingPlaylist
{
	public:
			CDirectoryPlaylist( const std::string &_directory ) : ContentDecoder::CLoopingPlaylist()
			{
				std::set<std::string> valid;

				for( directory_iterator i(_directory), end; i != end; ++i )
				{
					std::string file = i->string();
					size_t pos = file.rfind('.');
					if( pos != std::string::npos )
					{
						const std::string ext  = file.substr( pos+1 );
						if( ext == "avi" )
							Add( file );
					}
				}
			}
};

MakeSmartPointers( CDirectoryPlaylist );

#endif
/*#ifndef DIRECTIRYPLAYLIST_H_INCLUDED
#define DIRECTIRYPLAYLIST_H_INCLUDED

#include <set>
#include "playlist.h"
#include <boost/filesystem/operations.hpp>*/


/*
	CDirectoryPlaylist.
	Scans a directory for avi files and loops them.
*/
/*class	CDirectoryPlaylist : ContentDecoder::CLoopingPlaylist
{
	std::string	m_Directory;

	public:
			CDirectoryPlaylist( std::string _directory ) : ContentDecoder::CLoopingPlaylist(), m_Directory( _directory )
			{
				std::set<std::string> valid;

				for( directory_iterator i(m_Directory), end; i != end; ++i )
				{
					if( extension( *i ) != ".avi" )
						continue;

					std::string file = i->string();

					Add( file );
				}
			}
};

MakeSmartPointers( CDirectoryPlaylist );

#endif*/
