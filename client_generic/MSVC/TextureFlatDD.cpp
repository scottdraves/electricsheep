#ifdef WIN32
#ifdef _MSC_VER
#include <assert.h>
#include <inttypes.h>
#include <string>

#include "base.h"
#include "Log.h"
#include "MathBase.h"
#include "Exception.h"
#include "DisplayOutput.h"
#include "RendererDD.h"
#include "TextureFlatDD.h"


namespace	DisplayOutput
{

CTextureFlatDS::CTextureFlatDS(CBackBufDD *backbuffer, const uint32 _flags ) : CTextureFlat( _flags ), m_pTextureDS(NULL), m_BackBuffer(backbuffer)
{
}


CTextureFlatDS::~CTextureFlatDS()
{
	SAFE_RELEASE( m_pTextureDS );
}


bool	CTextureFlatDS::Upload( spCImage _spImage )
{
	if( m_spImage == NULL )
		m_spImage = _spImage;

	CImageFormat	format = m_spImage->GetFormat();

	if( m_Size.iWidth() != (int32)_spImage->GetWidth() ||
		m_Size.iHeight() != (int32)_spImage->GetHeight() ||
		m_Format != format.getFormatEnum() )
	{
		//	Stuff changed, nuke old texture.
		SAFE_RELEASE( m_pTextureDS );
	}

	if( !m_pTextureDS )
	{
		DDSURFACEDESC2	sd = {0};
		sd.dwSize = sizeof(sd);
		sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;
		sd.dwWidth = 1;
		sd.dwHeight	= 1;

		HRESULT hr = m_BackBuffer->CreateSurface(&sd, &m_pTextureDS);
		g_Log->Info( "%s texture created, HR=%d", format.GetDescription().c_str(), hr);

		m_Size = Base::Math::CRect( fp4(m_spImage->GetWidth()), fp4(m_spImage->GetHeight()) );
		m_Format = format.getFormatEnum();

		if (m_pTextureDS == NULL)
			return false;
	}

	return true;
}

bool CTextureFlatDS::Bind( const uint32 _index )
{
	if (m_pTextureDS == NULL)
		return false;
	DDSURFACEDESC2	sd = {0};
	sd.dwSize = sizeof(sd);
	sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
	sd.dwWidth = m_spImage->GetWidth();
	sd.dwHeight = m_spImage->GetHeight();
	sd.lPitch =  m_spImage->GetWidth()*4;
	sd.lpSurface = m_spImage->GetData( 0 );
	sd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	sd.ddpfPixelFormat.dwRGBBitCount = 32;
	sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	sd.ddpfPixelFormat.dwRBitMask = 0xff0000;
	sd.ddpfPixelFormat.dwGBitMask = 0x00ff00;
	sd.ddpfPixelFormat.dwBBitMask = 0x0000ff;

	if (FAILED(m_pTextureDS->SetSurfaceDesc(&sd, 0)))
			return false;

	m_pTextureDS->AddRef();
	m_BackBuffer->SetContexts(m_pTextureDS, m_Size.iWidth(), m_Size.iHeight());

	return true;
}

bool CTextureFlatDS::Unbind( const uint32 _index )
{
	return true;
}


}
#endif
#endif