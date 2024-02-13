#include	<stdint.h>
#include	"base.h"
#include	"Log.h"
#include	"Image.h"
#include	"png.h"

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
	LoadPNG().

*/
bool	CImage::LoadPNG( const std::string &_fileName, const bool _wantMipMaps )
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	FILE *file;

	png_byte pbSig[8];
	int iBitDepth, iColorType;
	double dGamma;
	png_color_16 *pBackground;
	png_byte **ppbRowPointers;

	// open the PNG input file
	if( (file = fopen( _fileName.c_str(), "rb")) == NULL )
	{
		g_Log->Warning( "Unable to open %s", _fileName.c_str() );
		return( false );
	}

	// first check the eight byte PNG signature
	size_t signature_nmemb_read = fread(pbSig, 1, 8, file);
    if (signature_nmemb_read < 8) {
      g_Log->Warning("Failed to read PNG signature from %s", _fileName.c_str());
      return false;
    }
	if( png_sig_cmp( pbSig, 0, 8 ) != 0)
	{
		g_Log->Warning( "%s doesn't have a valid png signature...", _fileName.c_str() );
		fclose( file );
		return( false );
	}

	// create the two png(-info) structures
	if( (png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr) NULL, (png_error_ptr) NULL) ) == NULL )
	{
		fclose( file );
		return( false );
	}

	if( (info_ptr = png_create_info_struct(png_ptr)) == NULL )
	{
		png_destroy_read_struct( &png_ptr, NULL, NULL );
		fclose( file );
		return( false );
	}

	// initialize the png structure
	png_init_io( png_ptr, file );
	png_set_sig_bytes( png_ptr, 8 );

	// read all PNG info up to image data
	png_read_info( png_ptr, info_ptr );

	// get width, height, bit-depth and color-type
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) &m_Width, (png_uint_32 *) &m_Height, &iBitDepth, &iColorType, NULL, NULL, NULL);

	// expand images of all color-type and bit-depth to 3x8 bit RGB images
	// let the library process things like alpha, transparency, background
	if( iBitDepth == 16 )									png_set_strip_16( png_ptr );
	if( iColorType == PNG_COLOR_TYPE_PALETTE )				png_set_expand( png_ptr );
	if( iBitDepth < 8 )										png_set_expand( png_ptr );
	if( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) )	png_set_expand( png_ptr );

	// set the background color to draw transparent and alpha images over.
	if( png_get_bKGD( png_ptr, info_ptr, &pBackground ) )
	png_set_background( png_ptr, pBackground, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0 );

	// if required set gamma conversion
	if( png_get_gAMA( png_ptr, info_ptr, &dGamma ) )
	png_set_gamma( png_ptr, 2.2, dGamma );

	//if( !useRGBA )
	//	png_set_bgr( png_ptr );

	// after the transformations have been registered update info_ptr data
	png_read_update_info( png_ptr, info_ptr );

	// get again width, height and the new bit-depth and color-type
	png_get_IHDR( png_ptr, info_ptr, (png_uint_32 *) &m_Width, (png_uint_32 *) &m_Height, &iBitDepth, &iColorType, NULL, NULL, NULL );
	uint32 nChannels = png_get_channels( png_ptr, info_ptr );

	switch( nChannels )
	{
		case 1:	m_Format = CImageFormat( eImage_I8 );		break;
		case 2:	m_Format = CImageFormat( eImage_IA8 );		break;
		case 3:	m_Format = CImageFormat( eImage_RGB8 );		break;
		case 4:	m_Format = CImageFormat( eImage_RGBA8 );	break;
	}

	if( !_wantMipMaps )
		m_nMipMaps = 1;
	else
		m_nMipMaps = getNumberOfMipMapsFromDimesions();

	// now we can allocate memory to store the image
	//m_pData = new uint8[ getMipMappedSize( 0, _wantMipMaps ? 0x7fffffff : 1 ) ];
	m_spData = new Base::CAlignedBuffer( getMipMappedSize( 0, m_nMipMaps ) );

	// set the individual row-pointers to point at the correct offsets
	ppbRowPointers = new png_bytep[ m_Height ];
	for( uint32 i=0; i<m_Height; i++ )
		ppbRowPointers[i] = m_spData->GetBufferPtr() + i * m_Width * nChannels;

	// now we can go ahead and just read the whole image
	png_read_image( png_ptr, ppbRowPointers );

	// read the additional chunks in the PNG file (not really needed)
	png_read_end( png_ptr, NULL );

#if defined(MAC) && !defined(__BIG_ENDIAN__)
	if (nChannels < 3) {
		g_Log->Warning( "bad number of channels %d ...", nChannels );
	} else {
	  for( uint32 i=0; i<m_Height; i++ )
	    {
	      flipChannelsRB( ppbRowPointers[i], m_Width, nChannels );
	    }
	}
#endif

#if (defined(MAC)||defined(LINUX_GNU)) && defined(__BIG_ENDIAN__)
	for( uint32 i=0; i<m_Height; i++ )
	{		
		flipChannels( ppbRowPointers[i], m_Width, nChannels );
	}
#endif

	SAFE_DELETE_ARRAY( ppbRowPointers );

	// and we're done
	png_destroy_read_struct( &png_ptr, &info_ptr, NULL );

	fclose( file );

	//int numChannels = m_Format.GetChannels();
	//flipChannels( m_pData, getMipMappedSize(0,m_nMipMaps) / numChannels, numChannels );

	return( true );
}

};
