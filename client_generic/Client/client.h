#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <string>
#include <sstream>
#include <iomanip>
#include "base.h"
#include "Exception.h"
#include "Log.h"
#include "Player.h"
#include "storage.h"
#include "Networking.h"
#include "Settings.h"
#include "clientversion.h"

#include "ContentDownloader.h"
#include "SheepGenerator.h"
#include "Shepherd.h"
#include "Voting.h"
#include "Timer.h"
#include "TextureFlat.h"
#include "Hud.h"
#include "Splash.h"
#include "StatsConsole.h"
#include "ServerMessage.h"
#include "Matrix.h"
#include "CrossFade.h"
#include "StartupScreen.h"
#if defined(WIN32) && defined(_MSC_VER)
#include "../msvc/msvc_fix.h"
#include "../msvc/cpu_usage_win32.h"
#else
#if defined(LINUX_GNU)
#include <limits.h>
#include "cpu_usage_linux.h"
#else
#include "cpu_usage_mac.h"
#endif
#endif
/*
	CElectricSheep().
	Prime mover for the client, used from main.cpp...
*/
class	CElectricSheep
{
	protected:
		ESCpuUsage		m_CpuUsage;
		fp8				m_LastCPUCheckTime;
		int				m_CpuUsageTotal;
		int				m_CpuUsageES;
		int				m_HighCpuUsageCounter;
		int				m_CpuUsageThreshold;
		bool			m_MultipleInstancesMode;
		bool			m_bConfigMode;
		bool			m_SeamlessPlayback;
		Base::CTimer	m_Timer;
		Base::CTimer	m_F1F4Timer;

		Hud::spCHudManager		m_HudManager;
		Hud::spCStartupScreen	m_StartupScreen;
		
		fp8						m_PNGDelayTimer;

		//	The base framerate(from config).
		fp8			m_PlayerFps;

		//	Potentially adjusted framerate(from <- and -> keys)
		fp8			m_CurrentFps;

		fp8			m_OriginalFps;

		//	Voting object.
		CVote			*m_pVoter;

		//	Default root directory, ie application data.
		std::string	m_AppData;

		//	Default application working directory.
		std::string	m_WorkingDir;


		//	Splash images.
		Hud::spCSplash m_spSplashPos;
		Hud::spCSplash m_spSplashNeg;
		
		// Splash PNG
		Hud::spCSplashImage m_spSplashPNG;
		Base::CTimer m_SplashPNGDelayTimer;
		int m_nSplashes;
		std::string m_SplashFilename;

		Hud::spCCrossFade m_spCrossFade;

		std::deque<std::string> m_ConnectionErrors;
		
		uint32 m_curPlayingID;
		uint32 m_curPlayingGen;
		uint64 m_lastPlayedSeconds;
		
#ifdef DO_THREAD_UPDATE
		boost::barrier* m_pUpdateBarrier;
		
		boost::thread_group *m_pUpdateThreads;
		
		boost::mutex m_BarrierMutex;
#endif
		

		//	Init tuplestorage.
		bool	InitStorage(bool _bReadOnly = false)
		{
			g_Log->Info( "InitStorage()" );
#ifndef LINUX_GNU
			if (g_Settings()->Init( m_AppData, m_WorkingDir, _bReadOnly ) == false)
#else
			char appdata[PATH_MAX];
			snprintf( appdata, PATH_MAX, "%s/.electricsheep/", getenv("HOME") );
			if (g_Settings()->Init( appdata, SHAREDIR ) == false)
#endif
				return false;

			//	Trigger this to exist in the settings.
#ifndef LINUX_GNU
			g_Settings()->Set( "settings.app.InstallDir", m_WorkingDir );
#else
			g_Settings()->Set( "settings.app.InstallDir", SHAREDIR );
#endif
			return true;
		}

		//	Attach log.
		void	AttachLog()
		{
			TupleStorage::IStorageInterface::CreateFullDirectory( m_AppData + "Logs/" );
			if (g_Settings()->Get( "settings.app.log", false ))
				g_Log->Attach( m_AppData + "Logs/" );
			g_Log->Info( "AttachLog()" );

            g_Log->Info( "******************* %s (Built %s / %s)...\n", CLIENT_VERSION_PRETTY, __DATE__,__TIME__ );
        }

	public:
			CElectricSheep()
			{
				m_CpuUsageTotal = -1;
				m_CpuUsageES = -1;
				m_CpuUsageThreshold = 50;
				m_HighCpuUsageCounter = 0;
				m_bConfigMode = false;
				m_MultipleInstancesMode = false;
				printf( "CElectricSheep()\n" );

				m_pVoter = NULL;
#ifndef LINUX_GNU
				m_AppData = "./.ElectricSheep/";
				m_WorkingDir = "./";
#else
				m_AppData = std::string(getenv("HOME"))+"/.electricsheep/";
				m_WorkingDir = SHAREDIR;
#endif			
#ifdef DO_THREAD_UPDATE
				m_pUpdateBarrier = NULL;
				m_pUpdateThreads = NULL;
#endif
			}

			virtual ~CElectricSheep()
			{
			}

			//
			virtual const bool	Startup()
			{
				m_CpuUsageThreshold = g_Settings()->Get( "settings.player.cpuusagethreshold", 50 );

				if (m_MultipleInstancesMode == false)
				{
					g_NetworkManager->Startup();

					//	Set proxy info.
					if( g_Settings()->Get( "settings.content.use_proxy", false ) )
					{
						g_Log->Info( "Using proxy server..." );
						g_NetworkManager->Proxy( g_Settings()->Get( "settings.content.proxy", std::string("") ),
												 g_Settings()->Get( "settings.content.proxy_username", std::string("") ),
												 g_Settings()->Get( "settings.content.proxy_password", std::string("") ) );
					}
					
					//if some user has a legacy clear-text password, lets convert it to md5 one.  (should we remove the old one ???)
					if (g_Settings()->Get( "settings.content.password_md5", std::string("") ) == "") {
						g_Settings()->Set( "settings.content.password_md5", ContentDownloader::Shepherd::computeMD5( g_Settings()->Get( "settings.content.password", std::string("") ) ) );
						//g_Settings()->Set( "settings.content.password", "" );
					}

					g_NetworkManager->Login( g_Settings()->Get( "settings.generator.nickname", std::string("") ), g_Settings()->Get( "settings.content.password_md5", std::string("") ) );
					
					m_pVoter = new CVote();
				}

				g_Player().SetMultiDisplayMode( (CPlayer::MultiDisplayMode)g_Settings()->Get( "settings.player.MultiDisplayMode", 0 ) );

                //	Init the display and create decoder.
                if( !g_Player().Startup() )
                    return false;
					
#ifdef DO_THREAD_UPDATE
				CreateUpdateThreads();
#endif

				m_curPlayingID = 0;
				m_curPlayingGen = 0;
				m_lastPlayedSeconds = 0;

                //	Set framerate.
                m_PlayerFps = g_Settings()->Get( "settings.player.player_fps", 20. );
				if ( m_PlayerFps < 0.1 )
					m_PlayerFps = 1.;
				m_OriginalFps = m_PlayerFps;
                m_CurrentFps = m_PlayerFps;
				
				g_Player().Framerate( m_CurrentFps );


                //	Get hud font size.
                std::string	hudFontName = g_Settings()->Get( "settings.player.hudFontName", std::string("Trebuchet MS") );
                int32		hudFontSize = g_Settings()->Get( "settings.player.hudFontSize", 24 );

				m_PNGDelayTimer = g_Settings()->Get( "settings.player.pngdelaytimer", 600);

				m_HudManager = new Hud::CHudManager;

				m_HudManager->Add( "helpmessage", new Hud::CStatsConsole( Base::Math::CRect( 1, 1 ), hudFontName, hudFontSize ) );
				
				Hud::spCStatsConsole spHelpMessage = (Hud::spCStatsConsole)m_HudManager->Get( "helpmessage" );
				spHelpMessage->Add( new Hud::CStringStat( "message", "Electric Sheep\n\nYou are now part of a cyborg mind composed of thousands of\ncomputers and people communicating with a genetic algorithm.\n\nKeyboard Commands\nUp-arrow: vote for this sheep\nDown-arrow: vote against this sheep and delete it\nLeft-arrow: go back to play previous sheep\nRight-arrow: go forward through history\n"
#ifdef MAC
				"Cmd" 
#else
				"Control" 
#endif
				"-F: go full screen (if in window)\nF1: help (this page)\nF2: download status and error log\nF3: render status\nF4: playback status\n\nby Scott Draves and open source programmers all over the world\nElectricSheep.org", "" ) );	
				
				std::string ver = GetVersion();
                
				if (!ver.empty())
				{
					spHelpMessage->Add( new Hud::CStringStat( "version", "\nVersion: ", ver ) );
				}
				
				m_SeamlessPlayback = g_Settings()->Get( "settings.player.SeamlessPlayback", false );

				//	Add some display stats.
                m_HudManager->Add( "displaystats", new Hud::CStatsConsole( Base::Math::CRect( 1, 1 ), hudFontName, hudFontSize ) );
                Hud::spCStatsConsole spStats = (Hud::spCStatsConsole)m_HudManager->Get( "displaystats" );
                spStats->Add( new Hud::CStringStat( "decodefps", "Decoding video at ", "? fps" ) );
				

                int32 displayMode = g_Settings()->Get( "settings.player.DisplayMode", 0 );
                if( displayMode == 2 )
                    spStats->Add( new Hud::CAverageCounter( "displayfps", 
					g_Player().Renderer()->Description() +
					" piecewise cubic display at ", " fps", 1.0 ) );
                else if( displayMode == 1 )
                    spStats->Add( new Hud::CAverageCounter( "displayfps", 
					g_Player().Renderer()->Description() +
					" piecewise linear display at ", " fps", 1.0 ) );
                else
                    spStats->Add( new Hud::CAverageCounter( "displayfps", 
					g_Player().Renderer()->Description() +
					" display at ", " fps", 1.0 ) );

				spStats->Add( new Hud::CStringStat( "currentid", "Currently playing sheep: ", "n/a" ) );
                spStats->Add( new Hud::CStringStat( "uptime", "\nClient uptime: ", "...." ) );

                //	Add some server stats.
                m_HudManager->Add( "serverstats", new Hud::CStatsConsole( Base::Math::CRect( 1, 1 ), hudFontName, hudFontSize ) );
                spStats = (Hud::spCStatsConsole)m_HudManager->Get( "serverstats" );
				spStats->Add( new Hud::CStringStat( "loginstatus", "", "Not logged in" ) );
				spStats->Add( new Hud::CStringStat( "all", "Local flock: ", "unknown..." ) );
                spStats->Add( new Hud::CStringStat( "server", "Server is ", "not known yet" ) );
                spStats->Add( new Hud::CStringStat( "transfers", "", "" ) );
				spStats->Add( new Hud::CStringStat( "deleted", "", "" ) );
				spStats->Add( new Hud::CStringStat( "bsurvivors", "", "" ) );
				if (m_MultipleInstancesMode == true)
					spStats->Add( new Hud::CTimeCountDownStat( "svstat", "", "Downloading disabled, read-only mode" ) );
				else
					if (g_Settings()->Get( "settings.content.download_mode", true ) == false)
						spStats->Add( new Hud::CTimeCountDownStat( "svstat", "", "Downloading disabled" ) );
					else
						spStats->Add( new Hud::CTimeCountDownStat( "svstat", "", "Preparing downloader..." ) );
				spStats->Add( new Hud::CStringStat( std::string("zconnerror"), "", "" ) );

                //	Add some display stats.
                m_HudManager->Add( "renderstats", new Hud::CStatsConsole( Base::Math::CRect( 1, 1 ), hudFontName, hudFontSize ) );
                spStats = (Hud::spCStatsConsole)m_HudManager->Get( "renderstats" );
				spStats->Add( new Hud::CTimeCountDownStat( "countdown", "", "Rendering disabled" ) );
                spStats->Add( new Hud::CIntCounter( "rendering", "Currently rendering ", " frames" ) );
                spStats->Add( new Hud::CIntCounter( "totalframes", "", " frames rendered" ) );
				
				bool hasBattery = (GetACLineStatus() != -1);								
				
				if (hasBattery)
					spStats->Add( new Hud::CStringStat( "zbattery", "\nPower source: ", "Unknown" ) );
			
				spStats->Add( new Hud::CStringStat( "zzacpu", "CPU usage: ", "Unknown" ) );

#ifndef LINUX_GNU
				std::string defaultDir = std::string(".\\");
#else
				std::string defaultDir = std::string("");
#endif
				//	Vote splash.
				m_spSplashPos = new Hud::CSplash( 0.2f, g_Settings()->Get( "settings.app.InstallDir", defaultDir ) + "electricsheep-smile.png" );
				m_spSplashNeg = new Hud::CSplash( 0.2f, g_Settings()->Get( "settings.app.InstallDir", defaultDir ) + "electricsheep-frown.png" );
                
				// PNG splash
				m_SplashFilename = g_Settings()->Get( "settings.player.attrpngfilename", g_Settings()->Get( "settings.app.InstallDir", defaultDir ) + "electricsheep-attr.png"  );

				const char *percent;

				if (m_SplashFilename.empty() == false && (percent = strchr(m_SplashFilename.c_str(), '%')))
				{
					if (percent[1] == 'd')
					{
						FILE *test;
						while (1)
						{
							char fNameFormatted[FILENAME_MAX];
							snprintf(fNameFormatted, FILENAME_MAX, m_SplashFilename.c_str(), m_nSplashes);
							if ((test = fopen(fNameFormatted, "r")))
							{
								fclose(test);
								m_nSplashes++;
							}
							else 
							{
								break;
							}
						}
					}
				}


				// if multiple splashes are found then they are loaded when the timer goes off, not here
				if ( m_SplashFilename.empty() == false && g_Settings()->Get( "settings.app.attributionpng", true ) == true )
				  m_spSplashPNG = new Hud::CSplashImage( 0.2f, m_SplashFilename.c_str(),
									 fp4( g_Settings()->Get( "settings.app.pngfadein", 10 ) ),
									 fp4( g_Settings()->Get( "settings.app.pnghold", 10 ) ),
									 fp4( g_Settings()->Get( "settings.app.pngfadeout", 10 ) )
									 );

				
				m_spCrossFade = new Hud::CCrossFade( g_Player().Display()->Width(), g_Player().Display()->Height(), true );
				
                //	Start downloader.
                g_Log->Info( "Starting downloader..." );

				g_ContentDownloader().Startup( false, m_MultipleInstancesMode );

				//call static method to fill sheep counts
				ContentDownloader::Shepherd::GetFlockSizeMBsRecount(0);
				ContentDownloader::Shepherd::GetFlockSizeMBsRecount(1);
                //	For testing...
                //ContentDownloader::Shepherd::addMessageText( "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua", 50, 18 );

                //	And we're off.
				m_SplashPNGDelayTimer.Reset();
                m_Timer.Reset();
                g_Player().Start();
				m_F1F4Timer.Reset();
				m_LastCPUCheckTime = m_Timer.Time();
				return true;
			}

			//
			virtual void	Shutdown()
			{
				printf( "CElectricSheep::Shutdown()\n" );

#ifdef DO_THREAD_UPDATE
				DestroyUpdateThreads();
#endif

				m_spSplashPos = NULL;
				m_spSplashNeg = NULL;
				m_spSplashPNG = NULL;
				m_nSplashes = 0;
				m_SplashFilename = std::string();
				m_spCrossFade = NULL;
				m_StartupScreen = NULL;
				m_HudManager = NULL;
				
				if( !m_bConfigMode )
				{	
					g_ContentDownloader().Shutdown();
					
					//	This stuff was never started in config mode.
					if (m_MultipleInstancesMode == false)
					{
						SAFE_DELETE( m_pVoter );

						g_NetworkManager->Shutdown();						
					}
					g_Player().Shutdown();
					g_Settings()->Shutdown();
				}
			}

			const bool	Run()
			{
				while( true )
				{
					//g_Player().Renderer()->BeginFrame();

					if( !Update() )
					{
						g_Player().Renderer()->EndFrame();
							return false;
					}
				}

				return true;
			}
			
			std::string FormatTimeDiff(uint64 timediff, bool showseconds)
			{						
				std::stringstream ss;
				
				//	Prettify uptime.
				uint32	seconds = ( timediff % 60 );
				uint32	minutes = (timediff / 60) % 60;
				uint32	hours = (timediff / 3600) % 24;
				uint32	days = uint32((timediff / (60*60*24)));
				const char *space = "";
				if (days > 0)
				{
					ss << days << " day" << ((days != 1) ? "s" : "") << ",";
					space = " ";
				}
					
				if (!showseconds)
				{
					if (days > 0)
					{
						if (hours > 0)
							ss << space << hours << " hour" << ((hours != 1) ? "s" : "");
						else if (minutes > 0)
							ss << space << minutes << " minute" << ((minutes != 1) ? "s" : "");
						else if (seconds > 0)
							ss << space << seconds << " second" << ((seconds != 1) ? "s" : "");
					}
					else
					{
						ss << space;
						
						if (hours > 0)
							ss << space << std::setfill('0') << std::setw(2) << hours << "h";
						
						ss << std::setfill('0') << std::setw(2) << minutes << "m" << std::setfill('0') << std::setw(2) << seconds << "s";					 
					}
				}
				else
				{
					ss << space;
					
					if (hours > 0)
						ss << space << std::setfill('0') << std::setw(2) << hours << "h";
					
					ss << std::setfill('0') << std::setw(2) << minutes << "m" << std::setfill('0') << std::setw(2) << seconds << "s";					 
				}
											
				return ss.str();
			}
			
#ifdef DO_THREAD_UPDATE
			//
			virtual void CreateUpdateThreads()
			{
				int displayCnt = g_Player().GetDisplayCount();
									
				m_pUpdateBarrier = new boost::barrier( displayCnt + 1 );
				
				m_pUpdateThreads = new boost::thread_group;
				
				for (int i = 0; i < displayCnt; i++)
				{
					boost::thread* th = new boost::thread(&CElectricSheep::UpdateFrameThread, this, i);
#ifdef WIN32
					SetThreadPriority( (HANDLE)th->native_handle(), THREAD_PRIORITY_HIGHEST );
					SetThreadPriorityBoost( (HANDLE)th->native_handle(), FALSE );
#else
					struct sched_param sp;
					sp.sched_priority = sched_get_priority_max(SCHED_RR); //HIGH_PRIORITY_CLASS - THREAD_PRIORITY_NORMAL
					pthread_setschedparam( (pthread_t)th->native_handle(), SCHED_RR, &sp );
#endif
					m_pUpdateThreads->add_thread(th);
				}
			}
			
			//
			virtual void DestroyUpdateThreads()
			{
				if ( m_pUpdateThreads != NULL )
				{
					m_pUpdateThreads->interrupt_all();
					m_pUpdateThreads->join_all();
					
					SAFE_DELETE(m_pUpdateThreads);
				}

				SAFE_DELETE(m_pUpdateBarrier);
			}
#endif

			//
			virtual const bool Update()
			{
				g_Player().BeginFrameUpdate();

#ifdef DO_THREAD_UPDATE
				{
					boost::mutex::scoped_lock lock( m_BarrierMutex );
				
					m_pUpdateBarrier->wait();
				
					m_pUpdateBarrier->wait();
				}
#else
				int displayCnt = g_Player().GetDisplayCount();
									
				for (int i = 0; i < displayCnt; i++)
				{
					DoRealFrameUpdate(i);
				}				
#endif
								
				g_Player().EndFrameUpdate();

				return true;
			}
			
#ifdef DO_THREAD_UPDATE
			virtual void UpdateFrameThread(int32 displayUnit)
			{
				try {
					while (true)
					{
						m_pUpdateBarrier->wait();
						
						DoRealFrameUpdate(displayUnit);
						
						m_pUpdateBarrier->wait();
					}
				}
				catch(boost::thread_interrupted const&)
				{
				}
			}
#endif
			
			virtual bool DoRealFrameUpdate(int32 displayUnit)
			{
				if ( g_Player().BeginDisplayFrame( displayUnit ) )
				{
				
					//g_Player().Renderer()->BeginFrame();
					
					if( g_Player().Closed() )
					{
						g_Log->Info( "Player closed..." );
						g_Player().Renderer()->EndFrame();
						return false;
					}

					bool drawNoSheepIntro = false;
					bool drawn = g_Player().Update(displayUnit, drawNoSheepIntro);
					
					if ( (drawNoSheepIntro && displayUnit == 0) || (drawn && displayUnit == 0) )
					{
						if (drawNoSheepIntro)
						{
							if (m_StartupScreen.IsNull())
								m_StartupScreen = new Hud::CStartupScreen(Base::Math::CRect(0,0,1.,1.), "Trebuchet MS", 24);
							m_StartupScreen->Render(0., g_Player().Renderer());
						}
						//	Process any server messages.
						std::string msg;
						fp8 duration;

						if ( !m_spSplashPNG.IsNull() )
						{
							if ( m_SplashPNGDelayTimer.Time() >  m_PNGDelayTimer)
							{


							  // update m_spSplashPNG here, so every time it is shown, it is randomized among our shuffle group.

								if (m_nSplashes > 0)
								{
									char fNameFormatted[FILENAME_MAX];
									snprintf( fNameFormatted, FILENAME_MAX, m_SplashFilename.c_str(), rand() % m_nSplashes );
									if ( m_SplashFilename.empty() == false && g_Settings()->Get( "settings.app.attributionpng", true ) == true )
										m_spSplashPNG = new Hud::CSplashImage( 0.2f, fNameFormatted,
														fp4( g_Settings()->Get( "settings.app.pngfadein", 10 ) ),
														fp4( g_Settings()->Get( "settings.app.pnghold", 10 ) ),
														fp4( g_Settings()->Get( "settings.app.pngfadeout", 10 ) )
														);
								}



								m_HudManager->Add( "splash_png", m_spSplashPNG,
									fp4( g_Settings()->Get( "settings.app.pngfadein", 10 ) +
									g_Settings()->Get( "settings.app.pnghold", 10 ) +
									g_Settings()->Get( "settings.app.pngfadeout", 10 ) )
									);
								m_SplashPNGDelayTimer.Reset();
							}
						}

						if( ContentDownloader::Shepherd::PopMessage( msg, duration ))
						{
							bool addtohud = true;
							time_t lt = time(NULL);

							if (( msg == "error connecting to server" ) ||
								( msg == "server request failed, using free one" ))
							{
								std::string temptime = ctime(&lt);
								msg =  temptime.substr(0, temptime.length() - strlen("\n")) + " " + msg;
								g_Log->Error(msg.c_str());
								if (m_ConnectionErrors.size() > 20)
									m_ConnectionErrors.pop_front();
								m_ConnectionErrors.push_back(msg);
								if (g_Settings()->Get( "settings.player.quiet_mode", true ) == true)
									addtohud = false;
							}

							if (addtohud == true)
							{
								m_HudManager->Add( "servermessage", new Hud::CServerMessage( msg, Base::Math::CRect( 1, 1 ), 24 ), duration );
								m_HudManager->HideAll();
							}
						}
						if (m_F1F4Timer.Time() > 20*60)
						{
							m_F1F4Timer.Reset();
							m_HudManager->Hide( "helpmessage" );
							m_HudManager->Hide( "serverstats" );
							m_HudManager->Hide( "renderstats" );	
							m_HudManager->Hide( "displaystats" );	
						}

						
						std::string batteryStatus = "Unknown";
						bool blockRendering = false;
						if (m_Timer.Time() > m_LastCPUCheckTime + 3.)
						{
							m_LastCPUCheckTime = m_Timer.Time();
							if (m_CpuUsage.GetCpuUsage(m_CpuUsageTotal, m_CpuUsageES))
							if (m_CpuUsageTotal != -1 && m_CpuUsageES != -1 && m_CpuUsageTotal > m_CpuUsageES)
							{
								if ((m_CpuUsageTotal - m_CpuUsageES) > m_CpuUsageThreshold)
								{
									++m_HighCpuUsageCounter;
									if (m_HighCpuUsageCounter > 10)
									{
										m_HighCpuUsageCounter = 5;
										blockRendering = true;
									}
								} else
								{
									if (m_HighCpuUsageCounter > 0)
										--m_HighCpuUsageCounter;
								}
							}
						}
						if (m_HighCpuUsageCounter > 0 && ContentDownloader::Shepherd::RenderingAllowed() == false)
							blockRendering = true;

						switch( GetACLineStatus() )
						{
							case 1:
							{
								batteryStatus = "Power adapter";
								/*m_PlayerFps = m_OriginalFps;
								m_CurrentFps = m_PlayerFps;
								g_Player().Framerate( m_CurrentFps );*/
								break;
							}
							case 0:
							{
								batteryStatus = "Battery";
								/*m_PlayerFps = m_OriginalFps/2; // half speed on battery power
								m_CurrentFps = m_PlayerFps;
								g_Player().Framerate( m_CurrentFps );*/
								blockRendering = true;
								break;
							}
							case 255:
							{
								batteryStatus = "Unknown1";
								break;
							}
							default:
							{
								batteryStatus = "Unknown2";
								break;
							}
						}

						if (blockRendering)
							ContentDownloader::Shepherd::SetRenderingAllowed(false);
						else
							ContentDownloader::Shepherd::SetRenderingAllowed(true);

						//	Update some stats.
						Hud::spCStatsConsole spStats = (Hud::spCStatsConsole)m_HudManager->Get( "displaystats" );
						std::stringstream decodefpsstr;
						decodefpsstr.precision(2);
						decodefpsstr << std::fixed << m_CurrentFps << " fps";
						((Hud::CStringStat *)spStats->Get( "decodefps" ))->SetSample( decodefpsstr.str() );
						((Hud::CIntCounter *)spStats->Get( "displayfps" ))->AddSample( 1 );

						uint32 playingID = g_Player().GetCurrentPlayingSheepID();
						uint32 playingGen = g_Player().GetCurrentPlayingSheepGeneration();
						uint16 playCnt = g_PlayCounter().PlayCount( playingGen, playingID ) - 1;
						
						char strCurID[256];
						if (m_curPlayingID != playingID || m_curPlayingGen != playingGen)
						{
							if (playCnt > 0)
							{
								time_t lastatime = g_Player().GetCurrentPlayingatime();
								time_t currenttime = time(NULL);
								m_lastPlayedSeconds = (uint64)floor(difftime(currenttime, lastatime));
							}
							else
								m_lastPlayedSeconds = 0;
							
							m_curPlayingID = playingID;
							m_curPlayingGen = playingGen;
						}
						
						char playCntStr[128];
												
						if (playCnt > 0)
							sprintf(playCntStr, "Played: %hu time%s %s\nLast time: %s ago", 
								playCnt, 
								(playCnt == 1) ? "" : "s",
								( g_PlayCounter().ReadOnlyPlayCounts() ) ? " (not updated, read-only instance)" : "",
								FormatTimeDiff(m_lastPlayedSeconds, false).c_str()
								);
						else
							strcpy(playCntStr, "Playing for the first time");
						
						snprintf( strCurID, 256, "#%d.%05d (%s)\n%s\n",
							g_Player().GetCurrentPlayingGeneration(),
							playingID,
							g_Player().IsCurrentPlayingEdge() ? "edge" : "loop",
							playCntStr
							);
						if (playingID != 0)
							((Hud::CStringStat *)spStats->Get( "currentid" ))->SetSample( strCurID );

						//	Prettify uptime.
						uint64	uptime = (uint64)m_Timer.Time();

						char strHP[128];
						snprintf( strHP, 127, "%s", FormatTimeDiff(uptime, true).c_str() );
						((Hud::CStringStat *)spStats->Get( "uptime" ))->SetSample( strHP );

						//	Serverstats.
						spStats = (Hud::spCStatsConsole)m_HudManager->Get( "serverstats" );
						
						std::stringstream tmpstr;
						uint64 flockcount = 0;
						uint64 flockmbs = 0;
						uint64 flockcountfree = ContentDownloader::Shepherd::getClientFlockCount(0);
						uint64 flockcountgold = ContentDownloader::Shepherd::getClientFlockCount(1);
						uint64 flockmbsfree = ContentDownloader::Shepherd::getClientFlockMBs(0);
						uint64 flockmbsgold = ContentDownloader::Shepherd::getClientFlockMBs(1);
						switch ( g_Player().UsedSheepType() )
						{
						case 0: // only gold, if any
							{
								flockcount = flockcountgold;
								flockmbs = flockmbsgold;
								if (g_Player().HasGoldSheep() == false)
								{
									flockcount += flockcountfree;
									flockmbs += flockmbsfree;
								}
							}
							break;
						case 1: // free sheep only
								flockcount = flockcountfree;
								flockmbs = flockmbsfree;
							break;
						case 2: // all sheep
								flockcount = flockcountfree + flockcountgold;
								flockmbs = flockmbsfree + flockmbsgold;
							break;
						};
						tmpstr << flockcount << ((g_Player().UsedSheepType() == 0 && g_Player().HasGoldSheep() == true) ? " gold sheep, " : " sheep, ") << flockmbs << "MB";
						((Hud::CStringStat *)spStats->Get( "all" ))->SetSample(tmpstr.str());

						const char *servername = ContentDownloader::Shepherd::serverName( false );
						if ( servername != NULL && servername[0] )
						{
							((Hud::CStringStat *)spStats->Get( "server" ))->SetSample( servername );
							((Hud::CStringStat *)spStats->Get( "server" ))->Visible( true );
						}
						else
							((Hud::CStringStat *)spStats->Get( "server" ))->Visible( false );
						

						Hud::CStringStat	*pTmp = (Hud::CStringStat *)spStats->Get( "transfers" );
						if( pTmp )
						{
							std::string serverStatus = g_NetworkManager->Status();
							if( serverStatus == "" )
								pTmp->Visible( false );
							else
							{
								pTmp->SetSample( serverStatus );
								pTmp->Visible( true );
							}
						}
						
						pTmp = (Hud::CStringStat *)spStats->Get( "loginstatus" );
						if( pTmp )
						{
							bool visible = true;
							
							const char *role = ContentDownloader::Shepherd::role();
							std::string loginstatus;
							if (role != NULL)
								loginstatus = role;
							else
								visible = false;
								
							if( loginstatus.empty() || loginstatus == "none" )
							{
								pTmp->SetSample( "Not logged in" );
							}
							else
							{
								std::stringstream loginstatusstr;
								loginstatusstr << "Logged in as " << ContentDownloader::SheepGenerator::nickName() << " (" << loginstatus << ")";
								pTmp->SetSample( loginstatusstr.str() );
							}
							
							pTmp->Visible( visible );
						}

						pTmp = (Hud::CStringStat *)spStats->Get( "bsurvivors" );
						if( pTmp )
						{
							std::stringstream survivors;

							survivors << "Survivors: median cut=" << g_PlayCounter().GetMedianCutSurvivors();
							
							if (m_SeamlessPlayback)
							survivors << ", dead end eliminator=" << g_PlayCounter().GetDeadEndCutSurvivors();

							pTmp->SetSample( survivors.str() );
							pTmp->Visible( true );
						}
						
						pTmp = (Hud::CStringStat *)spStats->Get( "deleted" );
						if( pTmp )
						{
							std::string deleted;
							if (ContentDownloader::Shepherd::PopOverflowMessage( deleted ) && (deleted != ""))
							{
								pTmp->SetSample( deleted );
								pTmp->Visible( true );
							} else
								pTmp->Visible( false );
						}

						pTmp = (Hud::CStringStat *)spStats->Get( "zconnerror" );
						if( pTmp )
						{
							if (m_ConnectionErrors.size() > 0)
							{
								std::string allConnectionErrors;
								for (size_t ii = 0; ii < m_ConnectionErrors.size(); ++ii)
									allConnectionErrors += m_ConnectionErrors.at(ii) + "\n";
								if (allConnectionErrors.size() > 0)
									allConnectionErrors.erase(allConnectionErrors.size() - 1);
								pTmp->SetSample( allConnectionErrors );
								pTmp->Visible( true );
							} else
								pTmp->Visible( false );
						}

						Hud::CTimeCountDownStat *pTcd = (Hud::CTimeCountDownStat *)spStats->Get( "svstat" );
						if( pTcd )
						{
							bool isnew = false;
							
							std::string dlState = ContentDownloader::Shepherd::downloadState( isnew );
							
							if ( isnew )
							{
								pTcd->SetSample( dlState );
								pTcd->Visible( true );
							}
						}

						//	Renderer stats.
						spStats = (Hud::spCStatsConsole)m_HudManager->Get( "renderstats" );
						((Hud::CIntCounter *)spStats->Get( "rendering" ))->SetSample( ContentDownloader::Shepherd::FramesRendering() );
						((Hud::CIntCounter *)spStats->Get( "totalframes" ))->SetSample( ContentDownloader::Shepherd::TotalFramesRendered() );
						
						Hud::CStringStat *batteryStat = ((Hud::CStringStat *)spStats->Get( "zbattery" ));
						
						if (batteryStat != NULL)
							batteryStat->SetSample( batteryStatus );
						
						if (m_CpuUsageTotal != -1 && m_CpuUsageES != -1)
						{
							std::stringstream temp;
							
							temp << " ES " << m_CpuUsageES << "%, total " << m_CpuUsageTotal << "% ";

							if ( ContentDownloader::Shepherd::RenderingAllowed() )
								temp << "(new rendering allowed)";
							else
								temp << "(new rendering blocked)";

							((Hud::CStringStat *)spStats->Get( "zzacpu" ))->SetSample( temp.str() );
						}

						pTcd = (Hud::CTimeCountDownStat *)spStats->Get( "countdown" );
						if( pTcd )
						{
							bool isnew = false;
							
							std::string renderState = ContentDownloader::Shepherd::renderState( isnew );
							
							if ( isnew )
							{
								pTcd->SetSample( renderState );
								pTcd->Visible( true );
							}
						}

						//	Finally render hud.
						m_HudManager->Render( g_Player().Renderer() );
						
						//	Update display events.
						g_Player().Display()->Update();
					}

					g_Player().EndDisplayFrame( displayUnit, drawn );
				}
				
				return true;

			}

			virtual bool HandleOneEvent( DisplayOutput::spCEvent &_event )
			{
				static const fp4 voteDelaySeconds = 1;

				if( _event->Type() == DisplayOutput::CEvent::Event_KEY )
					{
						DisplayOutput::spCKeyEvent spKey = static_cast<DisplayOutput::spCKeyEvent>( _event );
						switch( spKey->m_Code )
						{
								//	Vote for sheep.
							case	DisplayOutput::CKeyEvent::KEY_UP:
								if( m_pVoter != NULL && m_pVoter->Vote( g_Player().GetCurrentPlayingID(), true, voteDelaySeconds ) )
									m_HudManager->Add( "splash_pos", m_spSplashPos, voteDelaySeconds*0.9f );
								break;
							case	DisplayOutput::CKeyEvent::KEY_DOWN:
								if( m_pVoter != NULL && m_pVoter->Vote( g_Player().GetCurrentPlayingID(), false, voteDelaySeconds ) )
								{
									if (g_Settings()->Get( "settings.content.negvotedeletes", true ))
									{
										//g_Player().Stop();
										m_spCrossFade->Reset();
										m_HudManager->Add( "fade", m_spCrossFade, 1.5 );
									}

									m_HudManager->Add( "splash_pos", m_spSplashNeg, voteDelaySeconds*0.9f );
								}
								break;
								
								//	Repeat current sheep
							case	DisplayOutput::CKeyEvent::KEY_LEFT:
								g_Player().ReturnToPrevious();
								break;
								
								//  Force Next Sheep
							case	DisplayOutput::CKeyEvent::KEY_RIGHT:
								g_Player().SkipToNext();
								break;
								
								//	Repeat sheep
							case	DisplayOutput::CKeyEvent::KEY_F8:		
								g_Player().RepeatSheep();	
								break;
																
								//	OSD info.
								
								//	OSD info.
							case	DisplayOutput::CKeyEvent::KEY_F1:
								m_F1F4Timer.Reset();
								m_HudManager->Toggle( "helpmessage" );
								break;
							case	DisplayOutput::CKeyEvent::KEY_F2:
								m_F1F4Timer.Reset();
								m_HudManager->Toggle( "serverstats" );
								break;
							case	DisplayOutput::CKeyEvent::KEY_F3:
								m_F1F4Timer.Reset();
								m_HudManager->Toggle( "renderstats" );	
								break;
							case	DisplayOutput::CKeyEvent::KEY_F4:
								m_F1F4Timer.Reset();
								m_HudManager->Toggle( "displaystats" );	
								break;
								
							//	All other keys needs to be ignored, they are handled somewhere else...
							default:
							{
							    g_Log->Info( "Key event, ignoring" );
								return false;
							}
						}
						return true;
					}
				return false;
			}

			virtual const bool HandleEvents()
			{
				DisplayOutput::spCDisplayOutput	spDisplay = g_Player().Display();

				//	Handle events.
				DisplayOutput::spCEvent spEvent;
				while( spDisplay->GetEvent( spEvent ) )
				{
					if ( HandleOneEvent( spEvent ) == false )
						return false;
				}

				return true;
			}
			
			virtual std::string GetVersion()
			{
				return "";
			}
			
			virtual int GetACLineStatus()
			{
				return -1;
			}

			void SetUpdateAvailable(const std::string& verinfo)
			{	
				std::string message("New Electric Sheep ");
				
				message += verinfo;
				message += " is available.";
#ifdef MAC
				message += " Relaunch ES application or preference pane to update.";
#endif
				
				ContentDownloader::Shepherd::QueueMessage( message, 30.0 );
			}
};

#endif // CLIENT_H_INCLUDED
