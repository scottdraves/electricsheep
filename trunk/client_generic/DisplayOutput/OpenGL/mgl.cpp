#ifdef	MAC

#include <string>

#include "Log.h"
#include "Settings.h"
#include "mgl.h"


#include	"Exception.h"


namespace	DisplayOutput
{

uint32 CMacGL::s_DefaultWidth = 0;
uint32 CMacGL::s_DefaultHeight = 0;

/*
	CMacGL().

*/
CMacGL::CMacGL() : CDisplayOutput()
{
	m_glContext = NULL;
}

/*
	~CMacGL().

*/
CMacGL::~CMacGL()
{
}

void  CMacGL::ForceWidthAndHeight( int32 _width, int32 _height )
{
	m_Width = _width;
	m_Height = _height;
		
	if ( m_glContext )
	{		
		CGLSetCurrentContext (m_glContext);
		glViewport(0, 0, m_Width, m_Height);
	}
}
	
void	CMacGL::SetDefaultWidthAndHeight( uint32 defWidth, uint32 defHeight )
{
	s_DefaultWidth = defWidth;
	s_DefaultHeight = defHeight;
}
	
bool CMacGL::Initialize( CGLContextObj _glContext, bool _bPreview)
{
	if (_glContext)
	{		
		SetContext(_glContext);

		CGLSetCurrentContext (m_glContext);	
		
		GLint vblsync = 0;
		
		if ( g_Settings()->Get( "settings.player.vbl_sync", false ) )
		{
			vblsync = 1;
		}
		
		/*CGLError err =  *///CGLEnable( m_glContext, kCGLCEMPEngine);
		
		//if ( err != kCGLNoError )
		//	vblsync = 0;
			
		CGLSetParameter(m_glContext, kCGLCPSwapInterval, &vblsync);
	}
	
	m_Width = s_DefaultWidth;
	m_Height = s_DefaultHeight;
	
	return true;
}

//
void CMacGL::Title( const std::string &_title )
{
}

//
void	CMacGL::Update()
{	
}

/*
*/
void    CMacGL::SwapBuffers()
{
	CGLFlushDrawable(m_glContext);
}

};

#endif
