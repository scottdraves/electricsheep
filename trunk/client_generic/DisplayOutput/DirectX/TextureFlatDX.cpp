#include <assert.h>
#include <inttypes.h>
#include <string>

#include "base.h"
#include "Log.h"
#include "MathBase.h"
#include "Exception.h"
#include "DisplayOutput.h"
#include "RendererDX.h"
#include "TextureFlatDX.h"

namespace	DisplayOutput
{

//	These map 1:1 with eImageFormat in Image.h... *Keep them in sync!!!*
static const D3DFORMAT dxformats[] =
{
	D3DFMT_UNKNOWN,

	// Plain formats
	D3DFMT_L8,
	D3DFMT_A8L8,
	D3DFMT_X8R8G8B8,
	D3DFMT_A8R8G8B8,

	D3DFMT_L16,
	D3DFMT_G16R16,
	D3DFMT_UNKNOWN, // RGB16 not directly supported
	D3DFMT_A16B16G16R16,

	D3DFMT_R16F,
	D3DFMT_G16R16F,
	D3DFMT_UNKNOWN, // RGB16F not directly supported
	D3DFMT_A16B16G16R16F,

	D3DFMT_R32F,
	D3DFMT_G32R32F,
	D3DFMT_UNKNOWN, // RGB32F not directly supported
	D3DFMT_A32B32G32R32F,

	// Packed formats
	D3DFMT_A4R4G4B4,
	D3DFMT_R5G6B5,

	// Compressed formats
	D3DFMT_DXT1,
	D3DFMT_DXT3,
	D3DFMT_DXT5,

	// Depth formats
	D3DFMT_D16,
	D3DFMT_D24X8,
};

/*
*/
CTextureFlatDX::CTextureFlatDX( IDirect3DDevice9 *_pDevice, const uint32 _flags ) : CTextureFlat( _flags ), m_pDevice( _pDevice ), m_pTextureDX9(NULL)
{
	if (_pDevice == NULL)
		g_Log->Error( "Texture received device == NULL" );
}

/*
*/
CTextureFlatDX::~CTextureFlatDX()
{
	SAFE_RELEASE( m_pTextureDX9 );
}

/*
*/
bool	CTextureFlatDX::Upload( spCImage _spImage )
{
	if( m_spImage == NULL )
		m_spImage = _spImage;

	CImageFormat	format = m_spImage->GetFormat();

	if( m_Size.iWidth() != (int32)_spImage->GetWidth() ||
		m_Size.iHeight() != (int32)_spImage->GetHeight() ||
		m_Format != format.getFormatEnum() )
	{
		//	Stuff changed, nuke old texture.
		SAFE_RELEASE( m_pTextureDX9 );
	}

	if( !m_pTextureDX9 )
	{
		HRESULT	hr = m_pDevice->CreateTexture(	m_spImage->GetWidth(), m_spImage->GetHeight(),
												m_spImage->GetNumMipMaps(),
												0, dxformats[ format.getFormatEnum() ],
												D3DPOOL_MANAGED, &m_pTextureDX9, NULL );

		g_Log->Info( "%s texture created", format.GetDescription().c_str() );

		m_Size = Base::Math::CRect( fp4(m_spImage->GetWidth()), fp4(m_spImage->GetHeight()) );
		m_Format = format.getFormatEnum();

		if( FAILED( hr ) )
		{
			g_Log->Error( "Texture upload failed!" );
			return( false );
		}
	}

	//	Copy texturedata.
	uint32	mipMapLevel = 0;
	uint8	*pSrc = m_spImage->GetData( 0 );

	while( pSrc )
	{
		D3DLOCKED_RECT rect;

		HRESULT hr = m_pTextureDX9->LockRect( mipMapLevel, &rect, NULL, 0 );
		if( !FAILED(hr) )
		{
			memcpy( rect.pBits, pSrc, m_spImage->getMipMappedSize( mipMapLevel, 1 ) );
			m_pTextureDX9->UnlockRect( mipMapLevel );
		}

		pSrc = m_spImage->GetData( ++mipMapLevel );
	}

	m_bDirty = true;
	return true;
}

/*
*/
bool	CTextureFlatDX::Bind( const uint32 _index )
{
	HRESULT hr = m_pDevice->SetTexture( _index, m_pTextureDX9 );
	if( FAILED( hr ) )
	{
		g_Log->Error( "Bind failed!" );
		return( false );
	}

	return true;
}

/*
*/
bool	CTextureFlatDX::Unbind( const uint32 _index )
{
	HRESULT hr = m_pDevice->SetTexture( _index, NULL );
	if( FAILED( hr ) )
	{
		g_Log->Error( "Bind failed!" );
		return( false );
	}

	return true;
}

}
