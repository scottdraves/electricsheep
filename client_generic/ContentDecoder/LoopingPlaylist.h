#ifndef	_LOOPINGPLAYLIST_H
#define _LOOPINGPLAYLIST_H

#include "Playlist.h"
#include <queue>

namespace	ContentDecoder
{

/*
	CLoopingPlaylist().
	Looping playlist based on an array.
*/
class	CLoopingPlaylist : public CPlaylist
{
	std::vector<std::string>    m_List;
	uint32	m_Index;

	public:
			CLoopingPlaylist() : CPlaylist()
			{
				m_Index = 0;
			}

			virtual ~CLoopingPlaylist()
			{}

			//
			virtual bool	Add( const std::string &_file )
			{
				m_List.push_back( _file );
				return( true );
			}

			virtual bool	Next( std::string &_result )
			{
				if( m_List.empty() )
					return false;

				printf( "%d\n", m_Index );
				_result = m_List[ m_Index ];

				m_Index++;
				if( m_Index >= m_List.capacity() )
					m_Index = 0;

				return true;
			}
};

MakeSmartPointers( CLoopingPlaylist );

}


#endif
