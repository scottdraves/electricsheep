#include	<stdint.h>

#include	"DisplayDX.h"
#include	"RendererDX.h"
#include	"TextureFlatDX.h"
#include	"ShaderDX.h"
#include	"FontDX.h"
#include	"Matrix.h"

namespace DisplayOutput
{

/*
	GetBlendConstant().

*/
const int32	GetBlendConstant( const int32 _src )
{
	static	int32	formatLut[] =
	{
		D3DBLEND_ZERO,
		D3DBLEND_ONE,
		D3DBLEND_SRCCOLOR,
		D3DBLEND_INVSRCCOLOR,
		D3DBLEND_DESTCOLOR,
		D3DBLEND_INVDESTCOLOR,
		D3DBLEND_SRCALPHA,
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_DESTALPHA,
		D3DBLEND_INVDESTALPHA,
		D3DBLEND_SRCALPHASAT,
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
		D3DBLENDOP_ADD,
		D3DBLENDOP_SUBTRACT,
		D3DBLENDOP_REVSUBTRACT,
		D3DBLENDOP_MIN,
		D3DBLENDOP_MAX,
	};

	return( formatLut[ _src ] );
}



/*
*/
CRendererDX::CRendererDX() : CRenderer(), m_pDevice(NULL), m_pSprite(NULL)
{
}

/*
*/
CRendererDX::~CRendererDX()
{
	for (size_t ii = 0; ii < m_Fonts.size(); ++ii)
	{
		SAFE_RELEASE(m_Fonts[ii]);		
	}
	SAFE_RELEASE( m_pSprite );
	Reset( DisplayOutput::eEverything );
	Apply();
	SAFE_RELEASE( m_pDevice );
}

/*
*/
const bool	CRendererDX::Initialize( spCDisplayOutput _spDisplay )
{
	if( !CRenderer::Initialize( _spDisplay ) )
		return false;

	if (g_DLLFun->Init() == false)
		return false;

	spCDisplayDX spDisplay = (spCDisplayDX)_spDisplay;

	m_WindowHandle = spDisplay->WindowHandle();

	m_pDevice = spDisplay->Device();

	m_PresentationParams = spDisplay->PresentParameters();

	D3DVIEWPORT9 vp = { 0, 0, _spDisplay->Width(), _spDisplay->Height(), 0.0f, 1.0f };
	m_pDevice->SetViewport( &vp );

//	D3DXCreateLine( m_pDevice, &m_pLine );

	//	Create sprite object for batched rendering.
	HRESULT hr = g_DLLFun->D3DXCreateSprite_fun( m_pDevice, &m_pSprite );
	if( FAILED(hr) )
	{
        g_Log->Error( "D3DCreateSprite() failed!" );
        return false;
	}

	m_spSoftCorner = NULL;

	Defaults();

	return true;
}

void	CRendererDX::Defaults()
{
	g_Log->Info( "CRendererDX::Defaults()" );

	m_pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	m_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	m_pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL); 
	m_pDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)8); 

	m_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	m_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	m_pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	m_pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );

	m_pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	m_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );

	m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	m_pDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	m_pDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	m_pDevice->SetSamplerState( 3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	m_pDevice->SetSamplerState( 3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
}

bool	CRendererDX::TestResetDevice()
{
	HRESULT devicestate = m_pDevice->TestCooperativeLevel();
	if (devicestate == D3DERR_DEVICELOST)
	{
		Sleep(100);
		return true;
	}
	else
	if (devicestate == D3DERR_DEVICENOTRESET)
	{
		m_pSprite->OnLostDevice();
		for (size_t i = 0; i < m_Fonts.size(); ++i)
		{
			m_Fonts[i]->OnLostDevice();
		}
		m_pDevice->Reset(&m_PresentationParams);
		m_pSprite->OnResetDevice();
		for (size_t i = 0; i < m_Fonts.size(); ++i)
		{
			m_Fonts[i]->OnResetDevice();
		}
		Defaults();
		Reset( DisplayOutput::eEverything );
		Apply();
	}
	return false;
}
/*
*/
const bool	CRendererDX::BeginFrame( void )
{
	if( !CRenderer::BeginFrame() )
		return false;
	
	if (TestResetDevice())
		return false;

	m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0., 0);

    if(FAILED(m_pDevice->BeginScene()))
	{
        return false;
    }

	return true;
}

/*
*/
const bool	CRendererDX::EndFrame( bool drawn = true )
{
	if( !CRenderer::EndFrame() )
		return false;

	if (TestResetDevice())
		return false;

    m_pDevice->EndScene();
	//IDirect3DSwapChain9 *swapchain = NULL;
	//if SUCCEEDED(m_pDevice->GetSwapChain(0, &swapchain))
	//{
	//	if (swapchain->Present(NULL, NULL, NULL, NULL, D3DPRESENT_DONOTWAIT) == D3DERR_WASSTILLDRAWING)
	//	{
	//		return false;
	//	}
	//	swapchain->Release();
	//}

	m_pDevice->Present( NULL, NULL, NULL, NULL );

	return true;
}

/*
*/
void	CRendererDX::Apply()
{
	//	Update world transformation.
	if( isBit( m_bDirtyMatrices, eWorld ) )
	{
		m_pDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX *) (const fp4 *)m_WorldMat.m_Mat );
		remBit( m_bDirtyMatrices, eWorld );
	}

	//	Update view transformation.
	if( isBit( m_bDirtyMatrices, eView ) )
	{
		m_pDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX *) (const fp4 *)m_ViewMat.m_Mat );
		remBit( m_bDirtyMatrices, eView );
	}

	//	Update projection transformation.
	if( isBit( m_bDirtyMatrices, eProjection ) )
	{
		m_pDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX *) (const fp4 *)m_ProjMat.m_Mat );
		remBit( m_bDirtyMatrices, eProjection );
	}

	//	Blend state.	Suboptimal!
	if( m_spActiveBlend != m_spSelectedBlend )
	{
		m_spActiveBlend = m_spSelectedBlend;

		m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, m_spActiveBlend->m_bEnabled );
		m_pDevice->SetRenderState( D3DRS_SRCBLEND, GetBlendConstant( m_spActiveBlend->m_Src ) );
		m_pDevice->SetRenderState( D3DRS_DESTBLEND, GetBlendConstant( m_spActiveBlend->m_Dst ) );
		m_pDevice->SetRenderState( D3DRS_BLENDOP, GetBlendMode( m_spActiveBlend->m_Mode ) );
	}

	CRenderer::Apply();
}

/*
*/
void	CRendererDX::Reset( const uint32 _flags )
{
	CRenderer::Reset( _flags );
}


/*
*/
spCTextureFlat	CRendererDX::NewTextureFlat( spCImage _spImage, const uint32 _flags )
{
	spCTextureFlat	spTex = new CTextureFlatDX( m_pDevice, _flags );
	spTex->Upload( _spImage );
	return spTex;
}

/*
*/
spCTextureFlat	CRendererDX::NewTextureFlat( const uint32 _flags )
{
	spCTextureFlat	spTex = new CTextureFlatDX( m_pDevice, _flags );
	return spTex;
}

/*
*/
spCShader	CRendererDX::NewShader( const char *_pVertexShader, const char *_pFragmentShader )
{
	spCShader spShader = new CShaderDX( m_pDevice, fp4(m_spDisplay->Width()), fp4(m_spDisplay->Height()) );
	if( !spShader->Build( _pVertexShader, _pFragmentShader  ) )
		return NULL;

	if( m_spActiveShader != NULL )
		m_spActiveShader->Bind();

	return spShader;
}

/*
*/
spCBaseFont	CRendererDX::NewFont( CFontDescription &_desc )
{
	CBaseFont *pFont = new CFontDX( m_pDevice );
	pFont->FontDescription( _desc );
	pFont->Create();
	ID3DXFont *font = ((CFontDX *) pFont)->GetDXFont();
	font->AddRef();
	m_Fonts.push_back(font);
	return pFont;
}

/*
*/
void	CRendererDX::Text( spCBaseFont _spFont, const std::string &_text, const Base::Math::CVector4 &_color, const Base::Math::CRect &_rect, uint32 _flags )
{
    ASSERT( _text != "" );

	if( _spFont == NULL )
		return;

	const fp4 w05 = (fp4)m_spDisplay->Width() * 0.5f;
	const fp4 h05 = (fp4)m_spDisplay->Height() * 0.5f;
	Base::Math::CRect _r( lerpMacro( -w05, w05, _rect.m_X0 ), lerpMacro( -h05, h05, _rect.m_Y0 ), lerpMacro( -w05, w05, _rect.m_X1 ), lerpMacro( -h05, h05, _rect.m_Y1 ) );

	RECT r = { (LONG)_r.m_X0, (LONG)_r.m_Y0, (LONG)_r.m_X1, (LONG)_r.m_Y1, };

    DWORD d3dFlags = DT_NOCLIP;

	if( _flags & CBaseFont::Bottom )		d3dFlags |= DT_BOTTOM; // bug if CBaseFont::Bottom == 0 because of check _flag & 0 != 0
    if( _flags & CBaseFont::Top )			d3dFlags |= DT_TOP;
    if( _flags & CBaseFont::Center )		d3dFlags |= DT_CENTER;
    if( _flags & CBaseFont::Left )			d3dFlags |= DT_LEFT;
    if( _flags & CBaseFont::Right )			d3dFlags |= DT_RIGHT;
    if( _flags & CBaseFont::VCenter )		d3dFlags |= DT_VCENTER;
    if( _flags & CBaseFont::NoClip )		d3dFlags |= DT_NOCLIP;
    if( _flags & CBaseFont::ExpandTabs )	d3dFlags |= DT_EXPANDTABS;
    if( _flags & CBaseFont::WordBreak )		d3dFlags |= DT_WORDBREAK;

	DWORD d3dColor = D3DCOLOR_COLORVALUE( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );

	spCFontDX	spDXFont = _spFont;
	ID3DXFont	*pDXFont = spDXFont->GetDXFont();
    ASSERT( pDXFont );

    m_pSprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE );
    pDXFont->DrawTextA( m_pSprite, (const char *)_text.c_str(), -1, &r, d3dFlags, d3dColor );
    m_pSprite->End();
}

/*
*/
Base::Math::CVector2	CRendererDX::GetTextExtent( spCBaseFont _spFont, const std::string &_text )
{
    ASSERT( _text != "" );

	Base::Math::CVector2 result;

	if( _spFont == NULL )
		return result;

	uint32 width = 0;
    uint32 height = 0;

	fp4	dispWidth  = (fp4)m_spDisplay->Width();
    fp4	dispHeight = (fp4)m_spDisplay->Height();

	spCFontDX	spDXFont = _spFont;
	ID3DXFont	*pDXFont = spDXFont->GetDXFont();
    ASSERT( pDXFont );

    //	Make a copy of `text' and extend it by `.'.
	size_t textLength = _text.length();
	ASSERT( textLength < 2048 );

	static char	pTmp[ 2048 ];
    strcpy( pTmp, (const char *)_text.c_str() );
    pTmp[ textLength ] = '.';
    pTmp[ textLength + 1 ] = '\0';

    //	Determine extents of `.'.
    RECT dotRect = { 0, 0, 0, 0 };
    int32 h = pDXFont->DrawTextA( NULL, ".", -1, &dotRect, DT_LEFT | DT_NOCLIP | DT_CALCRECT, 0 );
    int32 dotWidth = dotRect.right - dotRect.left;

    RECT rect = { 0, 0, 0, 0 };
    h = pDXFont->DrawTextA( NULL, pTmp, -1, &rect, DT_LEFT | DT_NOCLIP | DT_CALCRECT, 0 );

    width = rect.right - rect.left - dotWidth;
    height = rect.bottom - rect.top;

	result = Base::Math::CVector2( (fp4(width) / dispWidth), (fp4(height) / dispHeight) );

	return( result );
}


/*
	DrawRect().

*/
void	CRendererDX::DrawRect( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const fp4 _width )
{
/*	ASSERT( m_pLine != NULL );

	D3DXVECTOR2 aVertices[] = { D3DXVECTOR2( _rect.m_X0, _rect.m_Y0 ),
								D3DXVECTOR2( _rect.m_X1, _rect.m_Y0 ),
								D3DXVECTOR2( _rect.m_X1, _rect.m_Y1 ),
								D3DXVECTOR2( _rect.m_X0, _rect.m_Y1 ),
								D3DXVECTOR2( _rect.m_X0, _rect.m_Y0 )	};

	DWORD rgba = D3DCOLOR_COLORVALUE( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );

	m_pLine->SetWidth( _width );
	m_pLine->Draw( aVertices, sizeof(aVertices) / sizeof(aVertices[0]), rgba );*/
}

/*
	DrawQuad().

*/
void	CRendererDX::DrawQuad( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color )
{
	m_pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

	const fp4 w05 = (fp4)m_spDisplay->Width() * 0.5f;
	const fp4 h05 = (fp4)m_spDisplay->Height() * 0.5f;
	Base::Math::CRect r( lerpMacro( -w05, w05, _rect.m_X0 ), lerpMacro( -h05, h05, _rect.m_Y0 ), lerpMacro( -w05, w05, _rect.m_X1 ), lerpMacro( -h05, h05, _rect.m_Y1 ) );

	DWORD rgba = D3DCOLOR_COLORVALUE( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );

	struct Q {
		D3DXVECTOR3	vertex;
		uint32		color;
	};

	Q qv[4];

	qv[0].vertex = D3DXVECTOR3( r.m_X0, r.m_Y0, 0 );
	qv[1].vertex = D3DXVECTOR3( r.m_X1, r.m_Y0, 0 );
	qv[2].vertex = D3DXVECTOR3( r.m_X1, r.m_Y1, 0 );
	qv[3].vertex = D3DXVECTOR3( r.m_X0, r.m_Y1, 0 );
	qv[0].color = rgba;
	qv[1].color = rgba;
	qv[2].color = rgba;
	qv[3].color = rgba;

	if(!SUCCEEDED(m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, qv, sizeof(Q) ))) {
	    g_Log->Error("DrawPrimitiveUP failed");
	}
}

/*
*/
void	CRendererDX::DrawSoftQuad( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const fp4 _width )
{
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
	DWORD rgba = D3DCOLOR_COLORVALUE( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );

	struct Q {
		D3DXVECTOR3	vertex;
		uint32		color;
		D3DXVECTOR2	tc;
		Q( const fp4 _x, const fp4 _y, const fp4 _tx, const fp4 _ty, const uint32 _c )
		{
			vertex = D3DXVECTOR3( _x, _y, 0 );
			tc = D3DXVECTOR2( _tx, _ty );
			color = _c;
		}
	};

	fp4 x0 = r.m_X0;
	fp4 y0 = r.m_Y0;
	fp4 x1 = r.m_X1;
	fp4 y1 = r.m_Y1;

	fp4 x0bw = r.m_X0 + _width;
	fp4 y0bw = r.m_Y0 + _width;
	fp4 x1bw = r.m_X1 - _width;
	fp4 y1bw = r.m_Y1 - _width;

	m_pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

	Q border[] =
	{
		Q(x0,   y0bw, 1, 0,rgba),
		Q(x0,   y0  , 1, 1,rgba),
		Q(x0bw, y0bw, 0, 0,rgba),
		Q(x0bw, y0  , 0, 1,rgba),
		Q(x1bw, y0bw, 0, 0,rgba),
		Q(x1bw, y0  , 0, 1,rgba),

		Q(x1bw, y0  , 0, 1,rgba),
		Q(x1,   y0  , 1, 1,rgba),
		Q(x1bw, y0bw, 0, 0,rgba),
		Q(x1,   y0bw, 1, 0,rgba),
		Q(x1bw, y1bw, 0, 0,rgba),
		Q(x1,   y1bw, 1, 0,rgba),

		Q(x1,   y1bw, 1, 0,rgba),
		Q(x1,   y1  , 1, 1,rgba),
		Q(x1bw, y1bw, 0, 0,rgba),
		Q(x1bw, y1  , 0, 1,rgba),
		Q(x0bw, y1bw, 0, 0,rgba),
		Q(x0bw, y1  , 0, 1,rgba),

		Q(x0bw, y1  , 0, 1,rgba),
		Q(x0,   y1  , 1, 1,rgba),
		Q(x0bw, y1bw, 0, 0,rgba),
		Q(x0,   y1bw, 1, 0,rgba),
		Q(x0bw, y0bw, 0, 0,rgba),
		Q(x0,   y0bw, 1, 0,rgba),
	};

	SetTexture( m_spSoftCorner, 0 );
	Apply();

	m_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	m_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	if(!SUCCEEDED(m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 22, border, sizeof(Q) ))) {
	    g_Log->Error("DrawSoftQuad::DrawPrimitiveUP failed");
	}

	// Center
	SetTexture( NULL, 0 );
	Apply();

	fp4 wu = _width / m_spDisplay->Width();
	fp4 hu = _width / m_spDisplay->Height();

	Base::Math::CRect r2( _rect.m_X0 + wu, _rect.m_Y0 + hu, _rect.m_X1 - wu, _rect.m_Y1 - hu );
	DrawQuad( r2, _color );
}


/*
	DrawQuad().

*/
void	CRendererDX::DrawQuad( const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const Base::Math::CRect &_uvrect )
{
	m_pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

	const fp4 w05 = (fp4)m_spDisplay->Width() * 0.5f;
	const fp4 h05 = (fp4)m_spDisplay->Height() * 0.5f;
	Base::Math::CRect r( lerpMacro( -w05, w05, _rect.m_X0 ), lerpMacro( -h05, h05, _rect.m_Y0 ), lerpMacro( -w05, w05, _rect.m_X1 ), lerpMacro( -h05, h05, _rect.m_Y1 ) );

	DWORD rgba = D3DCOLOR_COLORVALUE( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );

	struct Q {
		D3DXVECTOR3	vertex;
		uint32		color;
		D3DXVECTOR2	tc;
	};

	Q qv[4];

	qv[0].vertex = D3DXVECTOR3( r.m_X0-1.f, r.m_Y0-1.f, 0 );
	qv[1].vertex = D3DXVECTOR3( r.m_X1, r.m_Y0-1.f, 0 );
	qv[2].vertex = D3DXVECTOR3( r.m_X1, r.m_Y1, 0 );
	qv[3].vertex = D3DXVECTOR3( r.m_X0-1.f, r.m_Y1, 0 );
	qv[0].color = rgba;
	qv[1].color = rgba;
	qv[2].color = rgba;
	qv[3].color = rgba;
	qv[0].tc = D3DXVECTOR2( _uvrect.m_X0, _uvrect.m_Y0 );
	qv[1].tc = D3DXVECTOR2( _uvrect.m_X1, _uvrect.m_Y0 );
	qv[2].tc = D3DXVECTOR2( _uvrect.m_X1, _uvrect.m_Y1 );
	qv[3].tc = D3DXVECTOR2( _uvrect.m_X0, _uvrect.m_Y1 );

	if(!SUCCEEDED(m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, qv, sizeof(Q) ))) {
	    g_Log->Error("DrawPrimitiveUP 2 failed");
	}
}

/*
	DrawRect().

*/
void	CRendererDX::DrawLine( const Base::Math::CVector2 &_start, const Base::Math::CVector2 &_end, const Base::Math::CVector4 &_color, const fp4 _width )
{
/*	ASSERT( m_pLine != NULL );

	D3DXVECTOR2 aVertices[] = { D3DXVECTOR2( _start.m_X, _start.m_Y ), D3DXVECTOR2( _end.m_X, _end.m_Y ) };

	DWORD rgba = D3DCOLOR_COLORVALUE( _color.m_X, _color.m_Y, _color.m_Z, _color.m_W );

	m_pLine->SetWidth( _width );
	m_pLine->Draw( aVertices, sizeof(aVertices) / sizeof(aVertices[0]), rgba );*/
}

}

