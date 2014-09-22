#include	<stdint.h>
#include	<stdlib.h>
#include	<string.h>

#include	"base.h"
#include	"Exception.h"
#include	"Log.h"
#include	"MathBase.h"
#include	"Image.h"

namespace	DisplayOutput
{

/*
	CImage().

*/
CImage::CImage() :  m_Format( eImage_None ), m_Width(0), m_Height(0), m_nMipMaps(0), m_bRef( false )
{
}

/*
	CImage().

*/
void	CImage::Copy( const CImage &_image, const uint32 _mipLevel )
{
	g_Log->Info( "CImage( const CImage & )..." );

	if( _mipLevel == 0 )
	{
		m_Width = _image.m_Width;
		m_Height = _image.m_Width;
		m_nMipMaps = _image.m_nMipMaps;
		m_Format = _image.m_Format;
		m_bRef = _image.isReference();

		if( !m_bRef )
		{
			uint32	size = getMipMappedSize( 0, m_nMipMaps );
			m_spData = new Base::CAlignedBuffer( size );			
			if (m_spData->IsValid())
				memcpy( m_spData->GetBufferPtr(), _image.GetData( 0 ), size );
		}
	}
	else
	{
		m_Width = _image.GetWidth( _mipLevel );
		m_Height = _image.GetHeight( _mipLevel );
		m_nMipMaps = 1;//getNumberOfMipMapsFromDimesions();
		m_Format = _image.m_Format;
		m_bRef = false;

		if( !m_bRef )
		{
			uint32	size = getMipMappedSize( 0, m_nMipMaps );
			m_spData = new Base::CAlignedBuffer( size );
			if ( m_spData->IsValid() )
				memcpy( m_spData->GetBufferPtr(), _image.GetData( _mipLevel ), size );
		}
	}
}

/*
	CImage().

*/
void	CImage::Create( const uint32 _w, const uint32 _h, const eImageFormat _format, const bool _bMipmaps, const bool _bRef )
{
//	g_Log->Info( "Create( %d, %d, 0x%x )", _w, _h, _format );

	m_Width = _w;
	m_Height = _h;
	m_nMipMaps = _bMipmaps ? getNumberOfMipMapsFromDimesions() : 1;
	m_Format = _format;
	m_bRef = _bRef ;

	if( !m_bRef )
	{
		uint32	size = getMipMappedSize( 0, m_nMipMaps );
		m_spData  = new Base::CAlignedBuffer( size );
		memset( m_spData->GetBufferPtr(), 0, size );
	}
}

/*
	~CImage().

*/
CImage::~CImage()
{
//	g_Log->Info( "~CImage()..." );
}

/*
	GetData().

*/
uint8	*CImage::GetData( const uint32 _mipLevel ) const
{
	if( m_bRef && m_spData.IsNull() )
		return( NULL );
		
	const Base::CAlignedBuffer *ab = m_spData.GetRepPtr()->getRealPointer();

	if( _mipLevel == 0 )
		return( ab->GetBufferPtr() );

	return( _mipLevel < m_nMipMaps) ? ab->GetBufferPtr() + getMipMappedSize( 0, _mipLevel ) : NULL;
}

/*
	SetData().

*/
void	CImage::SetData( uint8* /*_pData*/ )
{
	//if( !m_bRef /*|| m_nMipMaps > 1*/ )
		//return;

	//m_pData = _pData;
}


/*
	GetPitch().

*/
uint32	CImage::GetPitch( const uint32 _level ) const
{
	return( GetWidth( _level ) * m_Format.getBPPixel() );
}


/*
*/
uint32 CImage::getMipMappedSize( const uint32 _firstMipMapLevel, const uint32 _nMipMapLevels ) const
{
	return( getMipMappedSize( _firstMipMapLevel, _nMipMapLevels, m_Format ) );
}

/*int Image::getPixelCount(const int firstMipMapLevel, int nMipMapLevels) const {
	int w = getWidth (firstMipMapLevel);
	int h = getHeight(firstMipMapLevel);
	int d = getDepth (firstMipMapLevel);
	int size = 0;
	while (nMipMapLevels){
		size += w * h * d;
		w >>= 1;
		h >>= 1;
		d >>= 1;
		if (w + h + d == 0) break;
		if (w == 0) w = 1;
		if (h == 0) h = 1;
		if (d == 0) d = 1;

		nMipMapLevels--;
	}

	return (depth == 0)? 6 * size : size;
}*/

/*
	getMipMappedSize().

*/
uint32 CImage::getMipMappedSize( const uint32 _firstMipMapLevel, const uint32 _nMipMapLevels, const CImageFormat &_format ) const
{
	uint32	w = GetWidth( _firstMipMapLevel ) << 1;
	uint32	h = GetHeight( _firstMipMapLevel ) << 1;

	uint32	level = 0;
	uint32	size = 0;

	while( level < _nMipMapLevels && (w != 1 || h != 1) )
	{
		if( w > 1 ) w >>= 1;
		if( h > 1 ) h >>= 1;

		if( _format.isCompressed() )
			size += ((w + 3) >> 2) * ((h + 3) >> 2);
		else
			size += w * h;

		level++;
	}

	if( _format.isCompressed() )
		size *= _format.getBPBlock();
	else
		size *= _format.getBPPixel();

	return( size );
}


/*
	getNumberOfMipMapsFromDimesions().

*/
uint32 CImage::getNumberOfMipMapsFromDimesions( void ) const
{
	uint32 m = (m_Width > m_Height)? m_Width : m_Height;
	uint32 i = 0;

	while( m > 0 )
	{
		m >>= 1;
		i++;
	}

	return( i );
}

/*
	Load().

*/
bool	CImage::Load( const std::string &_fileName, const bool _calcMipmaps )
{
	std::string ext = "";

	size_t offs = _fileName.find_last_of( '.', _fileName.size() );
	if( offs != _fileName.size() )
		ext = _fileName.substr( offs+1, _fileName.size()-1 );

	if( ext == "" )
	{
		g_Log->Warning( "CImage::Load() No extension found for %s", _fileName.c_str() );
		return( false );
	}

	bool	foundExt = false;

	//	DDS?
	if( ext == "dds" )
	{
		foundExt = true;
		if( !LoadDDS( _fileName, _calcMipmaps ) )
			return( false );
	}

	//	TGA?
	/*if( ext == "tga" )
	{
		foundExt = true;
		if( !LoadTGA( _fileName, _calcMipmaps ) )
			return( false );
	}*/

	//	JPG?
	/*if( ext == "jpg" )
	{
		foundExt = true;
		if( !LoadJPG( _fileName, _calcMipmaps ) )
			return( false );
	}*/

	//	PNG?
	if( ext == "png" )
	{
		foundExt = true;
		if( !LoadPNG( _fileName, _calcMipmaps ) )
			return( false );
	}

	//	Unknown extension?
	if( foundExt == false )
	{
		g_Log->Warning( "CImage::Load() Unknown extension for %s", _fileName.c_str() );
		return( false );
	}

	if( _calcMipmaps && !m_Format.isCompressed() )
	{
		if( !GenerateMipmaps() )
			return( false );
	}

	if( m_nMipMaps == 0 )
		m_nMipMaps = 1;

	//	Done!
	g_Log->Info( "CImage::Load( %s ): (%d x %d, %d MipMaps)", (const char *)_fileName.c_str(), m_Width, m_Height, m_nMipMaps );
	return( true );
}

/*
	GenerateMipmaps().

*/
bool	CImage::GenerateMipmaps( void )
{
	//	Check if the image is power of two.
	uint32 w = Base::Math::ClosestPowerOfTwo( m_Width );
	uint32 h = Base::Math::ClosestPowerOfTwo( m_Height );

	if( w != m_Width || h != m_Height)
	{
		//	TODO: resize?
		g_Log->Error( "CImage::GenerateMipmaps(): Image is not power of two!" );
		return( false );
	}

	m_nMipMaps = getNumberOfMipMapsFromDimesions();
	return( createMipMaps() );
}

/*
	Load().

*/
bool	CImage::Save( const std::string &_fileName )
{
	std::string ext = "";

	size_t offs = _fileName.find_last_of( '.', _fileName.size() );
	if( offs != _fileName.size() )
		ext = _fileName.substr( offs+1, _fileName.size()-1 );

	if( ext == "" )
	{
		g_Log->Warning( "CImage::Save() No extension found for %s", _fileName.c_str() );
		return( false );
	}

	//	DDS?
	if( ext == "dds" )
	{
		if( !SaveDDS( _fileName ) )
			return( false );
	}

	g_Log->Info( "CImage::Save( %s ): Complete!", (const char *)_fileName.c_str(), m_Width, m_Height, m_nMipMaps );
	return( true );
}

/*
	SaveDDS().

*/

#define DDPF_ALPHAPIXELS 0x00000001
#define DDPF_FOURCC      0x00000004
#define DDPF_RGB         0x00000040

#define DDSD_CAPS        0x00000001
#define DDSD_HEIGHT      0x00000002
#define DDSD_WIDTH       0x00000004
#define DDSD_PITCH       0x00000008
#define DDSD_PIXELFORMAT 0x00001000
#define DDSD_MIPMAPCOUNT 0x00020000
#define DDSD_LINEARSIZE  0x00080000
#define DDSD_DEPTH       0x00800000
#define DDSCAPS_COMPLEX  0x00000008
#define DDSCAPS_TEXTURE  0x00001000
#define DDSCAPS_MIPMAP   0x00400000

//
#pragma pack (push, 1)

struct DDSHeader {
	unsigned int ddsIdentifier;
	unsigned int size;
	unsigned int flags;
	unsigned int height;
	unsigned int width;
	unsigned int pitchOrLinearSize;
	unsigned int depth;
	unsigned int nMipMaps;
	unsigned int reserved[11];
	unsigned int size2;
	unsigned int flags2;
	unsigned int fourCC;
	unsigned int bpp;

	unsigned int rBitMask;
	unsigned int gBitMask;
	unsigned int bBitMask;
	unsigned int aBitMask;

	unsigned int caps1;
	unsigned int caps2;
	unsigned int reserved2[3];
};
#pragma pack (pop)

bool	CImage::SaveDDS( const std::string &_fileName )
{
	eImageFormat fmt = m_Format.getFormatEnum();

	if( (fmt < eImage_I8 || fmt > eImage_RGBA8 ) && fmt != eImage_RGB565 && !m_Format.isCompressed() )
		return( false );

	uint32 nChannels = m_Format.GetChannels();

	uint32	fourCC[] = {	MCHAR4('D','X','T','1'),
							MCHAR4('D','X','T','3'),
							MCHAR4('D','X','T','5'),
							MCHAR4('A','T','I','2')	};

	DDSHeader header = {
		MCHAR4('D','D','S',' '),
		124,
		DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | static_cast<unsigned int>(m_nMipMaps > 1? DDSD_MIPMAPCOUNT : 0),
		m_Height,
		m_Width,
		0,
		0,
		(m_nMipMaps > 1)? m_nMipMaps : 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		32,
		m_Format.isCompressed() ? DDPF_FOURCC : static_cast<unsigned int>((nChannels < 3)? 0x00020000 : DDPF_RGB) | static_cast<unsigned int>((nChannels & 1)? 0 : DDPF_ALPHAPIXELS),
		m_Format.isCompressed() ? fourCC[ fmt - eImage_DXT1 ] : 0,
		8 * nChannels,
		static_cast<unsigned int>((nChannels >= 3)? 0x00ff0000 : 0xFF),
		static_cast<unsigned int>((nChannels >= 3)? 0x0000ff00 : 0),
		static_cast<unsigned int>((nChannels >= 3)? 0x000000ff : 0),
		(nChannels >= 3)? 0xff000000 : (nChannels == 2)? 0xFF00 : 0,
		DDSCAPS_TEXTURE | static_cast<unsigned int>(m_nMipMaps > 1? DDSCAPS_MIPMAP | DDSCAPS_COMPLEX : 0),
		0,
		{ 0, 0, 0 },
	};

	FILE	*pFile = fopen( _fileName.c_str(), "wb" );
	if( !pFile )
		return( false );

	fwrite( &header, sizeof(header), 1, pFile );

	uint32	size = getMipMappedSize( 0, m_nMipMaps );

	bool	bAlpha = false;

	if( bAlpha && (fmt == eImage_RGB8 || fmt == eImage_RGBA8) )
		flipChannels( m_spData->GetBufferPtr(), size / nChannels, nChannels );

	fwrite( GetData( 0 ), size, 1, pFile );

	//	Flip back... =)
	if( bAlpha && (fmt == eImage_RGB8 || fmt == eImage_RGBA8) )
		flipChannels( m_spData->GetBufferPtr(), size / nChannels, nChannels );

	fclose( pFile );

	return( true );
}

/*
	buildMipMap8().

*/
void	buildMipMap8( uint8 *dest, uint8 *src, uint32 width, uint32 height, uint32 channels )
{
	uint32	xOff = (width  < 2)? 0 : channels;
	uint32	yOff = (height < 2)? 0 : width * channels;

	for( uint32 y=0; y<height; y += 2 )
	{
		for( uint32 x=0; x<width; x += 2 )
		{
			for( uint32 i=0; i<channels; i++ )
			{
				*dest++ = ((src[0] +  src[xOff] + src[yOff] + src[yOff + xOff]) + 2) >> 2;
				src++;
			}
			src += xOff;
		}
		src += yOff;
	}
}

/*
	buildMipMap32f().

*/
void	buildMipMap32f( fp4 *dest, fp4 *src, uint32 width, uint32 height, uint32 channels )
{
	uint32	xOff = (width  < 2)? 0 : channels;
	uint32	yOff = (height < 2)? 0 : width * channels;

	for( uint32 y=0; y<height; y += 2 )
	{
		for( uint32 x=0; x<width; x += 2 )
		{
			for( uint32 i=0; i<channels; i++ )
			{
				*dest++ = (src[0] + src[xOff] + src[yOff] + src[yOff + xOff]) * 0.25f;
				src++;
			}

			src += xOff;
		}

		src += yOff;
	}
}

/*
	buildMipMapRGB565().

*/
void	buildMipMapRGB565( uint16 *dest, uint16 *src, uint32 width, uint32 height )
{
	uint32	x,y,diff;
	uint32	xOff = (width  < 2)? 0 : 1;
	uint32	yOff = (height < 2)? 0 : 1;

	uint32	r,g,b;

	diff = yOff * width;

	for( y=0; y<height; y += 2 )
	{
		for( x=0; x<width; x += 2 )
		{
			r = (((src[0] >> 8) & 0xF8) + ((src[xOff] >> 8) & 0xF8) + ((src[diff] >> 8) & 0xF8) + ((src[diff + xOff] >> 8) & 0xF8));
			g = (((src[0] >> 3) & 0xFC) + ((src[xOff] >> 3) & 0xFC) + ((src[diff] >> 3) & 0xFC) + ((src[diff + xOff] >> 3) & 0xFC));
			b = (((src[0] << 3) & 0xF8) + ((src[xOff] << 3) & 0xF8) + ((src[diff] << 3) & 0xF8) + ((src[diff + xOff] << 3) & 0xF8));

			*dest++ = (uint16)(((r << 6) & 0xF800) | ((g << 1) & 0x07E0) | ((b >> 5) & 0x1F));
			src += 2;
		}

		src += width;
	}
}

/*
	createMipMaps().

*/
bool	CImage::createMipMaps( void )
{
	if( m_Format.isCompressed() )
		return( false );

	if( m_bRef )
		return( false );

	uint32	w = m_Width;
	uint32	h = m_Height;

	union {
		uint8	*src;
		uint16	*src16;
		fp4		*src32f;
	};

	union {
		uint8	*dest;
		uint16	*dest16;
		fp4		*dest32f;
	};

	if( m_nMipMaps <= 1 )
	{
		m_spData->Reallocate( getMipMappedSize() );
		m_nMipMaps = getNumberOfMipMapsFromDimesions();
	}

	dest = m_spData->GetBufferPtr();

	uint32	nChannels = m_Format.GetChannels();

	while( w > 1 || h > 1 )
	{
		src = dest;

		if( m_Format.isPlain() )
		{
			if( m_Format.isFloat() )
			{
				dest32f += w * h * nChannels;
				buildMipMap32f(dest32f, src32f, w, h, nChannels);
			}
			else
			{
				dest += w * h * nChannels;
				buildMipMap8(dest, src, w, h, nChannels);
			}
		}
		else
		{
			dest16 += w * h;
			buildMipMapRGB565(dest16, src16, w, h);
		}

		if( w > 1)	w >>= 1;
		if( h > 1)	h >>= 1;
	}

	return( true );
}

/*
*/
uint32 CImage::getNumPixels( const uint32 _firstMipMapLevel, const uint32 _nMipMapLevels ) const
{
	uint32	w = GetWidth( _firstMipMapLevel ) << 1;
	uint32	h = GetHeight( _firstMipMapLevel ) << 1;
	uint32	level = 0;
	uint32	size = 0;

	while( level < _nMipMapLevels && (w != 1 || h != 1) )
	{
		if( w > 1 ) w >>= 1;
		if( h > 1 ) h >>= 1;

		size += w * h;
		level++;
	}

	return( size );
}

/*
	Convert().

*/
bool	CImage::Convert( const eImageFormat _newFormatType )
{
	if( _newFormatType == m_Format.m_Format )
		return( false );

	if( !m_Format.isPlain() )
		return( false );

	if( m_bRef )
		return( false );

	//
	CImageFormat newFormat = CImageFormat( _newFormatType );
	if( !newFormat.isPlain() )
		return( false );

	uint8	*src = m_spData->GetBufferPtr();
	Base::spCAlignedBuffer newPixels = new Base::CAlignedBuffer( getMipMappedSize( 0, m_nMipMaps, newFormat ) );
	uint8	*dest = newPixels->GetBufferPtr();

	uint32	nPixels = getNumPixels( 0, m_nMipMaps );

	if( m_Format.is( eImage_RGB8 ) && newFormat.is( eImage_RGBA8 ) )
	{
		//	Fast path for RGB->RGBA8
		do
		{
			dest[0] = src[0];
			dest[1] = src[1];
			dest[2] = src[2];
			dest[3] = 255;
			dest += 4;
			src += 3;
		}	while ( --nPixels );
	}
	else
	{
		uint32	srcSize = m_Format.getBPPixel();
		uint32	nSrcChannels = m_Format.GetChannels();

		uint32	destSize = newFormat.getBPPixel();
		uint32	nDestChannels = newFormat.GetChannels();

		do
		{
			fp4	rgba[4];
            
            rgba[0] = rgba[1] = rgba[2] = rgba[3] = 0.0f;

			if( m_Format.isFloat() )
			{
				for( uint32 i=0; i<nSrcChannels; i++ )
					rgba[i] = ((fp4 *)src)[i];
			}
			else if( m_Format.m_Format >= eImage_I16 && m_Format.m_Format <= eImage_RGBA16 )
			{
				for( uint32 i=0; i<nSrcChannels; i++ )
					rgba[i] = ((uint16 *)src)[i] * (1.0f / 65535.0f);
			}
			else
			{
				for( uint32 i=0; i<nSrcChannels; i++ )
					rgba[i] = src[i] * (1.0f / 255.0f);
			}

			if( nSrcChannels  < 4 )	rgba[3] = 1.0f;
			if( nSrcChannels == 1 )	rgba[2] = rgba[1] = rgba[0];

			if( nDestChannels == 1 )
				rgba[0] = 0.30f * rgba[0] + 0.59f * rgba[1] + 0.11f * rgba[2];

			if( newFormat.isFloat() )
			{
				for( uint32 i=0; i<nDestChannels; i++ )
					((fp4 *)dest)[i] = rgba[i];
			}
			else if( newFormat.m_Format >= eImage_I16 && newFormat.m_Format <= eImage_RGBA16 )
			{
				for( uint32 i=0; i<nDestChannels; i++ )
					((uint16 *)dest)[i] = (uint16)(65535 * Base::Math::Clamped( rgba[i], 0.0f, 1.1f ) + 0.5f );
			}
			/*else if( newFormat == FORMAT_RGB10A2)
			{
			*(uint *) dest =
			(uint(1023.0f * saturate(rgba[0]) + 0.5f) << 22) |
			(uint(1023.0f * saturate(rgba[1]) + 0.5f) << 12) |
			(uint(1023.0f * saturate(rgba[2]) + 0.5f) <<  2) |
			(uint(   3.0f * saturate(rgba[3]) + 0.5f));
			}*/
			else
			{
				for( uint32 i=0; i<nDestChannels; i++ )
					dest[i] = (uint8)( 255 * Base::Math::Clamped( rgba[i], 0.0f, 1.0f ) + 0.5f );
			}

			src  += srcSize;
			dest += destSize;

		} while( --nPixels );
	}

	m_spData = newPixels;
	m_Format = newFormat;

	return( true );
}

static int32 icerp( int32 _a, int32 _b, int32 _c, int32 _d, int32 _x )
{
	int32 p = (_d - _c) - (_a - _b);
	int32 q = (_a - _b) - p;
	int32 r = _c - _a;
	return( (_x * (_x * (_x * p + (q << 7)) + (r << 14)) + (_b << 21)) >> 21 );
}


/*
	Scale().

*/
bool	CImage::Scale( const uint32 _newWidth, const uint32 _newHeight, const eScaleFilters _eFilter )
{
	if( !m_Format.isPlain() || m_Format.isFloat() )
	{
		if( m_Format.isCompressed() )
			g_Log->Warning( "CImage::Scale(): No deal, image is compressed." );

		if( m_Format.isFloat() )
			g_Log->Warning( "CImage::Scale(): No deal, image is float." );

		if( m_Format.isDepth() )
			g_Log->Warning( "CImage::Scale(): No deal, image is depth." );

		return( false );
	}
    
    if ( _newHeight < 2 || _newWidth < 2 )
    {
        g_Log->Warning( "CImage::Scale(): No deal, new size too small." );
        return ( false );
    }

	uint32 nChannels = m_Format.GetChannels();

	Base::spCAlignedBuffer newPixels = new Base::CAlignedBuffer( _newWidth * _newHeight * nChannels );
	
	uint8 *pData = GetData( 0 );

	uint32	x,y,k,sampleX, sampleY, wX, wY;
	uint8	*src, *dest = newPixels->GetBufferPtr();

	switch( _eFilter )
	{
		case	eImage_Nearest:
			for( y=0; y<_newHeight; y++ )
			{
				sampleY = (m_Height - 1) * y / (_newHeight - 1);
				for( x=0; x<_newWidth; x++ )
				{
					sampleX = (m_Width - 1) * x / (_newWidth - 1);
					for( k=0; k<nChannels; k++ )
					{
						*dest++ = pData[ (sampleY * m_Width + sampleX) * nChannels + k ];
					}
				}
			}
		break;

		case	eImage_Bilinear:
			for( y=0; y<_newHeight; y++ )
			{
				sampleY = (((m_Height - 1) * y) << 8) / (_newHeight - 1);

				if (y == _newHeight - 1)
					sampleY--;

				wY = sampleY & 0xFF;
				sampleY >>= 8;

				for( x=0; x<_newWidth; x++ )
				{
					sampleX = (((m_Width - 1) * x) << 8) / (_newWidth - 1);
					wX = sampleX & 0xFF;
					sampleX >>= 8;

					src = pData + (sampleY * m_Width + sampleX) * nChannels;

					for( k=0; k<nChannels; k++ )
					{
						*dest++ = ((	(256 - wX) * (256 - wY) * static_cast<uint32>(src[ 0 ]) +
									(256 - wX) * (      wY) * static_cast<uint32>(src[ m_Width * nChannels ]) +
									(      wX) * (256 - wY) * static_cast<uint32>(src[ nChannels ]) +
									(      wX) * (      wY) * static_cast<uint32>(src[ (m_Width + 1) * nChannels ]) ) >> 16) & 0xFF;
                        src++;
					}
				}
			}
		break;
		case	eImage_Bicubic:
			int a,b,c,d;
			int res;

			for( y=0; y<_newHeight; y++ )
			{
				sampleY = (((m_Height - 1) * y) << 7) / (_newHeight - 1);
				wY = sampleY & 0x7F;
				sampleY >>= 7;

				for( x=0; x<_newWidth; x++ )
				{
					sampleX = (((m_Width - 1) * x) << 7) / (_newWidth - 1);
					wX = sampleX & 0x7F;
					sampleX >>= 7;

					src = pData + ((sampleY - 1) * m_Width + (sampleX - 1)) * nChannels;

					for( k=0; k<nChannels; k++ )
					{
						b = icerp( src[ m_Width * nChannels], src[( m_Width + 1) * nChannels], src[(    m_Width + 2) * nChannels], src[(    m_Width + 3) * nChannels], static_cast<int32>(wX)) & 0xFF;
						if( sampleY > 0 )
							a = icerp(src[0], src[nChannels], src[2 * nChannels], src[3 * nChannels], static_cast<int32>(wX)) & 0xFF;
						else
							a = b;

						c = icerp(src[2 * m_Width * nChannels], src[(2 * m_Width + 1) * nChannels], src[(2 * m_Width + 2) * nChannels], src[(2 * m_Width + 3) * nChannels], static_cast<int32>(wX)) & 0xFF;
						if( sampleY < _newHeight - 1 )
							d = icerp(src[3 * m_Width * nChannels], src[(3 * m_Width + 1) * nChannels], src[(3 * m_Width + 2) * nChannels], src[(3 * m_Width + 3) * nChannels], static_cast<int32>(wX)) & 0xFF;
						else
							d = c;

						res = icerp( a, b, c, d, static_cast<int32>(wY) ) & 0xFF;
						*dest++ = (res < 0)? 0 : (res > 255)? 255 : (res & 0xFF);
						src++;
					}
				}
			}
		break;
	}

	m_spData = newPixels;
	m_Width  = _newWidth;
	m_Height = _newHeight;
	m_nMipMaps = 1;

	return( true );
}

/*
*/
void	CImage::PutPixel( const int32 _x, const int32 _y, const fp4 _r, const fp4 _g, const fp4 _b, const fp4 _a )
{
	//	Complicated formats are no go.
	if( !m_Format.isPlain() )
		return;

	if( _x < 0 || _x >= (int)m_Width )
		return;

	if( _y < 0 || _y >= (int)m_Height )
		return;

	fp4	rgba[4] = { _r, _g, _b, _a };

	uint32	nDestChannels = m_Format.GetChannels();
	uint8	*pData = (GetData(0) + (static_cast<uint32>(_y) * GetPitch())) + (static_cast<uint32>(_x) * m_Format.getBPPixel() );

	if( m_Format.isFloat() )
	{
		for( uint8 i=0; i<nDestChannels; i++ )
			((fp4 *)pData)[i] = rgba[i];
	}
	else if( m_Format.m_Format >= eImage_I16 && m_Format.m_Format <= eImage_RGBA16 )
	{
		for( uint8 i=0; i<nDestChannels; i++ )
			((uint16 *)pData)[i] = (uint16)(65535 * Base::Math::Clamped( rgba[i], 0.0f, 1.1f ) + 0.5f );
	}
	else
	{
		for( uint8 i=0; i<nDestChannels; i++ )
			pData[i] = (uint8)( 255 * Base::Math::Clamped( rgba[i], 0.0f, 1.0f ) + 0.5f );
	}
}

/*
*/
void	CImage::GetPixel( const int32 _x, const int32 _y, fp4 &_r, fp4 &_g, fp4 &_b, fp4 &_a )
{
	//	Complicated formats are no go.
	if( !m_Format.isPlain() )
		return;

	if( _x < 0 || _x > (int)m_Width )
		return;

	if( _y < 0 || _y > (int)m_Height )
		return;

	uint32	nSrcChannels = m_Format.GetChannels();
	uint8	*pData = (GetData(0) + (static_cast<uint32>(_y) * GetPitch())) + (static_cast<uint32>(_x) * m_Format.getBPPixel() );
	fp4		rgba[4];
    
    rgba[0] = rgba[1] = rgba[2] = rgba[3] = 0.0f;

	if( m_Format.isFloat() )
	{
		for( uint32 i=0; i<nSrcChannels; i++ )
			rgba[i] = ((fp4 *)pData)[i];
	}
	else if( m_Format.m_Format >= eImage_I16 && m_Format.m_Format <= eImage_RGBA16 )
	{
		for( uint32 i=0; i<nSrcChannels; i++ )
			rgba[i] = ((uint16 *)pData)[i] * (1.0f / 65535.0f);
	}
	else
	{
		for( uint32 i=0; i<nSrcChannels; i++ )
			rgba[i] = pData[i] * (1.0f / 255.0f);
	}

	if( nSrcChannels  < 4 )	rgba[3] = 1.0f;
	if( nSrcChannels == 1 )	rgba[2] = rgba[1] = rgba[0];

	_r = rgba[0];
	_g = rgba[1];
	_b = rgba[2];
	_a = rgba[3];
}



};

