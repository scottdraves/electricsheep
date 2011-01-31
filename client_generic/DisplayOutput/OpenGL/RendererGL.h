#ifndef	_RENDERERGL_H_
#define	_RENDERERGL_H_

#include <string>
#include "base.h"
#include "SmartPtr.h"
#include "Renderer.h"
#include "TextureFlat.h"
#include "Image.h"
#include "FontGL.h"
#ifdef MAC
#undef Random
#include <OpenGL/CGLMacro.h>
//#include <OpenGL/OpenGL.h>
#include <Carbon/Carbon.h>
#endif


namespace	DisplayOutput
{

/*
	CRendererGL().

*/
class CRendererGL : public CRenderer
{
#ifdef  WIN32
	HWND	m_WindowHandle;
	HDC		m_DeviceContext;
	HGLRC	m_RenderContext;
#else
#ifdef MAC
	CGLContextObj m_glContext;
	CGContextRef  m_textContext;
	CGLContextObj cgl_ctx;
#endif
#endif
	
	spCTextureFlat		m_spSoftCorner;
	
	spCImage			m_spTextImage;
	
	spCTextureFlat		m_spTextTexture;
	
	spCFontGL			m_glFont;
	
	Base::Math::CRect	m_textRect;

	public:
			CRendererGL();
			virtual ~CRendererGL();

			virtual const eRenderType	Type( void ) const {	return eGL;	};
			virtual const std::string	Description( void ) const { return "OpenGL"; }

			//
			const bool	Initialize( spCDisplayOutput _spDisplay );

			//
			void	Defaults();

			//
			const bool	BeginFrame( void );
			const bool	EndFrame( bool drawn = true );

			//
			void	Apply();
			void	Reset( const uint32 _flags );

			//
			spCTextureFlat	NewTextureFlat( const uint32 flags = 0 );
			spCTextureFlat	NewTextureFlat( spCImage _spImage, const uint32 flags = 0 );

			eTextureTargetType	GetTextureTargetType( void );

			//
			spCBaseFont		NewFont( CFontDescription &_desc );
			void			Text( spCBaseFont _spFont, const std::string &_text, const Base::Math::CVector4 &_color, const Base::Math::CRect &_rect, uint32 _flags );
			Base::Math::CVector2	GetTextExtent( spCBaseFont _spFont, const std::string &_text );

			//
			spCShader		NewShader( const char *_pVertexShader, const char *_pFragmentShader );

			//
			void	DrawQuad( const Base::Math::CRect	&_rect, const Base::Math::CVector4 &_color );
			void	DrawQuad( const Base::Math::CRect	&_rect, const Base::Math::CVector4 &_color, const Base::Math::CRect &_uvRect );
			void	DrawSoftQuad( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const fp4 _width );

			static	void	Verify( const char *_file, const int32 _line );
	
			void	SetCurrentGLContext();
};

#define	VERIFYGL	CRendererGL::Verify( __FILE__, __LINE__ )

MakeSmartPointers( CRendererGL );

}

#endif
