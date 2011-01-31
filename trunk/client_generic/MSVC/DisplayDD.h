#pragma once
#include "../msvc/BackBufDD.h"
#include "DisplayOutput.h"

namespace DisplayOutput
{

class CDisplayDD : public CDisplayOutput
{
	HWND		m_WindowHandle;
	HWND		m_ParentWindowHandle;

	HWND 		createwindow( uint32 _w, uint32 _h, const bool _bFullscreen );

	//	Only valid when running as a screensaver.
	bool		m_bScreensaver;
	bool		m_bPreview;

	static CBackBufDD	m_dd;

	public:
			CDisplayDD();
			virtual ~CDisplayDD();

			static LRESULT CALLBACK	wndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

			static char *Description()	{	return "Windows DirectDraw display";	};
			virtual HWND	Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen );

			virtual bool InitDD(HWND hWnd);
			HWND	WindowHandle( void )	{	return m_WindowHandle;	};

			//	This is used when running as a screensaver, where the hwnd might already be provided.
			virtual bool	Initialize( HWND _hWnd, bool _bPreview );
			CBackBufDD *GetBackBufferPtr()
			{
				return (CBackBufDD*)(&m_dd);
			}
			//
			virtual void	Title( const std::string &_title );
			virtual void	Update();

			virtual bool HasShaders() { return false; };
			void	SwapBuffers() {
			}
};

MakeSmartPointers( CDisplayDD );

};