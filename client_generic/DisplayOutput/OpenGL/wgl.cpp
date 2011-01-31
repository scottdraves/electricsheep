#ifdef	WIN32

#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

#include "wgl.h"
#include "Log.h"

#include	"Exception.h"

namespace	DisplayOutput
{

/*
	CWindowsGL().

*/
CWindowsGL::CWindowsGL() : CDisplayOutput()
{
	m_WindowHandle = NULL;
	m_bScreensaver = false;
}

/*
	~CWindowsGL().

*/
CWindowsGL::~CWindowsGL()
{
}

/*
	wndProc().
	Handle all events, push them onto the eventqueue.
*/
LRESULT CALLBACK CWindowsGL::wndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT	ps;

	switch( msg )
    {
		/*case WM_PAINT:
			BeginPaint( hWnd, &ps );
			EndPaint( hWnd, &ps );
			break;*/

		/*case WM_SIZE:
			width = LOWORD(lParam);
			height = HIWORD(lParam);
        break;*/

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
				case VK_LEFT:	spEvent->m_Code = CKeyEvent::KEY_LEFT;	break;
				case VK_RIGHT:	spEvent->m_Code = CKeyEvent::KEY_RIGHT;	break;
				case VK_UP:		spEvent->m_Code = CKeyEvent::KEY_UP;	break;
				case VK_DOWN:	spEvent->m_Code = CKeyEvent::KEY_DOWN;	break;
				case VK_SPACE:	spEvent->m_Code = CKeyEvent::KEY_SPACE;	break;
				case VK_F1:		spEvent->m_Code = CKeyEvent::KEY_F1;	break;
				case VK_F2:		spEvent->m_Code = CKeyEvent::KEY_F2;	break;
				case VK_F3:		spEvent->m_Code = CKeyEvent::KEY_F3;	break;
				case VK_F4:		spEvent->m_Code = CKeyEvent::KEY_F4;	break;
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
			return DefWindowProc( hWnd, msg, wParam, lParam );
    }
    return 0;
}

/*
	createwindow().
	Creates a new window, _w * _h in size, optionally fullscreen.
*/
static HWND createwindow( uint32 _w, uint32 _h, const bool _bFullscreen )
{
	WNDCLASS   wndclass;
	HMODULE    hInstance = GetModuleHandle(NULL);
	RECT       windowRect;
	SetRect(&windowRect, 0, 0, _w, _h );

	ZeroMemory( &wndclass, sizeof(WNDCLASS) );

	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc   = (WNDPROC)CWindowsGL::wndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon (NULL, IDI_WINLOGO);
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = "Electricsheep";

	if( !RegisterClass (&wndclass) )
	{
		OutputDebugString("Failed to register window class");
		return 0;
	}

	if( _bFullscreen )
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
	}

	unsigned long exStyle;
	unsigned long style;
	if( _bFullscreen )
	{
		exStyle = WS_EX_APPWINDOW;
		style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	AdjustWindowRectEx( &windowRect, style, false, exStyle );
	HWND hWnd = CreateWindowEx( exStyle, "Electricsheep", "Electricsheep", style, 0, 0, _w, _h, NULL, NULL, hInstance, NULL );

	return hWnd;
}

/*
	Initialize().
	We already have a window from the screensaver, so store it and pass along all 0's to initialize
	a child window.
*/
bool	CWindowsGL::Initialize( HWND _hWnd, bool _bPreview )
{
	m_bScreensaver = true;
	m_bPreview = true;
	m_WindowHandle = _hWnd;

	HMODULE    hInstance = GetModuleHandle(NULL);

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)CWindowsGL::wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "Electricsheep";
	RegisterClass( &wc );

	if( _bPreview )
	{
		RECT rc;
		GetWindowRect( _hWnd, &rc );
		int32 cx = rc.right - rc.left;
		int32 cy = rc.bottom - rc.top;

		m_WindowHandle = CreateWindowEx( 0, "Electricsheep", "Preview", WS_CHILD | WS_VISIBLE, 0, 0, cx, cy, _hWnd, NULL, hInstance, NULL );

		if( m_WindowHandle == NULL )
			ThrowStr( "Unable to create window..." );

		m_Width = cx;
		m_Height = cy;

		g_Log->Info( "Screensaver preview (%dx%d)", cx, cy );

		//	Show cursor.
		ShowCursor( true );
	}
	else
	{
//		int32 cx = GetSystemMetrics( SM_CXSCREEN );
//		int32 cy = GetSystemMetrics( SM_CYSCREEN );
		RECT rc;
		GetWindowRect( _hWnd, &rc );
		m_Width = rc.right;
		m_Height = rc.bottom;

		//DWORD exstyle = 0;//WS_EX_TOPMOST;
		//DWORD style = WS_CHILD | WS_VISIBLE;

		m_WindowHandle = _hWnd;//CreateWindowEx( exstyle, "Electricsheep", "Saver", style, 0, 0, cx, cy, _hWnd, NULL, hInstance, NULL );

		if( m_WindowHandle == NULL )
			ThrowStr( "Unable to create window..." );

		g_Log->Info( "Screensaver (%dx%d)", m_Width, m_Height );

		//	Hide cursor.
		ShowCursor( false );
	}

	//	Set up some opengl stuff for the window we just created.
	//SetupGL();
}


/*
	Initialize().

*/
HWND CWindowsGL::Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen )
{
	uint32 w, h;

	//	Create window if it was not already created.
	if( m_WindowHandle == NULL )
	{
		uint32 w = _width;
		uint32 h = _height;

		if( _bFullscreen )
		{
			w = GetSystemMetrics( SM_CXSCREEN );
			h = GetSystemMetrics( SM_CYSCREEN );
		}

		g_Log->Info( "(%dx%d)", w, h );

		m_WindowHandle = createwindow( w, h, _bFullscreen );
		if( m_WindowHandle == 0 )
			ThrowStr( "Unable to create window..." );
	}

	//	Show or Hide cursor.
	ShowCursor( !_bFullscreen );

	//	Get window dimensions.
	RECT rect;
    GetClientRect( m_WindowHandle, &rect );
    m_Width = rect.right;
    m_Height = rect.bottom;

    ShowWindow( m_WindowHandle, SW_SHOW );
    SetForegroundWindow( m_WindowHandle );
    SetFocus( m_WindowHandle );

	return	m_WindowHandle;
}

//
void CWindowsGL::Title( const std::string &_title )
{
	SetWindowText( m_WindowHandle, _title.c_str() );
}

//
void	CWindowsGL::Update()
{
	MSG msg;

	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		if( GetMessage( &msg, NULL, 0, 0) <= 0 )   // error or WM_QUIT
		{
			printf( "Message to close\n" );
			fflush( stdout );
			g_Log->Info( "Message to close..." );
			m_bClosed = true;
			return;
		}

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
}

/*
*/
void    CWindowsGL::SwapBuffers()
{
}

};

#endif
