#ifndef GLX_VIDEO_OUTPUT_H
#define GLX_VIDEO_OUTPUT_H

#ifndef WIN32

#ifdef _DisplayGL_H_
#error "DisplayGL.h included before glx.h!"
#endif

#include <X11/Xlib.h>
#include <unistd.h>

#ifndef LINUX_GNU
#include "GLee.h"
#else
#include <GLee.h>
#endif
#include "DisplayOutput.h"

namespace	DisplayOutput
{

class CUnixGL : public CDisplayOutput
{
    Display     *m_pDisplay;
    GLXContext  m_GlxContext;
    Window      m_Window;
    GLXWindow   m_GlxWindow;
    bool        m_FullScreen;
    int         m_VSync;

    uint32	m_WidthFS;
    uint32	m_HeightFS;

    void    setFullScreen( bool enabled );
    void    setWindowDecorations( bool enabled );
    void    toggleVSync();
    void    alwaysOnTop();
    void    checkClientMessages();
    //bool    checkResizeEvent( ResizeEvent &event);

    public:
            CUnixGL();
            virtual ~CUnixGL();

			static const char *Description()	{	return "X OpenGL display";	};

            virtual bool	Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen );

			//
			virtual void Title( const std::string &_title );
			virtual void Update();

			void SwapBuffers();
};

typedef	CUnixGL	CDisplayGL;

}

#endif

#endif
