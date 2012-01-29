#include	<stdint.h>
#include	<string.h>
#ifndef LINUX_GNU
#include	"./OpenGL/GLee.h"
#else
#include <GLee.h>
#endif
#ifdef MAC
#include	<OpenGL/CGLMacro.h>
//#include	<OpenGL/gl.h>
#include	<GLUT/glut.h>
#else
#include	<GL/gl.h>
#include	<GL/glut.h>
#endif

#include	"RendererGL.h"
#include	"TextureFlatGL.h"
#include	"ShaderGL.h"
#include	"FontGL.h"

namespace DisplayOutput
{
	
/*
 GetBlendConstant().
 
 */
const int32	GetBlendConstant( const int32 _src )
{
	static	int32	formatLut[] =
	{
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_SRC_ALPHA_SATURATE,
	};
	
	return( formatLut[ _src ] );
}

/*
 GetBlendMode().
 
 */
const int32	GetBlendMode( const int32 _src )
{
	static	int32	formatLut[] =
	{
		GL_FUNC_ADD,
		GL_FUNC_SUBTRACT,
		GL_FUNC_REVERSE_SUBTRACT,
		GL_MIN,
		GL_MAX,
	};
	
	return( formatLut[ _src ] );
}
	
	
/*
 */
CRendererGL::CRendererGL() : CRenderer()
{
}

/*
*/
CRendererGL::~CRendererGL()
{
#ifdef  WIN32
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( m_RenderContext );
	ReleaseDC( m_WindowHandle, m_DeviceContext );
#endif
}

/*
*/
const bool	CRendererGL::Initialize( spCDisplayOutput _spDisplay )
{
	if( !CRenderer::Initialize( _spDisplay ) )
		return false;

#ifdef  WIN32
	m_WindowHandle = _spDisplay->WindowHandle();

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof (PIXELFORMATDESCRIPTOR), /* struct size      */
		1,                              /* Version number   */
		PFD_DRAW_TO_WINDOW              /* Flags, draw to a window, */
		| PFD_DOUBLEBUFFER              /* Requires Doublebuffer hw */
		| PFD_SUPPORT_OPENGL,          	/* use OpenGL               */
		PFD_TYPE_RGBA,                  /* RGBA pixel values        */
		32,                             /* 32-bit color             */
		0, 0, 0,                        /* RGB bits & shift sizes.  */
		0, 0, 0,                        /* Don't care about them    */
		0, 0,                           /* No alpha buffer info     */
		0, 0, 0, 0, 0,                  /* No accumulation buffer   */
		16,                             /* depth buffer             */
		0,                              /* stencil buffer           */
		0,                              /* No auxiliary buffers     */
		PFD_MAIN_PLANE,                 /* Layer type               */
		0,                              /* Reserved (must be 0)     */
		0,                              /* No layer mask            */
		0,                              /* No visible mask          */
		0                               /* No damage mask           */
	};

    m_DeviceContext = GetDC( m_WindowHandle );

	int i = ChoosePixelFormat( m_DeviceContext, &pfd );
	if( i == 0 )
		ThrowStr( "ChoosePixelFormat() failed." );

	if( SetPixelFormat( m_DeviceContext, i, &pfd ) == FALSE )
		ThrowStr( "SetPixelFormat() failed." );

	m_RenderContext = wglCreateContext( m_DeviceContext );
	wglMakeCurrent( m_DeviceContext, m_RenderContext );

    if( GLEE_WGL_EXT_swap_control )
        wglSwapIntervalEXT( static_cast<int>(true) );
#endif

#ifdef MAC
	cgl_ctx = m_spDisplay->GetContext();
#endif

	Defaults();

	return true;
}

//
void	CRendererGL::Defaults()
{
	SetCurrentGLContext();

	glShadeModel( GL_FLAT );
	glClearDepth( 0.0f );
	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	glDepthFunc( GL_ALWAYS );
	glDisable( GL_LIGHTING );
	glFrontFace(GL_CW);
	glPixelStorei( GL_PACK_ALIGNMENT,   1 );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	glClear( GL_COLOR_BUFFER_BIT );
}



/*
*/
const bool	CRendererGL::BeginFrame( void )
{
#ifdef MAC
	CGLContextObj currContext = m_spDisplay->GetContext();
	
	if (currContext != NULL)
		CGLLockContext(currContext);
#endif

	SetCurrentGLContext();
	
	glClear( GL_COLOR_BUFFER_BIT );
	
	if( !CRenderer::BeginFrame() )
		return false;

	return true;
}

void CRendererGL::Verify( const char *_file, const int32 _line )
{
	/*int error = glGetError();
	if( error != GL_NO_ERROR )
	{
		std::string msg = "";

		if( error == GL_INVALID_ENUM )	msg = "GL_INVALID_ENUM";
		else if( error == GL_INVALID_VALUE ) msg = "GL_INVALID_VALUE";
		else if( error ==    GL_INVALID_OPERATION) msg = "GL_INVALID_OPERATION";
		else if( error ==    GL_STACK_OVERFLOW)	msg = "GL_STACK_OVERFLOW";
		else if( error ==    GL_STACK_UNDERFLOW)	msg = "GL_STACK_UNDERFLOW";
		else if( error ==    GL_OUT_OF_MEMORY)	msg = "GL_OUT_OF_MEMORY";
		else if( error ==    GL_INVALID_FRAMEBUFFER_OPERATION_EXT)	msg = "GL_INVALID_FRAMEBUFFER_OPERATION_EXT";
		else msg = "Unrecognized OpenGL error";

		g_Log->Error( "OpenGLError - %s(%d) in %s(%d)", msg.c_str(), error, _file, _line );
		
		//ThrowArgs(( "%s in %s(%d)", msg.c_str(), _file, _line ));
	}*/
}

/*
*/
const bool	CRendererGL::EndFrame( bool drawn )
{
	SetCurrentGLContext();
	
	if( !CRenderer::EndFrame( drawn ) )
		return false;

	if ( drawn )
	{
#ifdef  WIN32
		SwapBuffers( m_DeviceContext );
#else
		m_spDisplay->SwapBuffers();
		VERIFYGL;
#endif
	}
	
#ifdef MAC
	CGLContextObj currContext = m_spDisplay->GetContext();
	
	if (currContext != NULL)
		CGLUnlockContext(currContext);
#endif

	return true;
}

/*
*/
void	CRendererGL::Apply()
{
	SetCurrentGLContext();
	
	CRenderer::Apply();

	//	Update world transformation.
	if( isBit( m_bDirtyMatrices, eWorld ) )
	{
		glMatrixMode( GL_MODELVIEW );
		glLoadMatrixf( (GLfloat *)(const fp4 *)m_WorldMat.m_Mat );
		remBit( m_bDirtyMatrices, eWorld );
	}

	//	Update view transformation.
	if( isBit( m_bDirtyMatrices, eView ) )
	{
		remBit( m_bDirtyMatrices, eView );
	}

	//	Update projection transformation.
	if( isBit( m_bDirtyMatrices, eProjection ) )
	{
		glMatrixMode( GL_PROJECTION );
		glLoadMatrixf( (GLfloat *)(const fp4 *)m_ProjMat.m_Mat );
		remBit( m_bDirtyMatrices, eProjection );
	}
	
	//	Blend state.	Suboptimal!
	if( m_spActiveBlend != m_spSelectedBlend )
	{
		m_spActiveBlend = m_spSelectedBlend;
		
		glEnable(GL_BLEND);
		glBlendFunc(GetBlendConstant( m_spActiveBlend->m_Src ),GetBlendConstant( m_spActiveBlend->m_Dst ));
		glBlendEquation(GetBlendMode( m_spActiveBlend->m_Mode ) );
	}
}

/*
*/
void	CRendererGL::Reset( const uint32 _flags )
{	
	SetCurrentGLContext();
	
	CRenderer::Reset( _flags );
}


/*
*/
spCTextureFlat	CRendererGL::NewTextureFlat( spCImage _spImage, const uint32 _flags )
{
	SetCurrentGLContext();

	spCTextureFlat	spTex = new CTextureFlatGL( _flags | ( ( GetTextureTargetType() == eTexture2DRect ) ? kRectTexture : 0 ) );
	spTex->Upload( _spImage );
	return spTex;
}

/*
*/
spCTextureFlat	CRendererGL::NewTextureFlat( const uint32 _flags )
{
	SetCurrentGLContext();

	spCTextureFlat	spTex = new CTextureFlatGL( _flags | ( ( GetTextureTargetType() == eTexture2DRect ) ? kRectTexture : 0 ) );
	return spTex;
}

/*
*/
eTextureTargetType	CRendererGL::GetTextureTargetType( void )
{
#ifndef LINUX_GNU
	if (GL_EXT_texture_rectangle)
#else
	if (GL_ARB_texture_rectangle)
#endif
		return eTexture2DRect;
	else
		return eTexture2D;
}


/*
*/
spCShader	CRendererGL::NewShader( const char *_pVertexShader, const char *_pFragmentShader )
{
	SetCurrentGLContext();
	
	spCShader spShader = new CShaderGL();
	if( !spShader->Build( _pVertexShader, _pFragmentShader  ) )
		return NULL;

	if( m_spActiveShader != NULL )
		m_spActiveShader->Bind();

	return spShader;
}

/*
*/
spCBaseFont	CRendererGL::NewFont( CFontDescription &_desc )
{	
	if (m_glFont.IsNull())
	{
		SetCurrentGLContext();

		m_glFont = new CFontGL( NewTextureFlat() );
		m_glFont->FontDescription( _desc );
		m_glFont->Create();
	}
	
	return m_glFont;
}
		
void CRendererGL::Text( spCBaseFont _spFont, const std::string &_text, const Base::Math::CVector4 &_color, const Base::Math::CRect &_rect, uint32 _flags )
{
	SetCurrentGLContext();
	
	spCFontGL glFont = _spFont;
	
	spCTextureFlat texture = glFont->GetTexture();
		
	SetTexture( texture, 0 );
	Apply();
	
	fp4 x0 = _rect.m_X0, x1, y0 = _rect.m_Y0;
	
	Base::Math::CRect texpow2r = texture->GetRect();
	
	fp4 lineheight = glFont->LineHeight() / m_spDisplay->Height();
	
	for (size_t i = 0; i < _text.size(); i++)
	{
		char ch = _text[i];
		
		if (ch == '\n')
		{
			x0 = _rect.m_X0;
			y0 += lineheight;

			continue;
		}
		
		CFontGL::Glyph *glyph = glFont->GetGlyph(ch);
		
		if (glyph == NULL)
			continue;
		
		x1 = x0 + glyph->advance / m_spDisplay->Width();
		
		Base::Math::CRect r(x0, y0, x1, y0 + lineheight);
				
		Base::Math::CRect texr(glyph->tex_x1 * texpow2r.Width() , glyph->tex_y1 * texpow2r.Height(), glyph->tex_x2 * texpow2r.Width(), (glyph->tex_y1 + glFont->TexLineHeight()) * texpow2r.Height());
		
		DrawQuad( r, Base::Math::CVector4( 1.0f,1.0f,1.0f,1.0f ), texr );
		
		x0 = x1;
	}
	
	SetTexture( NULL, 0 );
	Apply();
}
		
Base::Math::CVector2 CRendererGL::GetTextExtent( spCBaseFont _spFont, const std::string &_text )
{
	SetCurrentGLContext();
	
	spCFontGL glFont = _spFont;
	
	fp4 lineheight = glFont->LineHeight();
	fp4 textheight = lineheight;
	fp4 textwidth = 0.0f;
	
	size_t start = 0, len = 0;
	
	for (size_t i = 0; i < _text.size(); i++)
	{
		char ch = _text[i];
		
		if (ch == '\n')
		{
			if (len > 0)
			{
				fp4 width =  glFont->StringWidth(_text.substr(start, len));
				
				if (width > textwidth)
					textwidth = width;
			}
			
			len = 0;
			start = i+1;
			textheight += lineheight;
			continue;
		}
		
		len++;
	}
	
	if (len > 0)
	{
		fp4 width =  glFont->StringWidth(_text.substr(start, len));
		
		if (width > textwidth)
			textwidth = width;
	}
	
	return Base::Math::CVector2( textwidth / (fp4)m_spDisplay->Width(), textheight / (fp4)m_spDisplay->Height() ); 
}	

/*
 */
void	CRendererGL::DrawQuad( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color )
{
	SetCurrentGLContext();
	
	const fp4 w05 = (fp4)m_spDisplay->Width() * 0.5f;
	const fp4 h05 = (fp4)m_spDisplay->Height() * 0.5f;
	Base::Math::CRect r( lerpMacro( -w05, w05, _rect.m_X0 ), lerpMacro( -h05, h05, _rect.m_Y0 ), lerpMacro( -w05, w05, _rect.m_X1 ), lerpMacro( -h05, h05, _rect.m_Y1 ) );
			
	glBegin(GL_TRIANGLE_FAN);
	{
		glColor4f( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );
		
		glVertex2f( r.m_X0, r.m_Y0 );
		
		glVertex2f( r.m_X1, r.m_Y0 );
		
		glVertex2f( r.m_X1, r.m_Y1 );
		
		glVertex2f( r.m_X0, r.m_Y1 );
	}
	glEnd();
}
	

/*
*/
void	CRendererGL::DrawQuad( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const Base::Math::CRect &_uvrect )
{
	SetCurrentGLContext();
	
	const fp4 w05 = (fp4)m_spDisplay->Width() * 0.5f;
	const fp4 h05 = (fp4)m_spDisplay->Height() * 0.5f;
	Base::Math::CRect r( lerpMacro( -w05, w05, _rect.m_X0 ), lerpMacro( -h05, h05, _rect.m_Y0 ), lerpMacro( -w05, w05, _rect.m_X1 ), lerpMacro( -h05, h05, _rect.m_Y1 ) );
		
	/*float vertices[]  = {r.m_X0, r.m_Y0, r.m_X1, r.m_Y0, r.m_X1, r.m_Y1, r.m_X0, r.m_Y1};
	float texcoords[] = {_uvrect.m_X0, _uvrect.m_Y0, _uvrect.m_X1, _uvrect.m_Y0, _uvrect.m_X1, _uvrect.m_Y1, _uvrect.m_X0, _uvrect.m_Y1 };
	
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glColor4f( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );
	
	glDrawArrays(GL_QUADS, 0, 4);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);*/

	glBegin(GL_QUADS);
	{
		glColor4f( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );

        glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y0 );
        glVertex2f( r.m_X0, r.m_Y0 );

        glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y0 );
        glVertex2f( r.m_X1, r.m_Y0 );

        glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y1 );
        glVertex2f( r.m_X1, r.m_Y1 );

        glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y1 );
        glVertex2f( r.m_X0, r.m_Y1 );
    }
    glEnd();
}
	
void	CRendererGL::DrawSoftQuad( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const fp4 _width )
{
	SetCurrentGLContext();
	
	if( m_spSoftCorner == NULL )
	{
		DisplayOutput::spCImage tmpImage = new DisplayOutput::CImage();
		tmpImage->Create( 32, 32, DisplayOutput::eImage_RGBA8 );
		
		for( uint32 y=0; y<32; y++)
			for( uint32 x=0; x<32; x++ )
			{
				fp4 c = Base::Math::saturate(1.0f - powf( Base::Math::Sqrt( fp4(x*x + y*y) ) / 31.0f, 1.0f ));
				tmpImage->PutPixel( x, y, c, c, c, c );
			}
		
		m_spSoftCorner = NewTextureFlat();
		m_spSoftCorner->Upload( tmpImage );
	}
	
	const fp4 w05 = (fp4)m_spDisplay->Width() * 0.5f;
	const fp4 h05 = (fp4)m_spDisplay->Height() * 0.5f;
	Base::Math::CRect r( lerpMacro( -w05, w05, _rect.m_X0 ), lerpMacro( -h05, h05, _rect.m_Y0 ), lerpMacro( -w05, w05, _rect.m_X1 ), lerpMacro( -h05, h05, _rect.m_Y1 ) );
	
	fp4 x0 = r.m_X0;
	fp4 y0 = r.m_Y0;
	fp4 x1 = r.m_X1;
	fp4 y1 = r.m_Y1;
	
	fp4 x0bw = r.m_X0 + _width;
	fp4 y0bw = r.m_Y0 + _width;
	fp4 x1bw = r.m_X1 - _width;
	fp4 y1bw = r.m_Y1 - _width;
	
	Base::Math::CRect _uvrect = m_spSoftCorner->GetRect();
		
	SetTexture( m_spSoftCorner, 0 );
	Apply();

	glBegin(GL_TRIANGLE_STRIP);
	{
		glColor4f( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );

		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y0 );
        glVertex2f( x0, y0bw );
		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y1 );
        glVertex2f( x0, y0 );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y0 );
        glVertex2f( x0bw, y0bw );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y1 );
        glVertex2f( x0bw, y0 );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y0 );
        glVertex2f( x1bw, y0bw );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y1 );
        glVertex2f( x1bw, y0 );
				
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y1 );
        glVertex2f( x1bw, y0 );
		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y1 );
        glVertex2f( x1, y0 );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y0 );
        glVertex2f( x1bw, y0bw );
		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y0 );
        glVertex2f( x1, y0bw );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y0 );
        glVertex2f( x1bw, y1bw );
		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y0 );
        glVertex2f( x1, y1bw );
		
		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y0 );
        glVertex2f( x1, y1bw );
		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y1 );
        glVertex2f( x1, y1 );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y0 );
        glVertex2f( x1bw, y1bw );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y1 );
        glVertex2f( x1bw, y1 );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y0 );
        glVertex2f( x0bw, y1bw );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y1 );
        glVertex2f( x0bw, y1 );		
		
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y1 );
        glVertex2f( x0bw, y1 );
		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y1 );
        glVertex2f( x0, y1 );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y0 );
        glVertex2f( x0bw, y1bw );
		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y0 );
        glVertex2f( x0, y1bw );
		glTexCoord2f( _uvrect.m_X0, _uvrect.m_Y0 );
        glVertex2f( x0bw, y0bw );
		glTexCoord2f( _uvrect.m_X1, _uvrect.m_Y0 );
        glVertex2f( x0, y0bw );
	}
	glEnd();
	
	// Center
	SetTexture( NULL, 0 );
	Apply();
	
	fp4 wu = _width / m_spDisplay->Width();
	fp4 hu = _width / m_spDisplay->Height();
	
	Base::Math::CRect r2( _rect.m_X0 + wu, _rect.m_Y0 + hu, _rect.m_X1 - wu, _rect.m_Y1 - hu );
	DrawQuad( r2, _color );	
}
	

void 	CRendererGL::SetCurrentGLContext()
{
#ifdef MAC
	CGLContextObj currContext = m_spDisplay->GetContext();
	
	if (currContext != NULL)
		CGLSetCurrentContext(currContext);
#endif	
}


}

