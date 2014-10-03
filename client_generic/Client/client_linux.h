#ifndef CLIENT_LINUX_H_INCLUDED
#define CLIENT_LINUX_H_INCLUDED

#ifdef WIN32
#error	This file is not supposed to be used for this platform...
#endif

#include <string>
#include "base.h"
#include "MathBase.h"
#include "Exception.h"
#include "Log.h"
#include "Player.h"
#include "SimplePlaylist.h"
#include "lua_playlist.h"
#include "Timer.h"
#include "storage.h"
#include "Settings.h"

/*
	CElectricSheep_Linux().
	Linux specific client code.
*/
class	CElectricSheep_Linux : public CElectricSheep
{
  /*	std::vector<uint32> m_glContextList;*/

	public:
			CElectricSheep_Linux() : CElectricSheep()
			{
				printf( "CElectricSheep_Linux()\n" );
			}

			//
			virtual bool	Startup()
			{
				using namespace DisplayOutput;

				printf( "Startup()\n" );

				m_AppData = std::string(getenv("HOME"))+"/.electricsheep/";
				m_WorkingDir = SHAREDIR;

				InitStorage();
				AttachLog();

				std::string tmp = "Working dir: " + m_WorkingDir;
				g_Log->Info( tmp.c_str() );

				//	Run gui.

				g_Player().AddDisplay(g_Settings()->Get( "settings.player.screen", 0 ));

				//if( true )
				{
					g_Log->Info( "Running config..." );
					//g_Settings()->Storage()->ConfigUI( m_WorkingDir + "Scripts/config.lua" );
				}

				if( CElectricSheep::Startup() == false )
					return false;

				//	Start downloader.	This should be moved to client.h!
				//g_Log->Info( "Starting downloader..." );
				//g_ContentDownloader().Startup( false /*m_ScrMode == ePreview*/ );	//	Removed for 2.7b11...

				return true;
			}

			virtual bool HandleEvents()
			{
				DisplayOutput::spCDisplayOutput	spDisplay = g_Player().Display();

				//	Handle events.
				DisplayOutput::spCEvent spEvent;
				while( spDisplay->GetEvent( spEvent ) )
				{
					if ( HandleOneEvent( spEvent ) == false )
					{
						if (CElectricSheep::HandleOneEvent( spEvent ) == false)
							return false;
					}
				}
				
				return true;
			}


			//
			bool Update()
			{
			  using namespace DisplayOutput;

				g_Player().Framerate( m_CurrentFps );

				if( !CElectricSheep::Update() )
					return false;

				DisplayOutput::spCDisplayOutput	spDisplay = g_Player().Display();

				//	Update display events.
				spDisplay->Update();

				static const fp4 voteDelaySeconds = 1;



				//	Handle events.
				spCEvent spEvent;
				while( spDisplay->GetEvent( spEvent ) )
				{
					//	Key events.
					if( spEvent->Type() == CEvent::Event_KEY )
					{
						spCKeyEvent spKey = static_cast<spCKeyEvent>( spEvent );

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

						case CKeyEvent::KEY_Esc: spDisplay->Close(); break;

							//	All other keys close...
							  // NO !!!
							default:
							  //	   spDisplay->Close();
							  break;
						}
					}
				}

				return true;
				return HandleEvents();

			}


			//
			virtual void Shutdown()
			{
				try
				{
					printf( "CElectricSheep_Linux::Shutdown()\n" );
                    CElectricSheep::Shutdown();
				}
				catch( Base::CException &_e )
				{
					_e.ReportCatch();
				}
			}

			/* gf: try
			void AddGLContext( uint32 _glContext )
			{					
				if ( g_Player().Display() == NULL )
				{
					m_glContextList.push_back( _glContext );
				}
				else
				{
					g_Player().AddDisplay( _glContext );
				}
				} */
		


};

#endif // CLIENT_H_INCLUDED
