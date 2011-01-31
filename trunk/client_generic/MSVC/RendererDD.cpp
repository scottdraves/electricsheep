#ifdef _MSC_VER
#include	<stdint.h>

#include	"d3d9.h"
#include	"DisplayDD.h"
#include	"RendererDD.h"
#include	"TextureFlatDD.h"
#include	"Matrix.h"
#include	"FontDD.h"

namespace DisplayOutput
{

CRendererDD::CRendererDD() : CRenderer()
{
}


CRendererDD::~CRendererDD()
{
}

const bool	CRendererDD::Initialize( spCDisplayOutput _spDisplay )
{
	if( !CRenderer::Initialize( _spDisplay ) )
		return false;

	spCDisplayDD m_spDisplay = (spCDisplayDD)_spDisplay;

	m_WindowHandle = m_spDisplay->WindowHandle();
	m_BackBufferPtr = m_spDisplay->GetBackBufferPtr();


	Defaults();

	return true;
}


void	CRendererDD::Defaults()
{
	g_Log->Info( "CRendererDD::Defaults()" );

}


spCTextureFlat	CRendererDD::NewTextureFlat( spCImage _spImage, const uint32 _flags )
{
	spCTextureFlat	spTex = new CTextureFlatDS(m_BackBufferPtr, _flags );
	spTex->Upload( _spImage );
	return spTex;
}

spCTextureFlat	CRendererDD::NewTextureFlat( const uint32 _flags )
{
	spCTextureFlat	spTex = new CTextureFlatDS(m_BackBufferPtr, _flags );
	return spTex;
}

spCShader	CRendererDD::NewShader( const char *_pVertexShader, const char *_pFragmentShader )
{
	return NULL;
}

spCBaseFont	CRendererDD::NewFont( CFontDescription &_desc )
{
	CBaseFont *pFont = new CFontDD();
	pFont->FontDescription( _desc );
	pFont->Create();
	return pFont;
}

Base::Math::CVector2	CRendererDD::GetTextExtent( spCBaseFont _spFont, const std::string &_text )
{
    ASSERT( _text != "" );

	Base::Math::CVector2 result;

	if( _spFont == NULL )
		return result;

	uint32 width = 0;
    uint32 height = 0;

	fp4	dispWidth  = (fp4)m_spDisplay->Width();
    fp4	dispHeight = (fp4)m_spDisplay->Height();

	spCFontDD	spDDFont = _spFont;
	HFONT	pDDFont = spDDFont->GetDDFont();

    //	Make a copy of `text' and extend it by `.'.
	uint32 textLength = _text.length();
	ASSERT( textLength < 2048 );

	static char	pTmp[ 2048 ];
    strcpy( pTmp, (const char *)_text.c_str() );
    pTmp[ textLength ] = '.';
    pTmp[ textLength + 1 ] = '\0';

    //	Determine extents of `.'.
    RECT dotRect = { 0, 0, LONG(dispWidth), LONG(dispHeight) };

	HDC xdc;
	if (FAILED(m_BackBufferPtr->GetDC(&xdc)))
	{
		return Base::Math::CVector2( 0, 0 );
	}


	HFONT hOldFont;
	if (hOldFont = (HFONT)SelectObject(xdc, pDDFont))
	{
		//SetTextColor(xdc,ddColor);
		SetTextColor(xdc, 0x00ffffff);
		SetBkMode(xdc, TRANSPARENT);

		int32 h = DrawTextA(xdc, ".", -1, &dotRect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT);
		int32 dotWidth = dotRect.right - dotRect.left;

		RECT rect = {0, 0, LONG(dispWidth), LONG(dispHeight) };
		h = DrawTextA(xdc, pTmp, -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT);

		width = rect.right - rect.left - dotWidth;
		height = rect.bottom - rect.top;
		
		result = Base::Math::CVector2( (fp4(width) / dispWidth), (fp4(height) / dispHeight) );

		SelectObject(xdc, hOldFont);
	}
	m_BackBufferPtr->ReleaseDC(xdc);

	return( result );
}

void	CRendererDD::Text( spCBaseFont _spFont, const std::string &_text, const Base::Math::CVector4 &_color, const Base::Math::CRect &_rect, uint32 _flags )
{
	ASSERT( _text != "" );

	const fp4 w05 = (fp4)m_spDisplay->Width() * 0.5f;
	const fp4 h05 = (fp4)m_spDisplay->Height() * 0.5f;
	Base::Math::CRect _r( lerpMacro( -w05, w05, _rect.m_X0 ), lerpMacro( -h05, h05, _rect.m_Y0 ), lerpMacro( -w05, w05, _rect.m_X1 ), lerpMacro( -h05, h05, _rect.m_Y1 ) );

	RECT r = { (LONG)_r.m_X0, (LONG)_r.m_Y0, (LONG)_r.m_X1, (LONG)_r.m_Y1, };

	//COLORREF ddColor = D3DCOLOR_COLORVALUE( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );
	
	HDC xdc;
	if (FAILED(m_BackBufferPtr->GetDC(&xdc)))
	{
		return;
	}

	spCFontDD	spDDFont = _spFont;
	HFONT hFont = spDDFont->GetDDFont();
	HFONT hOldFont;
	if (hOldFont = (HFONT)SelectObject(xdc, hFont))
	{
		//SetTextColor(xdc,ddColor);
		SetTextColor(xdc, 0x00ffffff);
		SetBkMode(xdc, TRANSPARENT);
		r.left = (LONG)w05 + r.left;
		r.top = (LONG)h05 + r.top;
		
		r.right = (LONG)w05 + r.right;
		r.bottom = (LONG)h05 + r.bottom;

		DrawTextA(xdc, _text.c_str(), -1, &r, DT_WORDBREAK);

		SelectObject(xdc, hOldFont);
	}
	m_BackBufferPtr->ReleaseDC(xdc);
}


void	CRendererDD::DrawQuad( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const Base::Math::CRect &_uvrect )
{
	m_BackBufferPtr->DrawQuad( _rect, _color, _uvrect );
}

const bool	CRendererDD::EndFrame( bool drawn )
{
	m_BackBufferPtr->Blt();
	return true ;
};

}

#endif