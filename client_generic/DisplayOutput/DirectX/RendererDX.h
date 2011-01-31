#ifndef	_RENDERERDX_H_
#define	_RENDERERDX_H_

#include <string>
#include <d3d9.h>
#include <d3dx9.h>

#include "base.h"
#include "SmartPtr.h"
#include "Renderer.h"
#include "TextureFlat.h"
#include "Image.h"
#include "Vector4.h"
#include	"../msvc/DirectX_DLL_functions.h"

namespace	DisplayOutput
{

/*
	CRendererDX().

*/
class CRendererDX : public CRenderer
{
	HWND	m_WindowHandle;
	D3DPRESENT_PARAMETERS m_PresentationParams;
	IDirect3DDevice9	*m_pDevice;

	//ID3DXLine			*m_pLine;
	ID3DXSprite			*m_pSprite;

	std::vector<ID3DXFont*> m_Fonts;
	spCTextureFlat		m_spSoftCorner;

	public:
			CRendererDX();
			virtual ~CRendererDX();

			IDirect3DDevice9	*Device()	{	return m_pDevice;	};

			virtual const eRenderType	Type( void ) const {	return eDX9;	};
			virtual const std::string	Description( void ) const { return "DirectX 9"; };

			//
			const bool	Initialize( spCDisplayOutput _spDisplay );

			//
			void	Defaults();

			//
			const bool	BeginFrame( void );
			const bool	EndFrame( bool drawn );

			//
			void	Apply();
			void	Reset( const uint32 _flags );

			bool	TestResetDevice();

			//
			spCTextureFlat	NewTextureFlat( const uint32 flags = 0 );
			spCTextureFlat	NewTextureFlat( spCImage _spImage, const uint32 flags = 0 );

			//
			spCBaseFont		NewFont( CFontDescription &_desc );
			void			Text( spCBaseFont _spFont, const std::string &_text, const Base::Math::CVector4 &_color, const Base::Math::CRect &_rect, uint32 _flags );
			Base::Math::CVector2	GetTextExtent( spCBaseFont _spFont, const std::string &_text );

			//
			spCShader		NewShader( const char *_pVertexShader, const char *_pFragmentShader );

			//	Aux functions.
			void	DrawLine( const Base::Math::CVector2 &_start, const Base::Math::CVector2 &_end, const Base::Math::CVector4 &_color, const fp4 _width = 1.0f );
			void	DrawRect( const Base::Math::CRect	&_rect, const Base::Math::CVector4 &_color, const fp4 _width = 1.0f );
			void	DrawQuad( const Base::Math::CRect	&_rect, const Base::Math::CVector4 &_color );
			void	DrawQuad( const Base::Math::CRect	&_rect, const Base::Math::CVector4 &_color, const Base::Math::CRect &_uvRect );
			void	DrawSoftQuad( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const fp4 _width );
};

MakeSmartPointers( CRendererDX );

}

#endif
