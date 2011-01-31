#ifndef _WGL__H_
#define _WGL__H_

#ifdef	WIN32

#ifdef _DisplayGL_H_
#error "DisplayGL.h included before wgl.h!"
#endif

#include "../DisplayOutput.h"
#include "glee.h"

namespace DisplayOutput
{

/*
	CWindowsGL.
	Windows OpenGL output.
*/
class CWindowsGL : public CDisplayOutput
{
	HWND		m_WindowHandle;

	//	Only valid when running as a screensaver.
	bool		m_bScreensaver;
	bool		m_bPreview;

	public:
			CWindowsGL();
			virtual ~CWindowsGL();

			static LRESULT CALLBACK	wndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

			static char *Description()	{	return "Windows OpenGL display";	};
			virtual HWND	Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen );

			HWND	WindowHandle( void )	{	return m_WindowHandle;	};

			//	This is used when running as a screensaver, where the hwnd might already be provided.
			virtual bool	Initialize( HWND _hWnd, bool _bPreview );

			//
			virtual void	Title( const std::string &_title );
			virtual void	Update();

			void    SwapBuffers();
};

typedef	CWindowsGL	CDisplayGL;

};

#endif

#endif
