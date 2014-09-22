#include	<stdint.h>
#include	"base.h"
#include	"Image.h"

namespace	DisplayOutput
{

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

/*
	LoadDDS().

*/
bool	CImage::LoadDDS( const std::string &_fileName, const bool _wantMipMaps )
{
	DDSHeader	header;

	FILE *pFileData = fopen( _fileName.c_str(), "rb" );
	if( !pFileData )
	{
		ThrowStr( ("CImage::LoadDDS() Unable to open " + _fileName).c_str() );
		return( false );
	}

	fread( &header, sizeof(header), 1, pFileData );
	if( header.ddsIdentifier != MCHAR4('D', 'D', 'S', ' ') )
	{
		ThrowStr( ("CImage::LoadDDS() No valid header in " + _fileName) );
		return( false );
	}

	m_Width    = header.width;
	m_Height   = header.height;

	m_nMipMaps = header.nMipMaps;

	if( m_nMipMaps <= 0 || !_wantMipMaps )
		m_nMipMaps = 1;
	else
		m_nMipMaps = getNumberOfMipMapsFromDimesions();

	switch( header.fourCC )
	{
		case MCHAR4('D', 'X', 'T', '1'):
			m_Format = CImageFormat( eImage_DXT1 );
			break;
		case MCHAR4('D', 'X', 'T', '3'):
			m_Format = CImageFormat( eImage_DXT3 );
			break;
		case MCHAR4('D', 'X', 'T', '5'):
			m_Format = CImageFormat( eImage_DXT5 );
			break;
		default:
			switch( header.bpp )
			{
				case 8:
					m_Format = CImageFormat( eImage_I8 );
					break;
				case 16:
					if( header.aBitMask )
						m_Format = CImageFormat( eImage_IA8 );
					else
						m_Format = CImageFormat( eImage_I16 );
					break;
				case 24:
					m_Format = CImageFormat( eImage_RGB8 );
					break;
				case 32:
					m_Format = CImageFormat( eImage_RGBA8 );
					break;
				default:
					return( false );
			}
	}

	//	Calculate how large buffer we need possibly including mipmaps.
	uint32 readSize = getMipMappedSize( 0, m_nMipMaps );
	uint32 size;

	if( m_Format.isPlain() && _wantMipMaps )
		size = getMipMappedSize( 0, 0x7fffffff );
	else
		size = readSize;

	m_spData = new Base::CAlignedBuffer( size );
	fread( m_spData->GetBufferPtr(), readSize, 1, pFileData );

	/*if( m_Format.is( eImage_RGB8 ) || m_Format.is( eImage_RGBA8 ) )
	{
		int nChannels = m_Format.GetChannels();
		flipChannels( m_pData, readSize / nChannels, nChannels );
	}*/

	fclose( pFileData );

	return( true );
}
};
