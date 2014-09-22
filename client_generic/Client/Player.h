#ifndef	_PLAYER_H_
#define	_PLAYER_H_

#ifdef WIN32
#include	<d3d9.h>
#include	<d3dx9.h>
#endif
#include	"DisplayOutput.h"
#include	"Renderer.h"
#include	"Singleton.h"
#include	"ContentDecoder.h"
#include	"lua_playlist.h"
#include	"Timer.h"
#include	"FrameDisplay.h"
#include	"Timer.h"

/**
	CPlayer.
	Singleton class to display decoded video to display.
*/
class	CPlayer : public Base::CSingleton<CPlayer>
{
public:
	typedef enum
	{
		kMDSharedMode,
		kMDIndividualMode,
		kMDSingleScreen
	} MultiDisplayMode;
	
private:
	typedef struct
	{
		DisplayOutput::spCDisplayOutput		spDisplay;
		DisplayOutput::spCRenderer			spRenderer;
		ContentDecoder::spCContentDecoder	spDecoder;
		spCFrameDisplay						spFrameDisplay;
		ContentDecoder::sMetaData			m_MetaData; // current frame meta data
	} DisplayUnit;
	
	typedef std::vector<DisplayUnit*>		DisplayUnitList;
	typedef std::vector<DisplayUnit*>::iterator DisplayUnitIterator;
	
	boost::mutex m_displayListMutex;
	
	friend class Base::CSingleton<CPlayer>;

	//	Private constructor accessible only to CSingleton.
	CPlayer();

	//	No copy constructor or assignment operator.
	NO_CLASS_STANDARDS( CPlayer );

	//	Videodecoder & framedisplay object.
	ContentDecoder::spCContentDecoder		m_spDecoder;
	
	DisplayUnitList							m_displayUnits;
	
	//	Playlist.
	ContentDecoder::spCLuaPlaylist			m_spPlaylist;


	//	Timer.
	Base::CTimer	m_Timer;

	//	Goal decoding framerate.
	fp8			m_PlayerFps;

	//	Goal display framerate;
	fp8			m_DisplayFps;

	//	Fullscreen or not.
	bool			m_bFullscreen;

	bool			m_InitPlayCounts;
	
	MultiDisplayMode m_MultiDisplayMode;
	
	bool			m_bStarted;

	//	Used to keep track of elapsed time since last frame.
	fp8	m_CapClock;

	bool m_HasGoldSheep;
	int m_UsedSheepType;
	
	boost::mutex	m_updateMutex;

#ifdef	WIN32
	HWND	m_hWnd;

	public:
			//	When running as a screensaver, we need to pass this along, as it's already created for us.
			void	SetHWND( HWND _hWnd );
			HWND	GetHWND( void )	{	return m_hWnd;	}
	private:
#endif
	
	ContentDecoder::CContentDecoder *CreateContentDecoder( bool _bStartByRandom = false );
	
	void FpsCap( const fp8 _cap );

	public:
			bool	Startup();
			bool	Shutdown( void );
			virtual ~CPlayer();

			const char *Description()	{	return( "Player" );	};

			bool  Closed( void )
			{
			    DisplayOutput::spCDisplayOutput spDisplay = Display();
				
				if( spDisplay == NULL )
			    {
			    	g_Log->Warning( "m_spDisplay is NULL" );
                    return true;
			    }

                return spDisplay->Closed();
			}

			bool	BeginFrameUpdate();
			bool	EndFrameUpdate();
			bool	BeginDisplayFrame( uint32 displayUnit );
			bool	EndDisplayFrame( uint32 displayUnit, bool drawn = true );
			bool	Update(uint32 displayUnit, bool &bPlayNoSheepIntro);
			void	Start();
			void	Stop();
			
#ifdef MAC
			bool	AddDisplay( CGLContextObj _glContext );
#else
#ifdef WIN32
			bool	AddDisplay( uint32 screen, IDirect3D9 *_pIDirect3D9 = NULL, bool _blank = false );
#else
			bool	AddDisplay( uint32 screen );
#endif
#endif

			inline void		PlayCountsInitOff()					{	m_InitPlayCounts = false; };
			inline void		Framerate( const fp8 _fps )			{	m_PlayerFps = _fps;	};
			inline void		Fullscreen( const bool _bState )	{	m_bFullscreen = _bState; };
			inline bool		Stopped()							{	return !m_bStarted;	};
			
			inline DisplayOutput::spCDisplayOutput	Display()	
			{ 	
				boost::mutex::scoped_lock lockthis( m_displayListMutex );

				return m_displayUnits.empty() ? NULL :  m_displayUnits[0]->spDisplay;
			}
			
			inline DisplayOutput::spCRenderer		Renderer()
			{
				boost::mutex::scoped_lock lockthis( m_displayListMutex );
				
				return m_displayUnits.empty() ? NULL :  m_displayUnits[0]->spRenderer;	
			}
			
			inline ContentDecoder::spCContentDecoder Decoder()
			{				
				boost::mutex::scoped_lock lockthis( m_displayListMutex );
				
				if ( m_MultiDisplayMode == kMDSharedMode )
					return m_spDecoder;
				else
					return m_displayUnits.empty() ? NULL :  m_displayUnits[0]->spDecoder;
			}

			//	Playlist stuff.
			inline std::string GetCurrentPlayingSheepFile()
			{
				return m_displayUnits[0]->m_MetaData.m_FileName;
			}
			inline uint32	GetCurrentPlayingSheepID()
			{	
				return m_displayUnits[0]->m_MetaData.m_SheepID;
			};
			inline uint32	GetCurrentPlayingSheepGeneration()
			{	
				return m_displayUnits[0]->m_MetaData.m_SheepGeneration;
			};
			inline time_t	GetCurrentPlayingatime()
			{	
				return m_displayUnits[0]->m_MetaData.m_LastAccessTime;
			};
			inline time_t	IsCurrentPlayingEdge()
			{	
				return m_displayUnits[0]->m_MetaData.m_IsEdge;
			};
			inline uint32	GetCurrentPlayingID()
			{	
				ContentDecoder::spCContentDecoder decoder = Decoder();
				
				if ( decoder.IsNull() )
					return 0;
					
				return decoder->GetCurrentPlayingID();	
			};
			
			inline uint32	GetCurrentPlayingGeneration()
			{	
				ContentDecoder::spCContentDecoder decoder = Decoder();
				
				if ( decoder.IsNull() )
					return 0;

				return decoder->GetCurrentPlayingGeneration();
			};
			
			inline void		Delete( const uint32 _id )			{	if ( !m_spPlaylist.IsNull() ) m_spPlaylist->Delete( _id );	};
			inline void		SkipToNext( void )
			{
				ContentDecoder::spCContentDecoder decoder = Decoder();
				
				if ( decoder.IsNull() )
					return;

				decoder->ForceNext();
			}
			
			inline void		ReturnToPrevious( void )
			{
				ContentDecoder::spCContentDecoder decoder = Decoder();
				
				if ( decoder.IsNull() )
					return;

				decoder->ForcePrevious( 2 );
			}
			
			inline void		RepeatSheep( void )
			{
				ContentDecoder::spCContentDecoder decoder = Decoder();
				
				if ( decoder.IsNull() )
					return;

				decoder->ForcePrevious( 1 );
			}
			
			inline void		SetMultiDisplayMode( MultiDisplayMode mode )	{ m_MultiDisplayMode = mode; }
			inline bool		HasGoldSheep() { return m_HasGoldSheep; }
			inline int		UsedSheepType() { return m_UsedSheepType; }
			
			inline uint32		GetDisplayCount() { return static_cast<uint32>(m_displayUnits.size()); }
};

/*
	Helper for less typing...

*/
inline CPlayer &g_Player( void )	{	return( CPlayer::Instance() );	}

#endif
