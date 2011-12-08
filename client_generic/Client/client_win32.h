#ifndef CLIENT_WIN32_H_INCLUDED
#define CLIENT_WIN32_H_INCLUDED

#ifndef WIN32
#error	This file is not supposed to be used for this platform...
#endif

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
//#include "../msvc/wxApp_main.h"
#include "base.h"
#include "ProcessForker.h"
#include "MathBase.h"
#include "Exception.h"
#include "Log.h"
#include "Player.h"
#include "storage.h"
#include "Settings.h"
#include "MonoInstance.h"
#if defined(WIN32) && defined(_MSC_VER)
#include "../msvc/msvc_fix.h"
#endif



/*
	CElectricSheep_Win32().
	Windows specific client code.
*/
class	CElectricSheep_Win32 : public CElectricSheep
{
	CMonoInstance	g_SingleInstanceObj;
	enum eScrMode
	{
		eNone,
		eConfig,							//	Forks the config user interface.
		ePassword,							//	Currently ignored, handled by windows anyway for XP>.
		ePreview,							//	Will ignore events, renders into small preview window.
		eSaver,								//	Full screensaver mode.
		eFullScreenStandalone,				//	Full screen
		eWindowed,							//	Windowed mode, will ignore mouse events from exiting.
		eWindowed_AllowMultipleInstances,	//	Same as eWindowed, but allows multiple instances.
	};

	//	Mode deduced from cmdline parsing.
	eScrMode m_ScrMode;
	IDirect3D9 * m_pD3D9;

	//	Previous mouse pos, for movement calcs.
	bool			m_bMouseUnknown;
	int32			m_MouseX, m_MouseY;

	bool			m_bAllowFKey;

	//	Grab a string from the registry.
	HRESULT RegGetString( HKEY hKey, LPCSTR szValueName, LPSTR * lpszResult )
	{
		#define	MAXBUF	4096

		//	Given a HKEY and value name returns a string from the registry.
		//	Upon successful return the string should be freed using free().
		//	eg. RegGetString(hKey, TEXT("my value"), &szString);

		DWORD dwType = 0, dwDataSize = 0, dwBufSize = 0;
		LONG lResult;

		//	In case we fail set the return string to null...
		if( lpszResult != NULL )
			*lpszResult = NULL;

		//	Check input parameters...
		if( hKey == NULL || lpszResult == NULL )
			return E_INVALIDARG;

		//	Get the length of the string in bytes (placed in dwDataSize)...
		lResult = RegQueryValueExA(hKey, szValueName, 0, &dwType, NULL, &dwDataSize );

		//	Check result and make sure the registry value is a string(REG_SZ)...
		if( lResult != ERROR_SUCCESS )
			return HRESULT_FROM_WIN32( lResult );
		else if( dwType != REG_SZ )
			return DISP_E_TYPEMISMATCH;

		//	Allocate memory for string - We add space for a null terminating character...
		dwBufSize = dwDataSize + (1 * sizeof(CHAR) );
		*lpszResult = (CHAR *)malloc( dwBufSize );

		if( *lpszResult == NULL )
			return E_OUTOFMEMORY;

		//	Now get the actual string from the registry...
		lResult = RegQueryValueExA( hKey, szValueName, 0, &dwType, (LPBYTE)*lpszResult, &dwDataSize );

		//	Check result and type again. If we fail here we must free the memory we allocated...
		if( lResult != ERROR_SUCCESS )
		{
			free( *lpszResult );
			return HRESULT_FROM_WIN32( lResult );
		}
		else if( dwType != REG_SZ )
		{
			free( *lpszResult );
			return DISP_E_TYPEMISMATCH;
		}

		//	We are not guaranteed a null terminated string from RegQueryValueEx so explicitly null terminate the returned string...
		(*lpszResult)[ (dwBufSize / sizeof(CHAR)) - 1 ] = '\0';

		return NOERROR;
	}

	public:
			CElectricSheep_Win32() : CElectricSheep(), g_SingleInstanceObj( "Global\\{"CLIENT_VERSION_PRETTY"}" ), m_bAllowFKey(false), m_pD3D9(NULL)
			{
				printf( "CElectricSheep_Win32()\n" );

				//	Windows spawns a screensaver process with idle priority, which will compete with forked generators.
				if( SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS ) )
					printf( "Changed priority class to normal\n" );
				else
					printf( "Failed to change priority class\n" );
			}

			virtual ~CElectricSheep_Win32()
			{
				if (m_pD3D9 != NULL)
				{
					m_pD3D9->Release();
				}
				if( m_ScrMode == eSaver || m_ScrMode == eFullScreenStandalone)
				{
					//BOOL bUnused;
					//SystemParametersInfo( SPI_SCREENSAVERRUNNING, FALSE, &bUnused, WM_SETTINGCHANGE );
				}
				//SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, TRUE, NULL, WM_SETTINGCHANGE );
			}

			//
			virtual const bool	Startup()
			{
				//	Check for multiple instances.

				m_ScrMode = eNone;

				//	Some very ugly win32 screensaver cmdline parsing.
				char *c = GetCommandLineA();
				if( *c=='\"' )
				{
					c++;
					while( *c!=0 && *c!='\"' )
						c++;
				}
				else
				{
					while( *c!=0 && *c!=' ')
						c++;
				}

				if( *c != 0 )
					c++;

				while( *c==' ' )
					c++;

				HWND hwnd = NULL;

				if( *c==0 )
				{
					m_ScrMode = eConfig;
					hwnd = NULL;
				}
				else
				{
					if( *c=='-' || *c=='/' )
						c++;

					if( *c=='p' || *c=='P' || *c=='l' || *c=='L' )
					{
						c++;
						while( *c==' ' || *c==':' )
							c++;

						hwnd = (HWND)atoi(c);
						m_ScrMode = ePreview;
					}
					else if( *c=='s' || *c=='S' )
					{
						//SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, NULL, WM_SETTINGCHANGE );
						m_ScrMode = eSaver;
					}
					else if( *c=='c' || *c=='C' )
					{
						c++;
						while( *c==' ' || *c==':' )
							c++;

						m_ScrMode = eConfig;
					}
					else if( *c=='a' || *c=='A' )
					{
						c++;
						while( *c==' ' || *c==':' )
							c++;

						hwnd = (HWND)atoi(c);
						m_ScrMode = ePassword;
					}
					//	Windowed test mode.
					else if( *c=='t' || *c=='T' )	
					{
						m_ScrMode = eWindowed;
					}
					//	Windowed test mode which allows multiple instances.
					else if( *c=='x' || *c=='X' )	
					{	
						m_ScrMode = eWindowed_AllowMultipleInstances;
						m_bAllowFKey = true;
					} 
					// fullscreen with working F keys
					else if (*c=='r' || *c=='R' )
					{
						//SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, NULL, WM_SETTINGCHANGE );
						m_ScrMode = eFullScreenStandalone;
						m_bAllowFKey = true;
					}
				}

				//	Check for multiple instances if we're not specifically asked not to.
				if( m_ScrMode != eWindowed_AllowMultipleInstances && m_ScrMode != eSaver && m_ScrMode != eConfig && m_ScrMode != eFullScreenStandalone)
				{
					if( g_SingleInstanceObj.IsAnotherInstanceRunning() )
					{
						//return false; // we could also exit here to prevent second instance
						m_MultipleInstancesMode = true;
					}
				} else
				{
					//	Instance check taken care of so revert back to eWindowed.
					if ( g_SingleInstanceObj.IsAnotherInstanceRunning() == false)
					{
						if (m_ScrMode != eSaver && m_ScrMode != eConfig && m_ScrMode != eFullScreenStandalone)
							m_ScrMode = eWindowed; // treat first instance normally
					}
					else
						m_MultipleInstancesMode = true;
				}

				char szPath[ MAX_PATH ];
				if( SUCCEEDED( SHGetFolderPathA( NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath ) ) )
				{
					PathAppendA( szPath, "\\ElectricSheep\\" );
					m_AppData = szPath;
				}

				//	Get the application installation directory from the registry. (stored by the installer)
				m_WorkingDir = ".\\";

				HKEY key;
				if( !RegOpenKeyA( HKEY_LOCAL_MACHINE, "SOFTWARE\\ElectricSheep", &key ) )
				{
					LPSTR temp;
					if( RegGetString( key, "InstallDir", &temp ) == NOERROR )
					{
						m_WorkingDir = temp;
						free( temp );
					}
                    RegCloseKey(key);
				}
				//	If the exe is renamed to .scr, the path lacks trailing slashes for some bizarre reason...
				size_t len = m_WorkingDir.size();
				if( m_WorkingDir[ len - 1 ] != '\\' )
				{
					m_WorkingDir.append( "\\" );
				}
				
				if (InitStorage(m_MultipleInstancesMode) == false)
				{
					MessageBox(NULL, L"Unable to initialize scripts. Try reinstalling.", L"Error!", MB_OK | MB_ICONERROR);
					return false;
				}
				
				if (g_SingleInstanceObj.IsAnotherInstanceRunning() == false)
					AttachLog();

                char szLogFileName[ MAX_PATH ] = "";

                //  Try to open the debug-report...
                if( GetModuleFileNameA( NULL, szLogFileName, MAX_PATH ) )
                {
                    LPSTR lpszDot;

                    // Look for the '.' before the "EXE" extension.  Replace the extension with "RPT".
                    if(( lpszDot = strrchr( szLogFileName, '.' ) ) )
                    {
                        lpszDot++;                          //  Advance past the '.'
                        strcpy( lpszDot, "RPT" );    // "RPT" -> "Report"
                    }
                    else
                        strcat( szLogFileName, ".RPT" );
                }
                else if( GetWindowsDirectoryA( szLogFileName, MAX_PATH ) )
                {
                        strcat( szLogFileName, "EXCHNDL.RPT" );
                }

                //  Append each line into the logfile.
                std::string line;
                std::ifstream rptfile( szLogFileName );
                if( rptfile.is_open() )
                {
                    while( !rptfile.eof() )
                    {
                        getline( rptfile, line );
                        g_Log->Fatal( line.c_str() );
                    }

                    rptfile.close();

                    //  And finally delete the report file.
                    if( remove( szLogFileName ) != 0 )
                        g_Log->Warning( "Failed to remove .rpt file!" );
                }

				std::string tmp = "Working dir: " + m_WorkingDir;
				g_Log->Info( tmp.c_str() );
				g_Log->Info( "Commandline: %s", GetCommandLineA() );

				_chdir( m_WorkingDir.c_str() );

				//	Run gui.
				if( m_ScrMode == eConfig )
				{
					g_Log->Info( "Running config." );
//#ifdef _DEBUG
//					#pragma comment(lib, "wxbase29ud.lib")
//					#pragma comment(lib, "wxmsw29ud_core.lib")
//					#pragma comment(lib, "wxpngd.lib")
//					#pragma comment(lib, "wxzlibd.lib")
//#else
//					#pragma comment(lib, "wxbase29u.lib")
//					#pragma comment(lib, "wxmsw29u_core.lib")
//					#pragma comment(lib, "wxpng.lib")
//					#pragma comment(lib, "wxzlib.lib")
//#endif
//					#pragma comment(lib, "comctl32.lib")
//					#pragma comment(lib, "rpcrt4.lib")
//					
//					wxApp::SetInstance(new wxWidgetsApp());
//					::wxEntry( GetModuleHandle(NULL) );

					
					SHELLEXECUTEINFOA sei = {0};
					sei.cbSize = sizeof(sei);
					sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS;
					sei.hwnd = NULL;
					if (IsUserAnAdmin())
						sei.lpVerb = "open";
					else
						sei.lpVerb = "runas";
					sei.lpFile = "settingsgui.exe";
					sei.lpParameters = NULL;
					sei.lpDirectory = m_WorkingDir.c_str();
					sei.nShow = SW_SHOWNORMAL;
					if (ShellExecuteExA(&sei) == TRUE)
					{
						WaitForSingleObject(sei.hProcess, INFINITE);
						CloseHandle(sei.hProcess);
					}
					m_bConfigMode = true;

					return false;
				}

				//	Exit if we're not supposed to render anything...
				if( m_ScrMode != eSaver &&
					m_ScrMode != eFullScreenStandalone &&
					m_ScrMode != ePreview &&
					m_ScrMode != eWindowed &&
					m_ScrMode != eWindowed_AllowMultipleInstances)
					return false;

				//	A window was provided, let's use it.
				if( hwnd )
					g_Player().SetHWND( hwnd );

				//	To prevent ctrl+alt+del stuff.
				//#warning NOTE (Keffo#1#): This is just annoying until all bugs are gone!
				if( m_ScrMode == eSaver || m_ScrMode == eFullScreenStandalone)
				{
					//BOOL bUnused;
					//SystemParametersInfo( SPI_SCREENSAVERRUNNING, TRUE, &bUnused, WM_SETTINGCHANGE );
				}

				//	User wants windowed?
				if( m_ScrMode == eWindowed || m_ScrMode == eWindowed_AllowMultipleInstances)
					g_Player().Fullscreen( false );

				uint32 monnum = g_Settings()->Get( "settings.player.screen", 0 );
				
				m_pD3D9 = Direct3DCreate9( D3D_SDK_VERSION );
				
				if ( g_Player().AddDisplay(g_Settings()->Get( "settings.player.screen", 0 ), m_pD3D9,
					g_Settings()->Get( "settings.player.MultiDisplayMode", 0 ) == CPlayer::kMDSingleScreen && m_ScrMode != eFullScreenStandalone  && m_ScrMode != eWindowed
					) == false)
				{
					bool foundfirstmon = false;
					g_Log->Error( "AddDisplay failed for screen %d", monnum );
					g_Log->Info( "Trying to autodetect usable monitors..." );
					monnum = 0;
					while (monnum < 9)
					{
						g_Log->Info( "Trying monitor %d", monnum );
						if (g_Player().AddDisplay( monnum, m_pD3D9 ) == true)
						{
							foundfirstmon = true;
							g_Log->Info( "Monitor %d ok", monnum );
							break;
						}
						else
						{
							g_Log->Error( "Monitor %d failed", monnum );
							++monnum;
						}
					}
					if (foundfirstmon == false)
						return false;
				} else
					g_Log->Info( "AddDisplay succeeded for screen %d", g_Settings()->Get( "settings.player.screen", 0 ));
				if ( m_ScrMode != eWindowed && m_ScrMode != eWindowed_AllowMultipleInstances && g_Settings()->Get( "settings.player.MultiDisplayMode", 0 ) != CPlayer::kMDSingleScreen)
					for ( DWORD dw = ( monnum + 1 ); dw < g_Player().Display()->GetNumMonitors(); ++dw )
					{
						if ( g_Player().AddDisplay( dw, m_pD3D9 ) == true )
							g_Log->Info( "AddDisplay succeeded for screen %d", dw );
						else
							g_Log->Error( "AddDisplay failed for screen %d", dw );
					}
				//
				if( CElectricSheep::Startup() == false )
					return false;

				//	Reset mouse calcs.
				m_bMouseUnknown = true;

				return true;
			}
			
			virtual bool HandleOneEvent( DisplayOutput::spCEvent & _event )
			{
				//	Handle events.
				if( _event->Type() == DisplayOutput::CEvent::Event_Power )
				{
					g_Log->Info( "Power broadcast event, closing..." );
					return false;
				}
				else
				//	Key events.
				if( _event->Type() == DisplayOutput::CEvent::Event_KEY )
				{
					DisplayOutput::spCKeyEvent spKey = static_cast<DisplayOutput::spCKeyEvent>( _event );

					switch( spKey->m_Code )
					{
						case	DisplayOutput::CKeyEvent::KEY_Esc:
								if (m_ScrMode != eFullScreenStandalone) // esc exits windowed and screensaver mode
									return false;
								else
								{
									::Base::RecreateProcess(std::string("-X"));
									return false;
								}
								break;

						case	DisplayOutput::CKeyEvent::KEY_F:	
								if (m_bAllowFKey == true)
								{
									if (m_ScrMode == eWindowed || m_ScrMode == eWindowed_AllowMultipleInstances)
									{
										// restart or change display here
										::Base::RecreateProcess(std::string("-R"));
										return false;
									} else
									{
										::Base::RecreateProcess(std::string("-X"));
										return false;
									}
								}
							break;
						case DisplayOutput::CKeyEvent::KEY_TAB:
							if (m_ScrMode != eWindowed && m_ScrMode != eWindowed_AllowMultipleInstances && m_ScrMode != eFullScreenStandalone)
								return false;
							break;
						case DisplayOutput::CKeyEvent::KEY_LALT:
						case DisplayOutput::CKeyEvent::KEY_MENU:
						case DisplayOutput::CKeyEvent::KEY_CTRL:
							break;

						//	All other keys close...
						default:
						{
							uint32 x = spKey->m_Code;
							g_Log->Info( "Key event, closing" );
							return false;
						}
					}
					return true;
				}
				else
				//	Process mouse events (ignored in eStandalone mode).
				if( _event->Type() == DisplayOutput::CEvent::Event_Mouse )
				{
					
					if( m_ScrMode != eWindowed 
					&& m_ScrMode != eWindowed_AllowMultipleInstances 
					&& m_ScrMode != ePreview
					&& m_ScrMode != eFullScreenStandalone )
					{
						DisplayOutput::spCMouseEvent spMouse = static_cast<DisplayOutput::spCMouseEvent>( _event );

						if( spMouse->m_Code == DisplayOutput::CMouseEvent::Mouse_MOVE )
						{
							fp4 dx = (fp4)spMouse->m_X - (fp4)m_MouseX;
							fp4 dy = (fp4)spMouse->m_Y - (fp4)m_MouseY;
							fp4	dist = dx*dx + dy*dy;

							//	Store for next run.
							m_MouseX = spMouse->m_X;
							m_MouseY = spMouse->m_Y;

							if( !m_bMouseUnknown )
							{
								//	Exit if mouse moved > 20 pixels...
								if( dist > 20*20 )
								{
									g_Log->Info( "Mouse moved too far(%f), closing...", Base::Math::Sqrt(dist) );
									return false;
								}
							}
							else g_Log->Info( "(ignored) Mouse move (%f)", Base::Math::Sqrt( dist ) );

							m_bMouseUnknown = false;
							return true;
						}
						else
						{
							//	All other mouse events exit.
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
					{
						if (CElectricSheep::HandleOneEvent( spEvent ) == false)
							return false;
					}
				}
				
				return true;
			}
			//
			const bool Update()
			{
				g_Player().Framerate( m_CurrentFps );

				if( !CElectricSheep::Update() )
					return false;

				DisplayOutput::spCDisplayOutput	spDisplay = g_Player().Display();

				//	We ignore events in preview mode.
				if( m_ScrMode == ePreview )
				{
					spDisplay->ClearEvents();
					return true;
				}

				static const fp4 voteDelaySeconds = 1;
				return HandleEvents();
			}

	virtual std::string GetVersion()
	{
		return CLIENT_VERSION;
	}

	virtual int GetACLineStatus()
	{
		SYSTEM_POWER_STATUS systemPowerStatus = {0};
		if (GetSystemPowerStatus(&systemPowerStatus) != 0)
		{
			if ((systemPowerStatus.BatteryFlag && 128 != 0) || (systemPowerStatus.BatteryFlag && 255 != 0))
				return -1; // no battery
			return systemPowerStatus.ACLineStatus;
		}
		return -1;
	}
};


#endif // CLIENT_H_INCLUDED
