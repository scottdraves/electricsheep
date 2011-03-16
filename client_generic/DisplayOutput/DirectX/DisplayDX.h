#ifndef _DISPLAYDX__H_
#define _DISPLAYDX__H_

#ifdef	WIN32

#include	<d3d9.h>
#include	<d3dx9.h>

#include "DisplayOutput.h"

namespace DisplayOutput
{

#define MAX_DISPLAYS 9
#define NO_ADAPTER 0xffffffff
#define NO_MONITOR 0xffffffff

//	Structure for holding information about a monitor.
struct MonitorInfo
{
	TCHAR          strDeviceName[ 128 ];
	TCHAR          strMonitorName[ 128 ];
	HMONITOR       hMonitor;
	RECT           rcScreen;
	DWORD          iAdapter;	// Which D3DAdapterInfo corresponds to this monitor
	HWND           hWnd;

	// Error message state
	FLOAT          xError;
	FLOAT          yError;
	FLOAT          widthError;
	FLOAT          heightError;
	FLOAT          xVelError;
	FLOAT          yVelError;
};


/*
	CDisplayDX.
	Windows DirectX output.
*/
class CDisplayDX : public CDisplayOutput
{
	bool m_BlankUnused;
	bool m_Shader20;
	D3DPRESENT_PARAMETERS	m_PresentationParams;
	IDirect3DDevice9		*m_pDevice;

	HWND					m_WindowHandle;
	uint32					m_DesiredScreenID;

	IDirect3D9 *m_pDirect3DInstance;

	void		EnumMonitors();
	HWND 		createwindow( uint32 _w, uint32 _h, const bool _bFullscreen );
	MonitorInfo	m_Monitors[ MAX_DISPLAYS ];
	DWORD		m_dwNumMonitors;

	//	Only valid when running as a screensaver.
	bool		m_bScreensaver;
	bool		m_bPreview;

	bool		m_bFullscreenSwitch;
	bool		m_bStartedFullscreen;

	bool	InitDX9();

	public:
			CDisplayDX(bool _blank, IDirect3D9 *_pIDirect3D9);
			virtual ~CDisplayDX();

			virtual DWORD	GetNumMonitors() { return m_dwNumMonitors; }
			void BlankUnusedMonitors(WNDCLASS &wnd, HWND hWnd, HINSTANCE hInstance);

			static LRESULT CALLBACK	wndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

			static char *Description()	{	return "Windows DirectX display";	};
			virtual HWND	Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen );

			UINT	GetAdapterOrdinal();
			HWND	WindowHandle( void )	{	return m_WindowHandle;	};
			IDirect3DDevice9	*Device()	{	return m_pDevice;		};
			D3DPRESENT_PARAMETERS PresentParameters() { return m_PresentationParams; };

			void	SetScreen( const uint32 _screen )	{	m_DesiredScreenID = _screen;	};

			//	This is used when running as a screensaver, where the hwnd might already be provided.
			virtual bool	Initialize( HWND _hWnd, bool _bPreview );

			//
			virtual void	Title( const std::string &_title );
			virtual void	Update();

			virtual bool HasShaders() { return m_Shader20; };
			void	SwapBuffers();
};

MakeSmartPointers( CDisplayDX );

};

#endif

#endif
