#ifndef	_PLAYLIST_H
#define _PLAYLIST_H

#include <string>
#include "base.h"
#include "SmartPtr.h"

namespace	ContentDecoder
{

/*
	CPlaylist().
	Playlist interface.
*/
class	CPlaylist
{
	public:
			CPlaylist()	{}
			virtual ~CPlaylist()	{}
			virtual uint32	Size() = PureVirtual;
			virtual bool	Add( const std::string &_file ) = PureVirtual;
			virtual bool	Next( std::string &_result, bool& _bEnoughSheep, uint32 _curID, const bool _bRebuild = false, bool _bStartByRandom = true ) = PureVirtual;
			virtual bool	ChooseSheepForPlaying( uint32 curGen, uint32 curID ) = PureVirtual;
			
			virtual bool GetSheepInfoFromPath( const std::string& _path, uint32& Generation, uint32& ID, uint32& First, uint32& Last, std::string& _filename )
			{
				//	Remove the full path so we can work on the filename.
				size_t offs = _path.find_last_of( "/\\", _path.size() );
				
				_filename = _path.substr(offs+1);

				//	Deduce graph position from filename.
				int ret = sscanf( _filename.c_str(), "%d=%d=%d=%d.avi", &Generation, &ID, &First, &Last );
				if( ret != 4 )
				{
					g_Log->Error( "Unable to deduce sheep info from %s", _path.c_str() );
					return false;
				}
				
				return true;
			}

};

MakeSmartPointers( CPlaylist );

}


#endif
