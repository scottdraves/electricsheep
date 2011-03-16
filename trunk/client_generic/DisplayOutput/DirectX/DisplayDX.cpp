#ifdef	WIN32

#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdint.h>
#include <tchar.h>
#include <ctype.h>
#include <regstr.h>
#include <multimon.h>
#include <mmsystem.h>

#include "DisplayDX.h"
#include "Log.h"
#include "settings.h"

#include	"Exception.h"

namespace	DisplayOutput
{

bool		m_bWaitForInputIdle = false;
bool		g_bPreview = false;
static HWND		gl_hFocusWindow = NULL;

/*
	CDisplayDX().

*/
CDisplayDX::CDisplayDX(bool _blank, IDirect3D9 *_pIDirect3D9) : CDisplayOutput()
{
	memset(&m_PresentationParams, 0, sizeof(m_PresentationParams));
	m_WindowHandle = NULL;
	m_bScreensaver = false;
	m_DesiredScreenID = 0;
	m_bWaitForInputIdle = false;
	m_dwNumMonitors = 0;
	m_Shader20 = false;
	m_pDirect3DInstance = _pIDirect3D9;
	m_BlankUnused = _blank;
}

/*
	~CDisplayDX().

*/
CDisplayDX::~CDisplayDX()
{
	g_Log->Info( "~CDisplayDX()" );
}

/*
	EnumMonitors().

*/
void	CDisplayDX::EnumMonitors()
{
	DWORD iDevice = 0;
	DISPLAY_DEVICE dispdev = {0};
	DISPLAY_DEVICE dispdev2 = {0};
	DEVMODE devmode = {0};

	dispdev.cb = sizeof(dispdev);
	dispdev2.cb = sizeof(dispdev2);
	devmode.dmSize = sizeof(devmode);
	devmode.dmDriverExtra = 0;
	MonitorInfo	*pMonitorInfoNew;

	while( EnumDisplayDevices( NULL, iDevice, (DISPLAY_DEVICE*)&dispdev, 0 ) )
	{
		//	Ignore NetMeeting's mirrored displays.
		if( ( dispdev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER ) == 0 )
		{
			//	To get monitor info for a display device, call EnumDisplayDevices a second time,
			//	passing dispdev.DeviceName (from the first call) as the first parameter.
			EnumDisplayDevices( dispdev.DeviceName, 0, (DISPLAY_DEVICE *)&dispdev2, 0 );

			pMonitorInfoNew = &m_Monitors[ m_dwNumMonitors ];
			ZeroMemory( pMonitorInfoNew, sizeof(MonitorInfo) );
			lstrcpy( pMonitorInfoNew->strDeviceName, dispdev.DeviceString );
			lstrcpy( pMonitorInfoNew->strMonitorName, dispdev2.DeviceString );
			pMonitorInfoNew->iAdapter = NO_ADAPTER;

			if( dispdev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP )
			{
				EnumDisplaySettings( dispdev.DeviceName, ENUM_CURRENT_SETTINGS, &devmode );
				//if( dispdev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE )
				//{
					//	For some reason devmode.dmPosition is not always (0, 0) for the primary display, so force it.
					//pMonitorInfoNew->rcScreen.left = 0;
					//pMonitorInfoNew->rcScreen.top = 0;
				//}
				//else
				//{
					pMonitorInfoNew->rcScreen.left = devmode.dmPosition.x;
					pMonitorInfoNew->rcScreen.top = devmode.dmPosition.y;
				//}

				pMonitorInfoNew->rcScreen.right = pMonitorInfoNew->rcScreen.left + devmode.dmPelsWidth;
				pMonitorInfoNew->rcScreen.bottom = pMonitorInfoNew->rcScreen.top + devmode.dmPelsHeight;

				pMonitorInfoNew->hMonitor = MonitorFromRect( &pMonitorInfoNew->rcScreen, MONITOR_DEFAULTTONULL );
			}

			m_dwNumMonitors ++;
			if( m_dwNumMonitors == MAX_DISPLAYS )
				break;
		}
		iDevice++;
	}
}

void CDisplayDX::BlankUnusedMonitors(WNDCLASS &wnd, HWND hWnd, HINSTANCE hInstance)
{
	if (m_BlankUnused)
	{
		g_Log->Info( "Blanking unused monitors");
		RECT rc;
		DWORD dwStyle;

		dwStyle = WS_VISIBLE | WS_POPUP;

		for( DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
		{
			MonitorInfo	*pMonitorInfo;
			pMonitorInfo = &m_Monitors[ iMonitor ];
			if( pMonitorInfo->hMonitor == NULL )
				continue;

			rc = pMonitorInfo->rcScreen;
			if( iMonitor != m_DesiredScreenID )
				pMonitorInfo->hWnd = CreateWindowEx( WS_EX_TOPMOST, L"ElectricsheepWndClass", L"ES", dwStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hWnd, NULL, hInstance, NULL );

		}

		MONITORINFO monitorInfo;

		for( DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
		{
			MonitorInfo *pMonitorInfo = &m_Monitors[iMonitor];
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo( pMonitorInfo->hMonitor, &monitorInfo );

			g_Log->Info( "Monitor %d x1=%d y1=%d x2=%d y2=%d", iMonitor, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);

			if( iMonitor != m_DesiredScreenID )
			{
				g_Log->Info( "Blanking");
				pMonitorInfo->rcScreen = monitorInfo.rcMonitor;
				SetWindowPos( pMonitorInfo->hWnd, HWND_TOPMOST, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_NOACTIVATE );
			}
		}
	} else
		g_Log->Info( "Not Blanking unused monitors");
}

/*
	wndProc().
	Handle all events, push them onto the eventqueue.
*/
LRESULT CALLBACK CDisplayDX::wndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT	ps;

	switch( msg )
    {
        case WM_USER:
            //	All initialization messages have gone through.  Allow 500ms of idle time, then proceed with initialization.
            SetTimer( hWnd, 1, 500, NULL );
			g_Log->Info( "Starting 500ms preview timer" );
            break;

        case WM_TIMER:
            //	Initial idle time is done, proceed with initialization.
            m_bWaitForInputIdle = false;
			g_Log->Info( "500ms preview timer done" );
            KillTimer( hWnd, 1 );
            break;

		case WM_PAINT:
		{
			if (BeginPaint( hWnd, &ps ) != NULL)
			{
			/*if( g_bPreview )
			{
				RECT rc;
				GetClientRect( hWnd,&rc );
				FillRect( ps.hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH) );
			}*/

			EndPaint( hWnd, &ps );
			}
			return 0;
		}

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_KEYUP:
		{
			CKeyEvent *spEvent = new CKeyEvent();
			spEvent->m_bPressed = true;

			switch( wParam )
			{
				case VK_TAB:	spEvent->m_Code = CKeyEvent::KEY_TAB; break;
				case VK_LWIN:	spEvent->m_Code = CKeyEvent::KEY_LALT; break;
				case VK_MENU:	spEvent->m_Code = CKeyEvent::KEY_MENU; break;
				case VK_LEFT:	spEvent->m_Code = CKeyEvent::KEY_LEFT;	break;
				case VK_RIGHT:	spEvent->m_Code = CKeyEvent::KEY_RIGHT;	break;
				case VK_UP:		spEvent->m_Code = CKeyEvent::KEY_UP;	break;
				case VK_DOWN:	spEvent->m_Code = CKeyEvent::KEY_DOWN;	break;
				case VK_SPACE:	spEvent->m_Code = CKeyEvent::KEY_SPACE;	break;
				case 0x46:		spEvent->m_Code = CKeyEvent::KEY_F;		break;
				case VK_CONTROL:spEvent->m_Code = CKeyEvent::KEY_CTRL;	break;
				case VK_F1:		spEvent->m_Code = CKeyEvent::KEY_F1;	break;
				case VK_F2:		spEvent->m_Code = CKeyEvent::KEY_F2;	break;
				case VK_F3:		spEvent->m_Code = CKeyEvent::KEY_F3;	break;
				case VK_F4:		spEvent->m_Code = CKeyEvent::KEY_F4;	break;
				case VK_F8:		spEvent->m_Code = CKeyEvent::KEY_F8;	break;
				case VK_ESCAPE:	spEvent->m_Code = CKeyEvent::KEY_Esc;	break;
			}

			spCEvent e = spEvent;
			m_EventQueue.push( e );
		}
		break;

		case WM_LBUTTONUP:
		{
			CMouseEvent *spEvent = new CMouseEvent();
			spEvent->m_Code = CMouseEvent::Mouse_LEFT;
			spEvent->m_X = MAKEPOINTS( lParam ).x;
			spEvent->m_Y = MAKEPOINTS( lParam ).y;
			spCEvent e = spEvent;
			m_EventQueue.push( e );
		}
		break;

		case WM_RBUTTONUP:
		{
			CMouseEvent *spEvent = new CMouseEvent();
			spEvent->m_Code = CMouseEvent::Mouse_RIGHT;
			spEvent->m_X = MAKEPOINTS( lParam ).x;
			spEvent->m_Y = MAKEPOINTS( lParam ).y;
			spCEvent e = spEvent;
			m_EventQueue.push( e );
		}
		break;

		case WM_MOUSEMOVE:
		{
			CMouseEvent *spEvent = new CMouseEvent();
			spEvent->m_Code = CMouseEvent::Mouse_MOVE;

			spEvent->m_X = MAKEPOINTS( lParam ).x;
			spEvent->m_Y = MAKEPOINTS( lParam ).y;

			spCEvent e = spEvent;
			m_EventQueue.push( e );
		}
		break;

		case WM_POWERBROADCAST:
			switch( LOWORD( wParam ) )
			{
				case PBT_APMBATTERYLOW:
				case PBT_APMSUSPEND:
				{
					CPowerEvent *spEvent = new CPowerEvent();
					spCEvent e = spEvent;
					m_EventQueue.push( e );
				}
			}
		break;

		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
    }
    return 0;
}

UINT	CDisplayDX::GetAdapterOrdinal()
{
	if (m_pDirect3DInstance == NULL)
	{
		g_Log->Error( "Using default adapter for screen %d", m_DesiredScreenID );
		return D3DADAPTER_DEFAULT;
	}
	MONITORINFO monitorInfo;
	for( DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
	{
		if (iMonitor == m_DesiredScreenID)
		{
			MonitorInfo *pMonitorInfo = &m_Monitors[iMonitor];
			monitorInfo.cbSize = sizeof(MONITORINFO);
			if ( GetMonitorInfo( pMonitorInfo->hMonitor, &monitorInfo ) != 0 )
				for ( size_t iAdapter = 0; iAdapter < m_pDirect3DInstance->GetAdapterCount(); ++iAdapter )
				{
					if ( m_pDirect3DInstance->GetAdapterMonitor(iAdapter) == pMonitorInfo->hMonitor )
					{
						g_Log->Info( "Using adapter %d for screen %d", iAdapter, m_DesiredScreenID );
						return iAdapter;
					}
				}
		}
	}
	g_Log->Info( "Using default adapter for screen %d", m_DesiredScreenID );
	return D3DADAPTER_DEFAULT;
}

/*
*/
bool	CDisplayDX::InitDX9()
{
	if( m_pDirect3DInstance == NULL )
	{
		g_Log->Error( "Couldn't initialize Direct3D\nMake sure you have DirectX 9.0c or later installed." );
		return false;
	} else
		g_Log->Info( "Direct3D Initialized from old IDirect3D9 object" );


	memset( &m_PresentationParams, 0, sizeof(m_PresentationParams) );

	m_PresentationParams.BackBufferFormat = D3DFMT_X8R8G8B8;

	m_PresentationParams.Windowed = !m_bFullScreen;

	m_PresentationParams.BackBufferWidth  = m_Width;
	m_PresentationParams.BackBufferHeight = m_Height;
	m_PresentationParams.BackBufferCount  = 1;

	m_PresentationParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (g_Settings()->Get( "settings.player.vbl_sync", false ) == true)
		m_PresentationParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	else
		m_PresentationParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	m_PresentationParams.SwapEffect = D3DSWAPEFFECT_DISCARD;

	m_PresentationParams.EnableAutoDepthStencil = false;

	m_PresentationParams.MultiSampleType = D3DMULTISAMPLE_NONE;

    D3DCAPS9 caps;
    DWORD dwVertexProcessing = 0;
    m_pDirect3DInstance->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);
	if ( caps.VertexShaderVersion >= D3DVS_VERSION(2,0) )
		m_Shader20 = true;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
    {
        if ( caps.VertexShaderVersion < D3DVS_VERSION(1,1) )
        {
            dwVertexProcessing = D3DCREATE_MIXED_VERTEXPROCESSING;
			g_Log->Info( "DX: Mixed vertex processing" );
			m_Shader20 = false;
        }
        else
        {
            dwVertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			g_Log->Info( "DX: Hardware vertex processing" );
			m_Shader20 = true;

        } 
    }
    else
    {
        dwVertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		g_Log->Info( "DX: Software vertex processing" );
		m_Shader20 = false;
    }

	HRESULT devresult = m_pDirect3DInstance->CreateDevice( m_bFullScreen ? GetAdapterOrdinal() : D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, gl_hFocusWindow == NULL ? m_WindowHandle : gl_hFocusWindow, D3DCREATE_FPU_PRESERVE | dwVertexProcessing, &m_PresentationParams, &m_pDevice);
/*	UINT adapterid = 0;
	for (adapterid = 0; adapterid < m_pDirect3D->GetAdapterCount(); ++adapterid)
	{
		D3DADAPTER_IDENTIFIER9 adapterid9;
		m_pDirect3D->GetAdapterIdentifier(adapterid, 0, &adapterid9);
		if (strstr(adapterid9.Description, "PerfHUD") != 0)
			break;
	}
	HRESULT devresult = m_pDirect3D->CreateDevice( adapterid, D3DDEVTYPE_REF, m_WindowHandle, D3DCREATE_FPU_PRESERVE | dwVertexProcessing, &m_PresentationParams, &m_pDevice);*/
	if(devresult  != D3D_OK )
	{
		g_Log->Error( "Failed to create Direct3D 9 device" );
		switch (devresult)
		{
		case D3DERR_DEVICELOST:
			g_Log->Info( "CreateDevice returned D3DERR_DEVICELOST" );
			break;
		case D3DERR_INVALIDCALL:
			g_Log->Info( "CreateDevice returned D3DERR_INVALIDCALL" );
			break;
		case D3DERR_NOTAVAILABLE:
			g_Log->Info( "CreateDevice returned D3DERR_NOTAVAILABLE" );
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			g_Log->Info( "CreateDevice returned D3DERR_OUTOFVIDEOMEMORY" );
			break;
		default:
			g_Log->Info( "CreateDevice returned %X", devresult );
			break;
		};
		return false;
	}
	if (gl_hFocusWindow == NULL)
		gl_hFocusWindow = m_WindowHandle;
	return true;
}


/*
	createwindow().
	Creates a new window, _w * _h in size, optionally fullscreen.
*/
HWND CDisplayDX::createwindow( uint32 _w, uint32 _h, const bool _bFullscreen )
{
	m_bFullScreen = _bFullscreen;
	HMODULE    hInstance = GetModuleHandle(NULL);
	
	EnumMonitors();

	WNDCLASS   wndclass = {0};
	RECT       windowRect;
	SetRect( &windowRect, 0, 0, _w, _h );

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = (WNDPROC)CDisplayDX::wndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon (GetModuleHandle(NULL), MAKEINTRESOURCE(1));
	wndclass.hCursor       = NULL;//LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = L"ElectricsheepWndClass";

	static bool wndclassAlreadyRegistered = false;
	if( wndclassAlreadyRegistered == false && !RegisterClass (&wndclass) )
	{
		return 0;
	}
	wndclassAlreadyRegistered = true;

	/*if( _bFullscreen )
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= _w;
		dmScreenSettings.dmPelsHeight	= _h;
		dmScreenSettings.dmBitsPerPel	= 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if( ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL )
			OutputDebugString("Failed to change display settings");
	}*/

	unsigned long exStyle;
	unsigned long style;
	if( _bFullscreen )
	{
		exStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
		style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		style =  WS_OVERLAPPEDWINDOW | 
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	AdjustWindowRectEx( &windowRect, style, false, exStyle );
	int ww = windowRect.right - windowRect.left;
	int hh = windowRect.bottom - windowRect.top;
	HWND hWnd = CreateWindowEx( exStyle, L"ElectricsheepWndClass", L"Electricsheep", style, 0, 0, ww, hh, NULL, NULL, hInstance, NULL );
	MONITORINFO monitorInfo;
	for( DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
	{
		if (iMonitor == m_DesiredScreenID && _bFullscreen)
		{
			MonitorInfo *pMonitorInfo = &m_Monitors[iMonitor];
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo( pMonitorInfo->hMonitor, &monitorInfo );
			pMonitorInfo->rcScreen = monitorInfo.rcMonitor;
			SetWindowPos( hWnd, HWND_TOPMOST, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_NOACTIVATE );
		}
	}
	BlankUnusedMonitors(wndclass, hWnd, hInstance);
	return hWnd;
}

/*
	Initialize().
	We already have a window from the screensaver, so store it and pass along all 0's to initialize
	a child window.
*/
bool	CDisplayDX::Initialize( HWND _hWnd, bool _bPreview )
{
	m_bScreensaver = true;
	m_bPreview = _bPreview;
	m_WindowHandle = _hWnd;

	HMODULE    hInstance = GetModuleHandle( NULL );

	WNDCLASS wc = {0};
	
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = (WNDPROC)CDisplayDX::wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon (GetModuleHandle(NULL), MAKEINTRESOURCE(1));
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"ElectricsheepWndClass";
	RegisterClass( &wc );

	if( _bPreview )
	{
		g_bPreview = true;

		RECT rc;
		//GetWindowRect( _hWnd, &rc );
        GetClientRect( _hWnd, &rc );
		int32 cx = rc.right - rc.left;
		int32 cy = rc.bottom - rc.top;

		g_Log->Info( "rc: %d, %d", cx, cy );

		DWORD dwStyle = WS_VISIBLE | WS_CHILD;
		AdjustWindowRect( &rc, dwStyle, FALSE );
		m_WindowHandle = CreateWindow( L"ElectricsheepWndClass", L"Preview", dwStyle, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, _hWnd, NULL, hInstance, NULL );

		if( m_WindowHandle == NULL )
		{
			g_Log->Error( "CDisplayDX::Initialize unable to create window for preview" );
			//ThrowStr( "Unable to create window..." );
			return false;
		}

		m_Width = cx;
		m_Height = cy;

		g_Log->Info( "Screensaver preview (%dx%d)", cx, cy );

		// In preview mode, "pause" (enter a limited message loop) briefly before proceeding, so the display control panel knows to update itself.
		m_bWaitForInputIdle = true;

		// Post a message to mark the end of the initial group of window messages
		PostMessage( m_WindowHandle, WM_USER, 0, 0 );

		InvalidateRect( GetParent( _hWnd ), NULL, FALSE );    // Invalidate the hwnd so it gets drawn
		UpdateWindow( GetParent( _hWnd ) );
		UpdateWindow( GetParent( _hWnd ) );

		MSG msg;
		while( m_bWaitForInputIdle )
		{
			// If GetMessage returns FALSE, it's quitting time.
			if( !GetMessage( &msg, m_WindowHandle, 0, 0 ) )
			{
				// Post the quit message to handle it later
				PostQuitMessage(0);
				break;
			}

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		if( !InitDX9() )
			return false;

		//ShowWindow( _hWnd, SW_SHOW );
		//SetForegroundWindow( _hWnd );
		SetFocus( _hWnd );
		ShowCursor( true );
	}
	else
	{
//		int32 cx = GetSystemMetrics( SM_CXSCREEN );
//		int32 cy = GetSystemMetrics( SM_CYSCREEN );
		RECT rc;
		GetWindowRect( _hWnd, &rc );
		m_Width = rc.right - rc.left;
		m_Height = rc.bottom - rc.top;

		//DWORD exstyle = 0;//WS_EX_TOPMOST;
		//DWORD style = WS_CHILD | WS_VISIBLE;

		m_WindowHandle = _hWnd;//CreateWindowEx( exstyle, "Electricsheep", "Saver", style, 0, 0, cx, cy, _hWnd, NULL, hInstance, NULL );
		if( m_WindowHandle == NULL )
		{
			g_Log->Error( "CDisplayDX::Initialize unable to create window from _hWnd" );
			return false;
		}

		g_Log->Info( "Screensaver (%dx%d)", m_Width, m_Height );

		//	Hide cursor.
		ShowCursor( false );

		if( !InitDX9() )
			return false;
	}

	return true;
}

			//w = GetSystemMetrics( SM_CXVIRTUALSCREEN );
			//h = GetSystemMetrics( SM_CYVIRTUALSCREEN );
/*	SetWindowPos( m_WindowHandle, HWND_TOPMOST,
					GetSystemMetrics( SM_XVIRTUALSCREEN ),
					GetSystemMetrics( SM_YVIRTUALSCREEN ),
                    GetSystemMetrics( SM_CXVIRTUALSCREEN ),
                    GetSystemMetrics( SM_CYVIRTUALSCREEN ), SWP_SHOWWINDOW );*/


/*
	Initialize().

*/
HWND CDisplayDX::Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen )
{
	m_WindowHandle = createwindow( _width, _height, _bFullscreen );
	if( m_WindowHandle == 0 )
	{
		g_Log->Error( "CDisplayDX::Initialize createwindow returned 0" );
		return 0;
	}

	//	Show or Hide cursor.
	ShowCursor( !_bFullscreen );
	
	//	Get window dimensions.
	RECT rect;
    GetClientRect( m_WindowHandle, &rect );
    m_Width = rect.right - rect.left;
    m_Height = rect.bottom - rect.top;

    ShowWindow( m_WindowHandle, SW_SHOW );
    SetForegroundWindow( m_WindowHandle );
    SetFocus( m_WindowHandle );

	if( !InitDX9() )
		return false;

	return	m_WindowHandle;
}

//
void CDisplayDX::Title( const std::string &_title )
{
	SetWindowTextA( m_WindowHandle, _title.c_str() );
}

//
void	CDisplayDX::Update()
{
	MSG msg;

	/*if( m_bPreview )
	{
		g_Log->Info( "Invalidating" );
		InvalidateRect( m_WindowHandle, NULL, FALSE );    // Invalidate the hwnd so it gets drawn
		UpdateWindow( m_WindowHandle );
	}*/

	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		if( GetMessage( &msg, NULL, 0, 0) <= 0 )   // error or WM_QUIT
		{
			m_bClosed = true;
			return;
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
}

/*
*/
void	CDisplayDX::SwapBuffers()
{
}


};

#endif
