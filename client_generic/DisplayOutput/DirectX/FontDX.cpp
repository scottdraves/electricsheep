#include	<stdint.h>
#include	<d3d9.h>
#include	<d3dx9.h>

#include	"base.h"
#include	"FontDX.h"
#include	"Log.h"
#include	"RendererDX.h"

namespace	DisplayOutput
{

/*
	CFontDX().

*/
CFontDX::CFontDX( IDirect3DDevice9 *_pDevice ) : CBaseFont(), m_pDXFont( NULL ), m_pDevice( _pDevice )
{
}


/*
	~CFontDX().

*/
CFontDX::~CFontDX()
{
	ASSERT( m_pDXFont );
	SAFE_RELEASE( m_pDXFont );
}

/*
*/
bool	CFontDX::Create()
{
	ASSERT( m_pDXFont == NULL );

	uint32 fontFlags = 0;
	switch( m_FontDescription.Style() )
	{
		case CFontDescription::Thin:		fontFlags |= FW_THIN;		break;
		case CFontDescription::Light:		fontFlags |= FW_LIGHT;		break;
		case CFontDescription::Normal:		fontFlags |= FW_NORMAL;		break;
		case CFontDescription::Bold:		fontFlags |= FW_BOLD;		break;
		case CFontDescription::UberBold:	fontFlags |= FW_EXTRABOLD;	break;
	}

	uint32	height = m_FontDescription.Height();
	//HDC hdc = GetDC(NULL);
	//if (hdc != NULL)
	//{
	//	height = -MulDiv(m_FontDescription.Height(), GetDeviceCaps(hdc ,LOGPIXELSY), 72);
	//	ReleaseDC(NULL, hdc);
	//	height = -MulDiv(m_FontDescription.Height(), 72, 72) - 1;
	//}
	//else
	//{
	//	height = -MulDiv(m_FontDescription.Height(), 72, 72) - 1;
	//}
	HRESULT hr = g_DLLFun->D3DXCreateFontA_fun(	m_pDevice,
								height, 0,
								fontFlags, 0,
								m_FontDescription.Italic(),
								DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS,
								m_FontDescription.AntiAliased() ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY,
								DEFAULT_PITCH|FF_DONTCARE,
								m_FontDescription.TypeFace().c_str(),
								&m_pDXFont );

	if( FAILED(hr ) )
        ThrowStr( "D3DXCreateFont() failed!" );

	ASSERT( m_pDXFont );

	return( true );
}

};
