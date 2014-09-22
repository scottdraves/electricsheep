#ifndef	_IMAGE_H_
#define	_IMAGE_H_

#include	"base.h"
#include	"Rect.h"
#include	"Log.h"
#include	"SmartPtr.h"
#include	"AlignedBuffer.h"

#ifdef LINUX_GNU
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __LITTLE_ENDIAN__ __LITTLE_ENDIAN
#undef __BIG_ENDIAN__
#else
#undef __LITTLE_ENDIAN__
#define __BIG_ENDIAN__ __BIG_ENDIAN
#endif
#endif

namespace	DisplayOutput
{

/*
	The imageformats are NOT flexible or changeable!
	If they are for some reason changed in any way, it¥s extremely important that the renderers conversion-tables are uptodate!
*/

enum	eImageFormat {

	eImage_None	= 0,

	//	Plain formats.
	eImage_I8,
	eImage_IA8,
	eImage_RGB8,
	eImage_RGBA8,

	eImage_I16,
	eImage_RG16,
	eImage_RGB16,
	eImage_RGBA16,

	eImage_I16F,
	eImage_RG16F,
	eImage_RGB16F,
	eImage_RGBA16F,

	eImage_I32F,
	eImage_RG32F,
	eImage_RGB32F,
	eImage_RGBA32F,

	//	Packed formats.
	eImage_RGBA4,
	eImage_RGB565,

	//	Compressed formats.
	eImage_DXT1,
	eImage_DXT3,
	eImage_DXT5,

	//	Depth formats.
	eImage_D16,
	eImage_D24,
};


enum	eScaleFilters	{
	eImage_Nearest = 0,
	eImage_Bilinear,
	eImage_Bicubic,
};

/*
*/
class	CImageFormat	{

	public:
			eImageFormat	m_Format;

			CImageFormat() : m_Format( eImage_None )	{};
			CImageFormat( eImageFormat _format ) : m_Format( _format )	{};
			CImageFormat( const CImageFormat &_image )	: m_Format( _image.m_Format )	{};

			//
			inline std::string GetDescription( void ) const
			{
				std::string desc = "";

				switch( m_Format )
				{
					case eImage_I8:			desc = "I8";		break;
					case eImage_I16:		desc = "I16";		break;
					case eImage_I16F:		desc = "I16F";		break;
					case eImage_I32F:		desc = "I32F";		break;
					case eImage_D16:		desc = "D16";		break;
					case eImage_D24:		desc = "D24";		break;
					case eImage_IA8:		desc = "IA8";		break;
					case eImage_RG16:		desc = "RG16";		break;
					case eImage_RG16F:		desc = "RG16F";		break;
					case eImage_RG32F:		desc = "RG32F";		break;
					case eImage_RGB8:		desc = "RGB8";		break;
					case eImage_RGB16:		desc = "RGB16";		break;
					case eImage_RGB16F:		desc = "RGB16F";	break;
					case eImage_RGB32F:		desc = "RGB32F";	break;
					case eImage_RGBA4:		desc = "RGBA4";		break;
					case eImage_RGB565:		desc = "RGB565";	break;
					case eImage_DXT1:		desc = "DXT1";		break;
					case eImage_RGBA8:		desc = "RGBA8";		break;
					case eImage_RGBA16:		desc = "RGBA16";	break;
					case eImage_RGBA16F:	desc = "RGBA16F";	break;
					case eImage_RGBA32F:	desc = "RGBA32F";	break;
					case eImage_DXT3:		desc = "DXT3";		break;
					case eImage_DXT5:		desc = "DXT5";		break;
					default:
						desc = "Unknown";	break;
				}

				return( desc );
			}

			//
			inline uint32 GetChannels( void ) const
			{
				switch( m_Format )
				{
					case eImage_I8:
					case eImage_I16:
					case eImage_I16F:
					case eImage_I32F:
					case eImage_D16:
					case eImage_D24:
									return( 1 );
					case eImage_IA8:
					case eImage_RG16:
					case eImage_RG16F:
					case eImage_RG32F:
					case eImage_RGBA4:
									return( 2 );
					case eImage_RGB8:
					case eImage_RGB16:
					case eImage_RGB16F:
					case eImage_RGB32F:
					case eImage_RGB565:
					case eImage_DXT1:
									return( 3 );
					case eImage_RGBA8:
					case eImage_RGBA16:
					case eImage_RGBA16F:
					case eImage_RGBA32F:
					case eImage_DXT3:
					case eImage_DXT5:
									return( 4 );
					default:
						g_Log->Error( "CImageFormat::GetChannels = fubar" );
						return( 0 );
				}
			}

			//	Bytes per block.
			inline uint32 getBPBlock( void ) const
			{
				return( (m_Format == eImage_DXT1 ) ? 8 : 16 );
			}

			//	Bytes per pixel.
			inline uint32 getBPPixel( void ) const
			{
				switch( m_Format )
				{
					case eImage_I8:
									return( 1 );
					case eImage_IA8:
					case eImage_I16:
					case eImage_I16F:
					case eImage_RGB565:
					case eImage_D16:
					case eImage_RGBA4:
									return( 2 );
					case eImage_RGB8:
					case eImage_D24:
									return( 3 );
					case eImage_RGBA8:
					case eImage_RG16:
					case eImage_RG16F:
					case eImage_I32F:
									return( 4 );
					case eImage_RGB16:
					case eImage_RGB16F:
									return( 6 );
					case eImage_RG32F:
									return( 8 );
					case eImage_RGBA16:
					case eImage_RGBA16F:
					case eImage_RGB32F:
									return( 12 );
					case eImage_RGBA32F:
									return( 16 );
					default:
							return(0);
				}
			}

			//
			inline bool is( const eImageFormat _format ) const	{	return( m_Format == _format );	};

			inline eImageFormat getFormatEnum( void ) const { return( m_Format ); }

			inline bool isPlain( void ) const		{	return( m_Format <= eImage_RGBA32F );	}
			inline bool isPacked( void ) const		{	return( m_Format == eImage_RGB565 );	}
			inline bool isCompressed(void) const	{	return( (m_Format >= eImage_DXT1) && ( m_Format <= eImage_DXT5 ) );	}
			inline bool isFloat(void) const			{	return( (m_Format >= eImage_I16F) && ( m_Format <= eImage_RGBA32F) );	}
			inline bool isDepth(void) const			{	return( (m_Format >= eImage_D16) && (m_Format <= eImage_D24) );	}
};

MakeSmartPointers( CImageFormat );


//
inline void flipChannels( uint8 *_pData, uint32 _nPixels, const uint32 _nChannels )
{
	uint8	tmp;

	do {
		tmp = _pData[_nChannels - 1];
		
		for ( uint32 i = _nChannels - 1; i > 0; i-- )
		{
			_pData[i] = _pData[i - 1];
		}
		
		_pData[0] = tmp;
		
		_pData += _nChannels;
	} while (--_nPixels);
}

inline void flipChannelsRB( uint8 *_pData, uint32 _nPixels, const uint32 _nChannels )
{
	uint8	tmp;

	do {
		tmp = _pData[0];
		_pData[0] = _pData[2];
		_pData[2] = tmp;
		_pData += _nChannels;
	} while (--_nPixels);
}

#if (defined(MAC) || defined(LINUX_GNU)) 
#ifdef __LITTLE_ENDIAN__
	#define MCHAR4(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))
#else
	#define MCHAR4(a, b, c, d) (d | (c << 8) | (b << 16) | (a << 24))
#endif
#else
#define MCHAR4(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))
#endif

#define MAX_MIPMAP (sizeof(uint32) * 8)

/*
	CImage.
	Image class.
*/
class	CImage	{

	uint32 getNumberOfMipMapsFromDimesions( void ) const;
	uint32 getMipMappedSize( const uint32 _firstMipMapLevel, const uint32 _nMipMapLevels, const CImageFormat &_format ) const;
	uint32 getNumPixels( const uint32 _firstMipMapLevel, const uint32 _nMipMapLevels ) const;
	bool	 createMipMaps( void );

	//	Image loaders.
	bool	LoadDDS( const std::string &_fileName, const bool _wantMipMaps = true );
	//bool	LoadTGA( const std::string &_fileName, const bool _wantMipMaps = true );
	bool	LoadPNG( const std::string &_fileName, const bool _wantMipMaps = true );
	//bool	LoadJPG( const std::string &_fileName, const bool _wantMipMaps = true );

	//	Image savers.
	bool	SaveDDS( const std::string &_fileName );

	protected:

		CImageFormat	m_Format;

		Base::spCAlignedBuffer m_spData;

		uint32			m_Width;
		uint32			m_Height;

		uint32			m_nMipMaps;

		bool			m_bRef;

	public:
			CImage();
			~CImage();

			//
			void	Create( const uint32 _w, const uint32 _h, const eImageFormat _format, const bool _bMipmaps = false, const bool _bRef = false );
			void	Copy( const CImage &_image, const uint32 _mipLevel );

			bool	Load( const std::string &_filename, const bool _calcMimaps = true );
			bool	Save( const std::string &_filename );
			
			Base::spCAlignedBuffer& GetStorageBuffer( void ) { return m_spData; }
			void	SetStorageBuffer( Base::spCAlignedBuffer &buffer ) { m_spData = buffer; }

			uint8	*GetData( const uint32 _mipLevel ) const;
			void	SetData( uint8	*_pData );

			uint32	GetPitch( const uint32 _mipMapLevel = 0 ) const;

			//
			bool	Scale( const uint32 _newWidth, const uint32 _newHeight, const eScaleFilters _eFilter );
			bool	Convert( const eImageFormat _newFormat );

			//
			bool	GenerateMipmaps( void );

			//
			inline bool			isReference( void )								const	{ return( m_bRef );	};
			inline uint32		GetWidth( const uint32 _mipMapLevel = 0 )		const	{ uint32 a = m_Width >> _mipMapLevel; return( (a == 0)? 1 : a ); }
			inline uint32		GetHeight( const uint32 _mipMapLevel = 0 )		const	{ uint32 a = m_Height >> _mipMapLevel; return( (a == 0)? 1 : a );	}
			inline Base::Math::CRect	GetRect( void )							const	{ return( Base::Math::CRect( (fp4)m_Width, (fp4)m_Height ) );	};
			inline uint32		GetNumMipMaps( void )							const	{ return( m_nMipMaps ); };
			uint32		getMipMappedSize( const uint32 _firstMipMapLevel = 0, const uint32 _nMipMapLevels = 0x7FFFFFFF ) const;
			inline const CImageFormat	&GetFormat( void )						const	{ return( m_Format ); };

			//
			void	GetPixel( const int32 _x, const int32 _y, fp4 &_r, fp4 &_g, fp4 &_b, fp4 &_a );
			void	PutPixel( const int32 _x, const int32 _y, const fp4 _r, const fp4 _g, const fp4 _b, const fp4 _a );
};

MakeSmartPointers( CImage );

};

#endif
