#ifndef	_GRAPHPLAYLIST_H
#define _GRAPHPLAYLIST_H

#include "playlist.h"
#include "Log.h"
#include "Timer.h"
#include "Settings.h"
#include <queue>
#include <map>
#include <set>
#include <sstream>


#include	"boost/filesystem/path.hpp"
#include	"boost/filesystem/operations.hpp"
#include	"boost/filesystem/convenience.hpp"

using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::no_check;
using boost::filesystem::directory_iterator;
using boost::filesystem::extension;

namespace ContentDecoder
{

/*
	CNode().
	One node in the flock graph.
*/
class	CNode
{
	NO_CLASS_STANDARDS( CNode );

	int32	m_Generation;
	int32	m_ID;
	int32	m_First;
	int32	m_Last;

	std::string	m_Filename;
	std::string	m_StorageUrl;

	int32	m_PlayCount;

	public:
			CNode() : m_Generation(-1), m_ID(-1), m_First(-1), m_Last(-1), m_PlayCount(0)
			{}

			~CNode()
			{
				TupleStorage::spIStorageNode spNode = g_Settings().OpenNode( m_StorageUrl );
				spNode->Set( "PlayCount", m_PlayCount );

			}

			//	Deduce generation & graph location from filename.
			bool	Init( std::string &_filename )
			{
				//	Save what we're all about.
				m_Filename = _filename;

				//	Remove the full path so we can work on the filename.
				size_t offs = _filename.find_last_of( "/\\", _filename.size() );
				std::string sheep = _filename.substr( offs+1, _filename.size()-1 );

				//	Deduce graph position from filename.
				int ret = sscanf( sheep.c_str(), "%d=%d=%d=%d.avi", &m_Generation, &m_ID, &m_First, &m_Last );
				if( ret != 4 )
				{
					g_Log->Error( "Unable to deduce graph position from %s", sheep.c_str() );
					return false;
				}

				g_Log->Error( "Node: %d->%d->%d->%d", m_Generation, m_ID, m_First, m_Last );

				//	Create storage url.
				std::stringstream s;
				s << "\\settings\\flocks\\" << sheep << "\\";
				m_StorageUrl = s.str();

				//	Update storage.
				m_PlayCount = g_Settings().Get( m_StorageUrl, "PlayCount", 0 );

				return true;
			}

			const std::string	&FileName()	{	return( m_Filename );	};
			const int32	PlayCount()			{	return( m_PlayCount );	};
			void IncPlayCount()				{	m_PlayCount++;	};
};

MakeSmartPointers( CNode );

typedef std::multimap< std::string, spCNode > gnode_t;

/*
	CGraphPlaylist().
	Perodically polls a directory for video, and traverses based on filename, in a graph-like manner,
	while prioritizing loops & edges a´la Spot style.
*/
#warning TODO (Keffo#1#): Move this stuff to Lua, no need to bother with this in C/C++!
class	CGraphPlaylist : public CPlaylist
{
	//	The default content to show when nothing is available...
	std::string	m_Default;

	//	Path to folder to monitor & update interval in seconds.
	path			m_Path;
	fp8				m_Interval;
	fp8				m_Clock;

	Base::CTimer	m_Timer;

	gnode_t			m_Graph;

	void	UpdateDirectory( path const &_dir )
	{
		//g_Log->Info( "Monitoring in %s...", monitor_dir.string().c_str() );
		for( directory_iterator i( _dir ), end; i != end; ++i )
		{
			#warning TODO (Keffo#1#): Remove hardcoded extension...
			if( extension(*i) != ".avi" )
				continue;

			std::string file = i->string();

			//	Add to collection if it's not already there.
			gnode_t::iterator k = m_Graph.find( file );
			if( k == m_Graph.end() )
			{
				//	Create a new node.
				spCNode spNode = new CNode();
				if( spNode->Init( file ) )
					m_Graph.insert( std::make_pair( file, spNode ) );
			}
		}
	}

	public:
			CGraphPlaylist( const std::string &_watchFolder, const std::string &_default ) : CPlaylist()
			{
				m_Interval = 2;
				m_Clock = 0;

				boost::filesystem::path::default_name_check(boost::filesystem::native);

				//	Update now so other parts of the code doesn't freak when this returns zero length;
				m_Path = _watchFolder.c_str();
				m_Default = _default;

				g_Log->Info( "Starting graphing playlist in %s...", m_Path.native_directory_string().c_str() );
				UpdateDirectory( m_Path );
			}

			virtual ~CGraphPlaylist()
			{}

			//
			virtual bool	Add( const std::string &_file )
			{
				printf( "%s\n", _file.c_str() );
				return( true );
			}

			virtual uint32	Size()	{	return m_Graph.size();	}

			virtual bool	Next( std::string &_result )
			{
				try
				{
					//	Update from directory if enough time has passed.
					if( m_Timer.Time() > m_Clock + m_Interval )
					{
						UpdateDirectory( m_Path );
						m_Clock = m_Timer.Time();
					}

					//	Placeholder!
					_result = m_Default;
					return true;
				}
				catch( ... )
				{
					g_Log->Error( "eh?" );
					_result = m_Default;
				}

				return true;
			}
};

MakeSmartPointers( CGraphPlaylist );

}

#endif
