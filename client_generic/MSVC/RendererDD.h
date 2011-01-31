#ifdef _MSC_VER
#ifndef	_RENDERERDS_H_
#define	_RENDERERDS_H_

#include <string>
#include "base.h"
#include "SmartPtr.h"
#include "Renderer.h"
#include "Image.h"
#include "Vector4.h"
#include "DisplayDD.h"


namespace	DisplayOutput
{


class CRendererDD : public CRenderer
{
	HWND	m_WindowHandle;
	CBackBufDD *m_BackBufferPtr;

	public:
			CRendererDD();
			virtual ~CRendererDD();

			virtual const eRenderType	Type( void ) const {	return eDX9;	};
			virtual const std::string	Description( void ) const { return "DirectX 7"; };

			const bool	Initialize( spCDisplayOutput _spDisplay );

			void	Defaults();

			spCTextureFlat	NewTextureFlat( const uint32 flags = 0 );
			spCTextureFlat	NewTextureFlat( spCImage _spImage, const uint32 flags = 0 );

			spCBaseFont		NewFont( CFontDescription &_desc );
			virtual void	Text( spCBaseFont _spFont, const std::string &_text, const Base::Math::CVector4 &_color, const Base::Math::CRect &_rect, uint32 _flags );
			virtual Base::Math::CVector2	GetTextExtent( spCBaseFont _spFont, const std::string &_text );

			spCShader		NewShader( const char *_pVertexShader, const char *_pFragmentShader );

			void	DrawQuad( const Base::Math::CRect	&_rect, const Base::Math::CVector4 &_color, const Base::Math::CRect &_uvRect );
			virtual const bool	CRendererDD::EndFrame( bool drawn = true );
};

MakeSmartPointers( CRendererDD );

}

#endif
#endif