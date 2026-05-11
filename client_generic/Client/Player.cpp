#ifdef	WIN32
#include <windows.h>
#endif

#if defined(WIN32) && !defined(_MSC_VER)
#include	<dirent.h>
#endif
#include	<math.h>
#include	<string.h>

#ifndef WIN32
#ifndef LINUX_GNU
#include	"GLee.h"
#else
#include <GLee.h>
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __LITTLE_ENDIAN__ __LITTLE_ENDIAN
#undef __BIG_ENDIAN__
#else
#undef __LITTLE_ENDIAN__
#define __BIG_ENDIAN__ __BIG_ENDIAN
#endif
#endif
#ifdef MAC
#include	<GLUT/glut.h>
#else
#include	<GL/glut.h>
#endif
#endif

#include	"base.h"
#include	"MathBase.h"
#include	"clientversion.h"
#include	"Log.h"
#include	"Player.h"

#ifdef	WIN32
#include	"DisplayDX.h"
#include	"RendererDX.h"
#if defined(WIN32) && defined(_MSC_VER) && !defined(_WIN64)
#include	"../msvc/DisplayDD.h"
#include	"../msvc/RendererDD.h"
#endif
#else
#include	"DisplayGL.h"
#include	"RendererGL.h"
#endif


#include	"lua_playlist.h"
#include	"Settings.h"
#include	"ContentDownloader.h"
#include	"PlayCounter.h"
#include	"storage.h"

#include	"FrameDisplay.h"
#include	"LinearFrameDisplay.h"
#include	"CubicFrameDisplay.h"

#include	"boost/filesystem/path.hpp"
#include	"boost/filesystem/operations.hpp"
#include	"boost/filesystem.hpp"

#if defined(MAC) || defined(WIN32)
	#define HONOR_VBL_SYNC
#endif

using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::directory_iterator;

using namespace DisplayOutput;


/*
*/
CPlayer::CPlayer()
{
	m_spDecoder = NULL;
	m_spPlaylist = NULL;
	
	m_PlayerFps = 15;	//	http://en.wikipedia.org/wiki/23_(numerology)
	m_DisplayFps = 60;

	m_bFullscreen = true;
	m_InitPlayCounts = true;
	
	m_MultiDisplayMode = kMDSharedMode;
	
	m_bStarted = false;
	
	m_CapClock = 0.0;

#ifdef	WIN32
	m_hWnd = NULL;
#endif
}

/*
	SetHWND().

*/
#ifdef	WIN32
void	CPlayer::SetHWND( HWND _hWnd )
{
	g_Log->Info( "Got hwnd... (0x%x)", _hWnd );
	m_hWnd = _hWnd;
};
#endif

#ifdef MAC
bool CPlayer::AddDisplay( CGLContextObj _glContext )
#else
#ifdef WIN32
bool CPlayer::AddDisplay( uint32 screen, IDirect3D9 *_pIDirect3D9, bool _blank)
#else
bool CPlayer::AddDisplay( uint32 screen )
#endif
#endif
{
	DisplayOutput::spCDisplayOutput		spDisplay;
	DisplayOutput::spCRenderer			spRenderer;
	ContentDecoder::spCContentDecoder	spDecoder;
	spCFrameDisplay						spFrameDisplay;

	static bool detectgold = true;
	if (detectgold)
	{
		detectgold = false;
		std::string content = g_Settings()->Root() + "content/";
		std::string watchFolder = g_Settings()->Get( "settings.content.sheepdir", content ) + "/mpeg/";

		std::vector<std::string> files;
		m_HasGoldSheep = Base::GetFileList( files,  watchFolder, "avi", true, false );
	}
	// modify aspect ratio and/or window size hint
	uint32	w = 1280;
	uint32 h = 720;
	m_UsedSheepType = g_Settings()->Get( "settings.player.PlaybackMixingMode", 0 );
	switch ( m_UsedSheepType )
	{
	case 0: // only gold, if any
		{
			if (m_HasGoldSheep == false)
			{
				w = 800;
				h = 592;
				m_UsedSheepType = 2;
			}
		}
		break;
	case 1: // free sheep only
		w = 800;
		h = 592;
		break;
	case 2: // all sheep
		break;
	};



#ifdef	WIN32
#ifndef _WIN64
	bool bDirectDraw = g_Settings()->Get( "settings.player.directdraw", false );
	CDisplayDD *pDisplayDD = NULL;
#endif
	CDisplayDX *pDisplayDX = NULL;
#ifndef _WIN64
	if (bDirectDraw)
	{
		g_Log->Info( "Attempting to open %s...", CDisplayDD::Description() );
		pDisplayDD = new CDisplayDD();
	}
	else
#endif
	{
		g_Log->Info( "Attempting to open %s...", CDisplayDX::Description() );
		pDisplayDX = new CDisplayDX( _blank, _pIDirect3D9 );
		pDisplayDX->SetScreen( screen );
	}
	
	
	if( pDisplayDX == NULL 
#ifndef _WIN64
		&& pDisplayDD == NULL
#endif
		)
	{
		g_Log->Error( "Unable to open display" );
		return false;
	}
#ifndef _WIN64
	if (bDirectDraw)
		spDisplay = pDisplayDD;
	else
#endif
		spDisplay = pDisplayDX;
	if( m_hWnd )
	{
		if( !spDisplay->Initialize( m_hWnd, true ) )
			return false;
	}
	else
		if ( !spDisplay->Initialize( w, h, m_bFullscreen ) )
			return false;
#ifndef _WIN64
	if (bDirectDraw)
		spRenderer = new CRendererDD();
	else
#endif
		spRenderer = new CRendererDX();
#else // !WIN32

	g_Log->Info( "Attempting to open %s...", CDisplayGL::Description() );
	spDisplay = new CDisplayGL();
	if( spDisplay == NULL )
		return false;
	
#ifdef MAC
	if (_glContext != NULL)
	{
		if( !spDisplay->Initialize( _glContext, true ) )
			return false;
			
		spDisplay->ForceWidthAndHeight(w, h);
	}
#else
 	if( !spDisplay->Initialize( w, h, m_bFullscreen ) )
		return false;
#endif
	
 	spRenderer = new CRendererGL();
#endif

	//	Start renderer & set window title.
	if (spRenderer->Initialize( spDisplay ) == false)
		return false;
	spDisplay->Title( "Electric Sheep" );

	//	Create frame display.
	int32 displayMode = g_Settings()->Get( "settings.player.DisplayMode", 2 );
	if( displayMode == 2 )
	{
		if( spDisplay->HasShaders() )
		{
			g_Log->Info( "Using piecewise cubic video display..." );
			spFrameDisplay = new CCubicFrameDisplay( spRenderer );
		}
	}
	else
	{
		if( displayMode == 1 )
		{
			if( spDisplay->HasShaders() )
			{
				g_Log->Info( "Using piecewise linear video display..." );
				spFrameDisplay = new CLinearFrameDisplay( spRenderer );
				g_Settings()->Set( "settings.player.DisplayMode", 1 );
			}
		}
	}

	if( spFrameDisplay && !spFrameDisplay->Valid() )
	{
		g_Log->Warning( "FrameDisplay failed, falling back to normal" );
		g_Settings()->Set( "settings.player.DisplayMode", 0 );
		spFrameDisplay = NULL;
	}

	//	Fallback to normal.
	if( spFrameDisplay == NULL )
	{
		g_Log->Info( "Using normal video display..." );
		spFrameDisplay = new CFrameDisplay( spRenderer );
		g_Settings()->Set( "settings.player.DisplayMode", 0 );
	}

	spFrameDisplay->SetDisplaySize( spDisplay->Width(), spDisplay->Height() );
	
	{
		DisplayUnit *du = new DisplayUnit;
		
		du->spFrameDisplay = spFrameDisplay;
		du->spRenderer = spRenderer;
		du->spDisplay = spDisplay;
		du->m_MetaData.m_SheepID = 0;
		du->m_MetaData.m_SheepGeneration = 0;
		du->m_MetaData.m_Fade = 1.f;
		du->m_MetaData.m_FileName = "";
		du->m_MetaData.m_LastAccessTime = time(NULL);
		du->m_MetaData.m_IsEdge = false;
		
		if ( m_MultiDisplayMode == kMDIndividualMode && !Stopped() )
		{
			du->spDecoder = CreateContentDecoder( true );
			du->spDecoder->Start();
		}
		

		boost::mutex::scoped_lock lockthis( m_displayListMutex );
		
		
		if (g_Settings()->Get( "settings.player.reversedisplays", false ) == true)
			m_displayUnits.insert(m_displayUnits.begin(), du);
		else
			m_displayUnits.push_back(du);
		
	}
	
	return true;
}

/*
*/
bool	CPlayer::Startup()
{
	m_DisplayFps = g_Settings()->Get( "settings.player.display_fps", 60. );
	
#ifdef HONOR_VBL_SYNC
	if ( g_Settings()->Get( "settings.player.vbl_sync", false ) )
	{
		m_DisplayFps = 0.0;
	}
#endif
	
	//	Grab some paths for the decoder.
    std::string content = g_Settings()->Root() + "content/";
#ifndef LINUX_GNU
	std::string	scriptRoot = g_Settings()->Get( "settings.app.InstallDir", std::string("./") ) + "Scripts";
#else
	std::string	scriptRoot = g_Settings()->Get( "settings.app.InstallDir", std::string(SHAREDIR) ) + "Scripts";
#endif
	std::string watchFolder = g_Settings()->Get( "settings.content.sheepdir", content ) + "/mpeg/";

 	if( TupleStorage::IStorageInterface::CreateFullDirectory( content.c_str() ) )
	{
		if (m_InitPlayCounts)
			g_PlayCounter().SetDirectory( content );
	}

    //  Tidy up the paths.
    path    scriptPath = scriptRoot;
    path    watchPath = watchFolder;

	//	Create playlist.
	g_Log->Info( "Creating playlist..." );
  	m_spPlaylist = new ContentDecoder::CLuaPlaylist(	scriptPath.string(),
														watchPath.string(),
														m_UsedSheepType );

	//	Create decoder last.
	g_Log->Info( "Starting decoder..." );
		
	m_bStarted = false;

	return true;
}

ContentDecoder::CContentDecoder *CPlayer::CreateContentDecoder( bool _bStartByRandom )
{
	if ( m_spPlaylist.IsNull() )
		return NULL;
	
#ifndef LINUX_GNU
	AVPixelFormat pf = AV_PIX_FMT_RGB32;
	
	//On PowerPC machines we need to use different pixel format!
#if defined(MAC) && defined(__BIG_ENDIAN__)
	pf = AV_PIX_FMT_BGR32_1;
#endif

#else

	AVPixelFormat pf = AV_PIX_FMT_BGR32;
#if defined(__BIG_ENDIAN__)
	pf = AV_PIX_FMT_RGB32_1;
#endif

#endif

	return new ContentDecoder::CContentDecoder( m_spPlaylist, _bStartByRandom, g_Settings()->Get( "settings.player.CalculateTransitions", true ), (uint32)abs(g_Settings()->Get( "settings.player.BufferLength", 25 )), pf );
}

/*
 
 */
void CPlayer::ForceWidthAndHeight(uint32 du, uint32 _w, uint32 _h)
{
    boost::mutex::scoped_lock lockthis( m_displayListMutex );
    
    if (du >= m_displayUnits.size())
        return;
    
	const DisplayUnit* duptr = m_displayUnits[du];
    
    if (duptr == NULL)
        return;
    
#ifdef MAC
    if (!duptr->spDisplay.IsNull())
    {
        spCDisplayOutput disp = duptr->spDisplay;
        disp->ForceWidthAndHeight(_w, _h);
    }
#endif
    
    if (!duptr->spFrameDisplay.IsNull())
    {
        spCFrameDisplay fd = duptr->spFrameDisplay;
        fd->SetDisplaySize(_w, _h);
    }
}

/*

*/
void	CPlayer::Start()
{	
	if ( !m_bStarted )
	{
		m_CapClock = 0.0;
		
		if ( m_MultiDisplayMode == kMDSharedMode )
		{
			m_spDecoder =  CreateContentDecoder( true );

			if( !m_spDecoder->Start() )
				g_Log->Warning( "Nothing to play" );
		}
		else
		{
			boost::mutex::scoped_lock lockthis( m_displayListMutex );
			
			DisplayUnitIterator it = m_displayUnits.begin();
			
			for ( ; it != m_displayUnits.end(); it++ )
			{
				if ((*it)->spDecoder.IsNull())
					(*it)->spDecoder = CreateContentDecoder( true );
					
				if( !(*it)->spDecoder->Start() )
					g_Log->Warning( "Nothing to play" );
			}
		}

		m_bStarted = true;
	
		//m_spRenderer->Reset( DisplayOutput::eEverything );
		//m_spRenderer->Orthographic();
	}
}

/*
*/
void	CPlayer::Stop()
{
	if ( m_bStarted )
	{
		if ( m_MultiDisplayMode == kMDSharedMode )
		{
			m_spDecoder->Stop();
		}
		else
		{
			boost::mutex::scoped_lock lockthis( m_displayListMutex );

			DisplayUnitIterator it = m_displayUnits.begin();
			
			for ( ; it != m_displayUnits.end(); it++ )
			{
				if (!(*it)->spDecoder.IsNull())
					(*it)->spDecoder->Stop();
			}
		}
	}
	
	m_bStarted = false;
}

/*
*/
bool	CPlayer::Shutdown( void )
{
	g_Log->Info( "CPlayer::Shutdown()\n" );
	
	Stop();

	if ( m_MultiDisplayMode == kMDSharedMode )
	{
		if (m_spDecoder.IsNull() == false)
			m_spDecoder->Close();
	}
	else
	{
		boost::mutex::scoped_lock lockthis( m_displayListMutex );

		DisplayUnitIterator it = m_displayUnits.begin();
		
		for ( ; it != m_displayUnits.end(); it++ )
		{
			if (!(*it)->spDecoder.IsNull())
				(*it)->spDecoder->Close();
				
		}
	}
	
	{
		boost::mutex::scoped_lock lockthis( m_displayListMutex );

		DisplayUnitIterator it = m_displayUnits.begin();
		
		for ( ; it != m_displayUnits.end(); it++ )
		{
			delete (*it);
		}
	}

	m_spPlaylist = NULL;
	
	m_spDecoder = NULL;
	
	m_displayUnits.clear();
	
	m_bStarted = false;
	
	return true;
}

/*
*/
CPlayer::~CPlayer()
{
	//	Mark singleton as properly shutdown, to track unwanted access after this point.
	SingletonActive( false );
}

bool	CPlayer::BeginFrameUpdate()
{
	if ( m_MultiDisplayMode == kMDSharedMode )
	{
		if (m_spDecoder.IsNull() == false)
			m_spDecoder->ResetSharedFrame();
	}
	else
	{
		boost::mutex::scoped_lock lockthis( m_displayListMutex );

		DisplayUnitIterator it = m_displayUnits.begin();
		
		for ( ; it != m_displayUnits.end(); it++ )
		{
			if (!(*it)->spDecoder.IsNull())
				(*it)->spDecoder->ResetSharedFrame();
				
		}
	}
	
	return true;
}

bool	CPlayer::EndFrameUpdate()
{
	spCFrameDisplay spFD;
	
	{
		boost::mutex::scoped_lock lockthis( m_displayListMutex );
		
		spFD = m_displayUnits[ 0 ]->spFrameDisplay;
	}
	
	fp8 capFPS = spFD->GetFps( m_PlayerFps, m_DisplayFps );
	
	if ( !spFD.IsNull() && capFPS > 0.000001)
		FpsCap( capFPS );
	
	return true;
}

bool	CPlayer::BeginDisplayFrame( uint32 displayUnit )
{
	DisplayUnit* du;
	
	{
		boost::mutex::scoped_lock lockthis( m_displayListMutex );
		
		if (displayUnit >= m_displayUnits.size())
			return false;
					
		du = m_displayUnits[ displayUnit ];
	}

	if (du->spRenderer->BeginFrame() == false)
		return false;
		
	return true;
}

bool	CPlayer::EndDisplayFrame( uint32 displayUnit, bool drawn )
{
	DisplayUnit* du;
	
	{	
		boost::mutex::scoped_lock lockthis( m_displayListMutex );
		
		if (displayUnit >= m_displayUnits.size())
			return false;
			
		du = m_displayUnits[ displayUnit ];
	}

	return du->spRenderer->EndFrame( drawn );
}

//	Chill the remaining time to keep the framerate.
void CPlayer::FpsCap( const fp8 _cap )
{
	fp8	diff = 1.0/_cap - (m_Timer.Time() - m_CapClock);
	if( diff > 0.0 )
		Base::CTimer::Wait( diff );

	m_CapClock = m_Timer.Time();
}


/*
	Update().

*/
bool	CPlayer::Update(uint32 displayUnit, bool &bPlayNoSheepIntro)
{	
	bPlayNoSheepIntro = false;

	DisplayUnit* du;
	
	{
		boost::mutex::scoped_lock lockthis( m_displayListMutex );
		
		if (displayUnit >= m_displayUnits.size())
			return false;
		
		du = m_displayUnits[ displayUnit ];
	}

	du->spRenderer->Reset( eEverything );
	du->spRenderer->Orthographic();
	du->spRenderer->Apply();
	
	{
		boost::mutex::scoped_lock lockthis( m_updateMutex );

	//	Update the frame display, it rests before doing any work to keep the framerate.
	if( !du->spFrameDisplay->Update( du->spDecoder.IsNull() ? m_spDecoder : du->spDecoder, m_PlayerFps, m_DisplayFps, du->m_MetaData ) )
	{
			if ( (m_spDecoder.IsNull() == false && m_spDecoder->PlayNoSheepIntro()) || 
				 (du->spDecoder.IsNull() == false && du->spDecoder->PlayNoSheepIntro()) )
			{
				bPlayNoSheepIntro = true;
				return true;
			}
			return false;
			//	Failed to update screen here, do something noticeable like show a logo or something.. :)
			//g_Log->Warning( "Failed to render frame..." );
	}
	}
	
	if ( (m_spDecoder.IsNull() == false && m_spDecoder->PlayNoSheepIntro()) || 
		 (du->spDecoder.IsNull() == false && du->spDecoder->PlayNoSheepIntro()) )
	{
		bPlayNoSheepIntro = true;
		return true;
	}

	return true;
}
