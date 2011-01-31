#ifdef	WIN32
#ifdef _MSC_VER

#include <windows.h>
#include <stdio.h>
#include "DisplayDD.h"
#include "Log.h"

namespace	DisplayOutput
{
bool		m_bWaitForInputIdleDS = false;
CBackBufDD CDisplayDD::m_dd;
CDisplayDD::CDisplayDD() : CDisplayOutput()
{
	m_WindowHandle = NULL;
	m_ParentWindowHandle = NULL;
	m_bScreensaver = false;
	m_bWaitForInputIdleDS = false;
}


CDisplayDD::~CDisplayDD()
{
}

bool CDisplayDD::InitDD(HWND hWnd)
{
	if (m_bFullScreen)
	{
		return m_dd.SetExclusive(hWnd, hWnd, true);
	}
	else
	{
		if (m_bPreview)
		{
			if (m_dd.Create(hWnd, m_ParentWindowHandle) == FALSE || m_dd.CreateSurface(m_Width, m_Height) == FALSE)
			{
				g_Log->Info(m_dd.GetLastErrorString());
				return false;
			} 
		}
		else
		if (m_dd.Create(hWnd, m_ParentWindowHandle) == FALSE || m_dd.CreateSurface(m_Width, m_Height) == FALSE)
		{
			g_Log->Info(m_dd.GetLastErrorString());
			return false;
		}
	}
	return true;
}

LRESULT CALLBACK CDisplayDD::wndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
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
            m_bWaitForInputIdleDS = false;
			g_Log->Info( "500ms preview timer done" );
            KillTimer( hWnd, 1 );
            break;

		case WM_PAINT:
		{
			if (BeginPaint( hWnd, &ps ) != NULL)
				EndPaint( hWnd, &ps );
			return 0;
		}

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			m_dd.Destroy();
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
		case WM_SIZE:
			{
				RECT rc;
				GetClientRect( hWnd, &rc );
				if (!m_dd.IsExclusive())
					m_dd.CreateSurface(rc.right - rc.left, rc.bottom - rc.top);
			}
		break;

		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
    }
    return 0;
}


HWND CDisplayDD::createwindow( uint32 _w, uint32 _h, const bool _bFullscreen )
{
	m_bFullScreen = _bFullscreen;
	HMODULE    hInstance = GetModuleHandle(NULL);

	if( _bFullscreen )
	{
		WNDCLASS    cls = {0};

		cls.hCursor        = NULL;//LoadCursor( NULL, IDC_ARROW );
		cls.hIcon          = LoadIcon (GetModuleHandle(NULL), MAKEINTRESOURCE(1));
		cls.lpszMenuName   = NULL;
		cls.lpszClassName  = TEXT("ElectricsheepWndClass");
		cls.hbrBackground  = (HBRUSH) GetStockObject( BLACK_BRUSH );
		cls.hInstance      = hInstance;
		cls.style          = CS_VREDRAW | CS_HREDRAW;
		cls.lpfnWndProc    = (WNDPROC)CDisplayDD::wndProc;
		cls.cbWndExtra     = 0;
		cls.cbClsExtra     = 0;
		RegisterClass( &cls );

		RECT rc = {0};
		DWORD dwStyle;

		dwStyle = WS_VISIBLE | WS_POPUP;
		HWND hWnd = CreateWindowEx( WS_EX_TOPMOST, L"ElectricsheepWndClass", L"Electricsheep", dwStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL );
		m_ParentWindowHandle = hWnd;
		return hWnd;
	}

	WNDCLASS   wndclass;
	RECT       windowRect;
	SetRect( &windowRect, 0, 0, _w, _h );

	ZeroMemory( &wndclass, sizeof(WNDCLASS) );

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = (WNDPROC)CDisplayDD::wndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon (GetModuleHandle(NULL), MAKEINTRESOURCE(1));
	wndclass.hCursor       = NULL;//LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = L"ElectricsheepWndClass";

	if( !RegisterClass (&wndclass) )
	{
		g_Log->Info("Failed to register window class");
		return 0;
	}

	unsigned long exStyle;
	unsigned long style;

	exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	AdjustWindowRectEx( &windowRect, style, false, exStyle );
	_w = windowRect.right - windowRect.left;
	_h = windowRect.bottom - windowRect.top;
	HWND hWnd = CreateWindowEx( exStyle, L"ElectricsheepWndClass", L"Electricsheep", style, 0, 0, _w, _h, NULL, NULL, hInstance, NULL );
	m_ParentWindowHandle = hWnd;
	return hWnd;
}

bool	CDisplayDD::Initialize( HWND _hWnd, bool _bPreview )
{
	m_bScreensaver = true;
	m_bPreview = _bPreview;

	HMODULE    hInstance = GetModuleHandle( NULL );

	WNDCLASS wc = {0};

	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = (WNDPROC)CDisplayDD::wndProc;
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
		RECT rc;
		//GetWindowRect( _hWnd, &rc );
        GetClientRect( _hWnd, &rc );
		int32 cx = rc.right - rc.left;
		int32 cy = rc.bottom - rc.top;

		g_Log->Info( "rc: %d, %d", cx, cy );

		DWORD dwStyle = WS_VISIBLE | WS_CHILD;
		AdjustWindowRect( &rc, dwStyle, FALSE );
		m_ParentWindowHandle = _hWnd;
		m_WindowHandle = CreateWindow( L"ElectricsheepWndClass", L"Preview", dwStyle, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, _hWnd, NULL, hInstance, this );

		if( m_WindowHandle == NULL )
			return false;

		m_Width = cx;
		m_Height = cy;

		g_Log->Info( "Screensaver preview (%dx%d)", cx, cy );

		// In preview mode, "pause" (enter a limited message loop) briefly before proceeding, so the display control panel knows to update itself.
		m_bWaitForInputIdleDS = true;

		// Post a message to mark the end of the initial group of window messages
		PostMessage( m_WindowHandle, WM_USER, 0, 0 );

		InvalidateRect( GetParent( _hWnd ), NULL, FALSE );    // Invalidate the hwnd so it gets drawn
		UpdateWindow( GetParent( _hWnd ) );
		UpdateWindow( GetParent( _hWnd ) );
		MSG msg;
		while( m_bWaitForInputIdleDS )
		{
			if( !GetMessage( &msg, m_WindowHandle, 0, 0 ) )
			{
				PostQuitMessage(0);
				break;
			}

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		InitDD( _hWnd );
		SetFocus( _hWnd );
		ShowCursor( true );
	}
	else
	{
		RECT rc;
		GetWindowRect( _hWnd, &rc );
		m_Width = rc.right;
		m_Height = rc.bottom;

		m_WindowHandle = _hWnd;
		m_ParentWindowHandle = _hWnd;
		if( m_WindowHandle == NULL )
			return false;

		g_Log->Info( "Screensaver (%dx%d)", m_Width, m_Height );

		//	Hide cursor.
		ShowCursor( false );

		InitDD( m_WindowHandle );
	}

	return true;
}


HWND CDisplayDD::Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen )
{
	m_WindowHandle = createwindow( _width, _height, _bFullscreen );
	m_ParentWindowHandle = m_WindowHandle;
	if( m_WindowHandle == 0 )
		return 0;

	//	Show or Hide cursor.
	ShowCursor( !_bFullscreen );

	if (!_bFullscreen)
	{
		RECT rcClient, rcWindow;
		POINT ptDiff;
		GetClientRect(m_WindowHandle, &rcClient);
		GetWindowRect(m_WindowHandle, &rcWindow);
		ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
		ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
		MoveWindow(m_WindowHandle, rcWindow.left, rcWindow.top, _width + ptDiff.x, _height + ptDiff.y, TRUE);
	}
	//	Get window dimensions.
	RECT rect;
    GetClientRect( m_WindowHandle, &rect );
    m_Width = rect.right;
    m_Height = rect.bottom;

    ShowWindow( m_WindowHandle, SW_SHOW );
    SetForegroundWindow( m_WindowHandle );
    SetFocus( m_WindowHandle );

	InitDD(m_WindowHandle);
	
	if (m_Width == 0 && m_Height == 0)
	{
		m_Width = m_dd.GetSize().cx;
		m_Height = m_dd.GetSize().cy;
	}

	return	m_WindowHandle;
}

//
void CDisplayDD::Title( const std::string &_title )
{
	SetWindowTextA( m_WindowHandle, _title.c_str() );
}

//
void	CDisplayDD::Update()
{
	MSG msg;

	//if( m_bPreview )
	//{
	//	//g_Log->Info( "Invalidating" );
	//	InvalidateRect( m_WindowHandle, NULL, FALSE );    // Invalidate the hwnd so it gets drawn
	//	UpdateWindow( m_WindowHandle );
	//}

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

};

#endif
#endif