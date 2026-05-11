#ifndef	_LUAPLAYLIST_H
#define _LUAPLAYLIST_H

#include "Common.h"
#include "Playlist.h"
#include "Log.h"
#include "Timer.h"
#include "Settings.h"
#include "LuaState.h"
#include "luaxml.h"
#include "PlayCounter.h"
#include <sstream>
#include <sys/stat.h>
#include "Shepherd.h"
#include "isaac.h"
#include "ContentDownloader.h"

#include	"boost/filesystem/path.hpp"
#include	"boost/filesystem/operations.hpp"
#include	"boost/filesystem.hpp"
#include	<boost/thread.hpp>

using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::directory_iterator;


//	Lua.
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
};


namespace ContentDecoder
{

#define kSheepNumTreshold 20

/*
	CLuaPlaylist().
	Abstract handling for playlists implemented in lua.
*/
static bool randinitialized = false;
static randctx ISAAC_ctx;

class	CLuaPlaylist : public CPlaylist
{
	boost::mutex	m_Lock;
	
	boost::mutex	m_CurrentPlayingLock;

	//	Path to folder to monitor & update interval in seconds.
	path			m_Path;
	fp8				m_NormalInterval;
	fp8				m_EmptyInterval;
	fp8				m_Clock;

	Base::CTimer	m_Timer;
	
	uint32			m_numSheep;
	
	bool			m_AutoMedian;
	bool			m_RandomMedian;
	fp8				m_MedianLevel;
	uint64			m_FlockMBs;
	uint64			m_FlockGoldMBs;

	//	The lua state that will do all the work.
	Base::Script::CLuaState	*m_pState;
	
	//	Simple function to use the logger..
	static int playlistLogger( lua_State *_pState )
	{
		assert( _pState !=NULL );
		const char *pString = luaL_checkstring( _pState, 1 );
		
		if ( pString != NULL )
			g_Log->Info( pString );
			
		return(0);
	}
	
	static int errorMessage( lua_State *_pState )
	{
		assert( _pState !=NULL );
		const char *pString = luaL_checkstring( _pState, 1 );
		
		if ( pString != NULL )
			ContentDownloader::Shepherd::QueueMessage( pString, 10.0f );
		
		return(0);
	}
	
	static int incPlayCount( lua_State *_pState )
	{
		uint16 retval = 0;
		
		uint32 generation = static_cast<uint32>(luaL_checkint( _pState, 1 ));
		uint32 idx = static_cast<uint32>(luaL_checkint( _pState, 2 ));
				
		g_PlayCounter().IncPlayCount( generation, idx );
		
		retval = g_PlayCounter().PlayCount( generation, idx );
		
		lua_pushinteger( _pState, retval );
		
		return(1);
	}

	static int playCount( lua_State *_pState )
	{
		uint16 retval = 0;
		
		uint32 generation = static_cast<uint32>(luaL_checkint( _pState, 1 ));
		uint32 idx = static_cast<uint32>(luaL_checkint( _pState, 2 ));
		
		retval = g_PlayCounter().PlayCount( generation, idx );
		
		lua_pushinteger( _pState, retval );
		
		return(1);
	}
	
	static int getRandomSeed( lua_State *_pState )
	{
		lua_pushinteger( _pState, lua_Integer(time(NULL)));
		
		return(1);
	}

	static int getRand( lua_State *_pState )
	{
		if (randinitialized == false)
		{
			randinitialized = true;
			ub4 default_isaac_seed = static_cast<ub4>(time(0));
			for (size_t lp = 0; lp < RANDSIZ; lp++)
				ISAAC_ctx.randrsl[lp] = default_isaac_seed;
			irandinit(&ISAAC_ctx, true);
		}

		lua_pushnumber( _pState, LUA_NUMBER(double(irand(&ISAAC_ctx))/double(0xffffffff) ));
		
		return(1);
	}

	static int accessTime( lua_State *_pState )
	{
		const char *fpath = luaL_checkstring( _pState, 1 );
		
		time_t atime = 0;

		if ( fpath != NULL )
		{
			struct stat fs;

			if ( !stat(fpath, &fs) )
			{
				atime = fs.st_atime;
			}
		}

		lua_pushinteger( _pState, lua_Integer(atime) ); // fix after year 2038 :)
		
		return(1);
	}
	
	static int clearMedianSurvivorsStats( lua_State *_pState )
	{
		g_PlayCounter().clearMedianSurvivorsStats();
		lua_pushinteger( _pState, lua_Integer(0));
		return(1);
	}

	static int clearDeadEndSurvivorsStats( lua_State *_pState )
	{
		g_PlayCounter().clearDeadEndSurvivorsStats();
		lua_pushinteger( _pState, lua_Integer(0));
		return(1);
	}

	static int incMedianCutSurvivors( lua_State *_pState )
	{
		g_PlayCounter().IncMedianCutSurvivors();
		lua_pushinteger( _pState, lua_Integer(0));
		return(1);
	}

	static int incDeadEndCutSurvivors( lua_State *_pState )
	{
		g_PlayCounter().IncDeadEndCutSurvivors();
		lua_pushinteger( _pState, lua_Integer(0));
		return(1);
	}
	//
	void	DeduceGraphnessFromFilenameAndQueue( path const &/*_basedir*/, const std::string& _filename )
	{
		uint32 Generation, ID, First, Last;
		std::string sheep;
		
		if( !GetSheepInfoFromPath( _filename, Generation, ID, First, Last, sheep ) )
		{
			g_Log->Error( "Unable to deduce graph position from %s", sheep.c_str() );
			return;
		}

		path fullPath(_filename );
		std::string xxxname( _filename );
		xxxname.replace(_filename.size() - 3, 3, "xxx");
		
		if ( exists( xxxname ) )
		{
			remove( fullPath );
			return;
		}

		struct stat fs;
				
		time_t atime = 0;

		if ( !stat( fullPath.string().c_str(), &fs ) )
		{
			atime = fs.st_atime;
		}

		m_pState->Pop( Base::Script::Call( m_pState->GetState(), "Add", "ssiiiii", (fullPath.parent_path().string() + std::string("/")).c_str(), fullPath.filename().string().c_str(), Generation, ID, First, Last, atime ) );
		m_numSheep++;
	}

	void	AutoMedianLevel(uint64 megabytes)
	{
		if (megabytes < 100)
		{
			g_Log->Info( "Flock < 100 MBs AutoMedian = 1." );
			m_MedianLevel = 1.;
		} else
		if (megabytes > 1000)
		{
			g_Log->Info( "Flock > 1000 MBs AutoMedian = .25" );
			m_MedianLevel = .25;
		} else
		{
			m_MedianLevel = 13./12. - fp8(megabytes)/1200.;
			if (m_MedianLevel > 1.)
				m_MedianLevel = 1.;
			if (m_MedianLevel < .25)
				m_MedianLevel = .25;
			g_Log->Info( "Flock 100 - 1000 MBs AutoMedian = %f", m_MedianLevel );
		}
	}

	//
	void	UpdateDirectory( path const &_dir, const bool _bRebuild = false )
	{
		//boost::mutex::scoped_lock locker( m_Lock );

		m_numSheep = 0;
		
		std::vector<std::string>	files;

		int usedsheeptype = g_Settings()->Get( "settings.player.PlaybackMixingMode", 0 );

		if ( usedsheeptype == 0 )
		{
			if ( Base::GetFileList( files, _dir.string().c_str(), "avi", true, false ) == false )
				usedsheeptype = 2; // only gold, if any - revert to all if gold not found
		}

		if ( usedsheeptype == 1 ) // free sheep only
			Base::GetFileList( files, _dir.string().c_str(), "avi", false, true );

		if ( usedsheeptype > 1 ) // play all sheep, also handle case of error (2 is maximum allowed value)
			Base::GetFileList( files, _dir.string().c_str(), "avi", true, true );

		//	Clear the sheep context...
		if( _bRebuild )
		{
			if (m_AutoMedian)
				AutoMedianLevel( m_FlockMBs + m_FlockGoldMBs );
			m_pState->Pop( Base::Script::Call( m_pState->GetState(), "Clear", "d", m_MedianLevel ) );
			//m_pState->Execute( "Clear()" );
		}

		for( std::vector<std::string>::const_iterator i=files.begin(); i!=files.end(); ++i )
			DeduceGraphnessFromFilenameAndQueue( _dir, *i );

		//	Trigger update on the lua side of things.
		m_pState->Execute( "Rebuild()" );
	}

	public:
			CLuaPlaylist( const std::string &_scriptRoot, const std::string &_watchFolder, int &/*_usedsheeptype*/ ) : CPlaylist()/*, m_UsedSheepType(_usedsheeptype)*/
			{
				m_NormalInterval = fp8(g_Settings()->Get( "settings.player.NormalInterval", 100 ));
				m_EmptyInterval = 10.0f;
				m_Clock = 0.0f;
 				m_Path = _watchFolder.c_str();
				
				m_numSheep = 0;
				
				g_Log->Info( "Starting lua playlist (updates every %d seconds)...", m_NormalInterval );

				//	Start the luastate.
				m_pState = new Base::Script::CLuaState();
				m_pState->Init( _scriptRoot );

				//	Logging...
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::playlistLogger );
				lua_setglobal( m_pState->GetState(), "g_Log" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::incPlayCount );
				lua_setglobal(  m_pState->GetState(), "g_IncPlayCount" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::playCount );
				lua_setglobal(  m_pState->GetState(), "g_PlayCount" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::accessTime );
				lua_setglobal(  m_pState->GetState(), "g_AccessTime" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::getRand);
				lua_setglobal(  m_pState->GetState(), "g_CRand" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::getRandomSeed );
				lua_setglobal(  m_pState->GetState(), "g_CRandomSeed" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::errorMessage );
				lua_setglobal(  m_pState->GetState(), "g_ErrorMessage" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::clearMedianSurvivorsStats );
				lua_setglobal(  m_pState->GetState(), "g_ClearMedianSurvivorsStats" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::clearDeadEndSurvivorsStats );
				lua_setglobal(  m_pState->GetState(), "g_ClearDeadEndSurvivorsStats" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::incMedianCutSurvivors );
				lua_setglobal(  m_pState->GetState(), "g_IncMedianCutSurvivors" );
				lua_pushcfunction( m_pState->GetState(), CLuaPlaylist::incDeadEndCutSurvivors );
				lua_setglobal(  m_pState->GetState(), "g_IncDeadEndCutSurvivors" );
				//m_pState->registerLib( "luaXML", luaopen_xml );


				//	Run the code that will populate this luastate with code.
				m_pState->Execute( "require 'playlist'" );

				int32	loopIterations = g_Settings()->Get( "settings.player.LoopIterations", 2 );
				bool	seamlessPlayback = g_Settings()->Get( "settings.player.SeamlessPlayback", false );
				fp8		playEvenly = (fp8) g_Settings()->Get( "settings.player.PlayEvenly", 100 ) / 100.0;
				m_MedianLevel = (fp8) g_Settings()->Get( "settings.player.MedianLevel", 80 ) / 100.0;
				m_AutoMedian = g_Settings()->Get( "settings.player.AutoMedianLevel", true );
				m_RandomMedian = g_Settings()->Get( "settings.player.RandomMedianLevel", true );
				if (m_AutoMedian)
				{
					// HACK to get flock size before full initialization
					ContentDownloader::Shepherd::setRootPath( g_Settings()->Get( "settings.content.sheepdir", g_Settings()->Root() + "content" ).c_str() );
					m_FlockMBs = ContentDownloader::Shepherd::GetFlockSizeMBsRecount(0);
					m_FlockGoldMBs = ContentDownloader::Shepherd::GetFlockSizeMBsRecount(1);
					AutoMedianLevel( m_FlockMBs );
				}

				m_pState->Pop( Base::Script::Call( m_pState->GetState(), "Init", "sibddbb", m_Path.string().c_str(), loopIterations, seamlessPlayback, playEvenly, m_MedianLevel, m_AutoMedian, m_RandomMedian) );
				
				UpdateDirectory( m_Path );
			}

			//
			virtual ~CLuaPlaylist()
			{				
				SAFE_DELETE( m_pState );
			}

			//
			virtual bool	Add( const std::string &_file )
			{
				boost::mutex::scoped_lock locker( m_Lock );
				m_pState->Pop( Base::Script::Call( m_pState->GetState(), "Add", "s", _file.c_str() ) );
				return( true );
			}

			virtual uint32	Size()
			{
				boost::mutex::scoped_lock locker( m_Lock );
				int32	ret = 0;
				m_pState->Pop( Base::Script::Call( m_pState->GetState(), "Size", ">i", &ret ) );
				return (uint32)ret;
			}

			virtual bool	Next( std::string &_result, bool &_bEnoughSheep, uint32 _curID, const bool _bRebuild = false, bool _bStartByRandom = true )
			{
				boost::mutex::scoped_lock locker( m_Lock );
								
				fp8 interval = ( m_numSheep >  kSheepNumTreshold ) ? m_NormalInterval : m_EmptyInterval;

				//	Update from directory if enough time has passed, or we're asked to.
				if( _bRebuild || ((m_Timer.Time() - m_Clock) > interval) )
				{
					if (g_PlayCounter().ReadOnlyPlayCounts())
					{
						g_PlayCounter().ClosePlayCounts();
						m_FlockMBs = ContentDownloader::Shepherd::GetFlockSizeMBsRecount(0);
						m_FlockGoldMBs = ContentDownloader::Shepherd::GetFlockSizeMBsRecount(1);
					}
					UpdateDirectory( m_Path, _bRebuild );
					m_Clock = m_Timer.Time();
				}

				_bEnoughSheep = ( m_numSheep > kSheepNumTreshold );
				
				//	Gently ask lua about a new file.
				int8	*ret = NULL;
				int32	stackdelta = Base::Script::Call( m_pState->GetState(), "Next", "ib>s", _curID, _bStartByRandom, &ret );
				
				if( stackdelta == 1 && ret != NULL)
					if ( *(const char *)ret )
						_result = std::string( (const char *)ret );
					else
					{
						m_pState->Pop( stackdelta );
						return false;
					}
				else
				{
					g_Log->Warning( "Playlist behaved weird" );
				}

				//	Clean up return string from stack.
				m_pState->Pop( stackdelta );
				
				return true;
			}
			
			virtual bool ChooseSheepForPlaying(uint32 curGen, uint32 curID)
			{
				g_PlayCounter().IncPlayCount(curGen, curID);
				
				return true;
			}

			//	Overrides the playlist to play _id next time.
			void	Override( const uint32 _id )
			{
				boost::mutex::scoped_lock locker( m_Lock );
				m_pState->Pop( Base::Script::Call( m_pState->GetState(), "Override", "i", _id ) );
			}

			//	Queues _id to be deleted.
			void	Delete( const uint32 _id )
			{
				boost::mutex::scoped_lock locker( m_Lock );
				m_pState->Pop( Base::Script::Call( m_pState->GetState(), "Delete", "i", _id ) );
			}
};

MakeSmartPointers( CLuaPlaylist );

}

#endif
