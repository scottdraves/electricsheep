#include <assert.h>
#include <inttypes.h>
#include <string.h>
#ifndef LINUX_GNU
#include "GLee.h"
#else
#include <GLee.h>
#endif
#ifdef MAC
#include <OpenGL/CGLMacro.h>
//#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include "base.h"
#include "Log.h"
#include "MathBase.h"
#include "Exception.h"
#include "DisplayOutput.h"
#include "RendererGL.h"
#include "TextureFlatGL.h"

namespace	DisplayOutput
{

//	These map 1:1 with eImageFormat in Image.h... *Keep them in sync!!!*
static const GLint internalFormats[] =
{
	0,


	GL_INTENSITY8,
	GL_LUMINANCE8_ALPHA8,
	GL_RGB8,
#ifdef MAC
	GL_RGBA,
#else
	GL_RGBA8,
#endif

	GL_INTENSITY16,
	GL_LUMINANCE16_ALPHA16,
	GL_RGB16,
	GL_RGBA16,

	GL_INTENSITY_FLOAT16_ATI,
	GL_LUMINANCE_ALPHA_FLOAT16_ATI,
	GL_RGB_FLOAT16_ATI,
	GL_RGBA_FLOAT16_ATI,

	GL_INTENSITY_FLOAT32_ATI,
	GL_LUMINANCE_ALPHA_FLOAT32_ATI,
	GL_RGB_FLOAT32_ATI,
	GL_RGBA_FLOAT32_ATI,

	GL_RGBA4,
	GL_RGB5,

	GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,

	GL_DEPTH_COMPONENT16,
	GL_DEPTH_COMPONENT24,
};

//
static const GLenum srcTypes[] =
{
	0,

	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
#ifdef MAC
	GL_UNSIGNED_INT_8_8_8_8_REV,
#else
	GL_UNSIGNED_BYTE,
#endif

	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,

	0,//GL_HALF_FLOAT_ARB,
	0,//GL_HALF_FLOAT_ARB,
	0,//GL_HALF_FLOAT_ARB,
	0,//GL_HALF_FLOAT_ARB,

	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,

	GL_UNSIGNED_SHORT_4_4_4_4_REV,
	GL_UNSIGNED_SHORT_5_6_5,

	0,
	0,
	0,

	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_INT,
};

/*
*/
#ifdef MAC
CTextureFlatGL::CTextureFlatGL( const uint32 _flags, CGLContextObj glCtx ) : CTextureFlat( _flags )
#else
CTextureFlatGL::CTextureFlatGL( const uint32 _flags ) : CTextureFlat( _flags )
#endif
{
	m_TexTarget = GL_TEXTURE_2D;
	
#ifdef MAC
	cgl_ctx = glCtx;//CGLGetCurrentContext();

	if ( _flags & kRectTexture )
		m_TexTarget = GL_TEXTURE_RECTANGLE_EXT;
#endif
	
	glGenTextures( (GLsizei)1, &m_TexID );
	VERIFYGL;
}

/*
*/
CTextureFlatGL::~CTextureFlatGL()
{
	glDeleteTextures( 1, &m_TexID );
	VERIFYGL;
}

/*
*/
bool	CTextureFlatGL::Upload( spCImage _spImage )
{
    m_spImage = _spImage;

	if (m_spImage==NULL) return false;

	CImageFormat	format = _spImage->GetFormat();

	static const GLenum srcFormats[] =
	{
		0,
		GL_LUMINANCE,
		GL_LUMINANCE_ALPHA,
		GL_RGB,
#ifdef MAC
		GL_BGRA
#else
		GL_RGBA
#endif
	};

	GLenum srcFormat = srcFormats[ format.GetChannels() ];
	GLenum srcType = srcTypes[ format.getFormatEnum() ];
	GLint internalFormat = internalFormats[ format.getFormatEnum() ];

	if( format.isFloat() )
		internalFormat = internalFormats[ format.getFormatEnum() - (eImage_RGBA32F - eImage_I16F)];

	glBindTexture( m_TexTarget, m_TexID );

	glTexParameteri( m_TexTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( m_TexTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Set filter modes.
	glTexParameteri( m_TexTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( m_TexTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	// Upload it all
	uint8	*pSrc;
	uint32 mipMapLevel = 0;
	while( (pSrc = _spImage->GetData( mipMapLevel ) ) != NULL )
	{
		if( format.isCompressed() )
		{
			// does the glCompressedTexImage2DARB need also power-of-two sized texture???
			glCompressedTexImage2DARB( m_TexTarget, mipMapLevel, internalFormat, _spImage->GetWidth( mipMapLevel ), _spImage->GetHeight( mipMapLevel ), 0, _spImage->getMipMappedSize( mipMapLevel, 1 ), pSrc );
			
			if (mipMapLevel == 0)
				SetRect( Base::Math::CRect( 1, 1 ) );
		}
		else
		{
			uint32 imgWidth = _spImage->GetWidth( mipMapLevel );
			uint32 imgHeight = _spImage->GetHeight( mipMapLevel );
						
			uint32 texWidth, texHeight;
			
#ifdef MAC
			GLint save2,
			save3,
			save4,
			save5;			
			
			glGetIntegerv(GL_UNPACK_ALIGNMENT, &save2);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glGetIntegerv(GL_UNPACK_ROW_LENGTH, &save3);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, _spImage->GetPitch() / 4);
			glGetIntegerv(GL_UNPACK_CLIENT_STORAGE_APPLE, &save4);
			glGetIntegerv(GL_TEXTURE_STORAGE_HINT_APPLE, &save5);
#endif
			
#ifndef LINUX_GNU
			if( GLEE_ARB_texture_non_power_of_two || m_TexTarget == GL_TEXTURE_RECTANGLE_EXT )
#else
			if( GLEE_ARB_texture_non_power_of_two || m_TexTarget == GL_TEXTURE_RECTANGLE_ARB )

#endif
			{
				texWidth = imgWidth;
				texHeight = imgHeight;
			}
			else
			{
				texWidth = Base::Math::UpperPowerOfTwo( _spImage->GetWidth( mipMapLevel ) );
				texHeight = Base::Math::UpperPowerOfTwo( _spImage->GetHeight( mipMapLevel ) );
			}
			
			if ( texWidth == imgWidth && texHeight == imgHeight )
			{
#ifdef MAC
				//glTexParameteri(m_TexTarget, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_SHARED_APPLE);
				glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
#endif
				glTexImage2D( m_TexTarget, mipMapLevel, internalFormat, texWidth, texHeight, 0, srcFormat, srcType, pSrc );
			
				if ( mipMapLevel == 0 )
				{
#ifndef LINUX_GNU
					if ( m_TexTarget == GL_TEXTURE_RECTANGLE_EXT )
#else 
					if ( m_TexTarget == GL_TEXTURE_RECTANGLE_ARB )
#endif
						SetRect( Base::Math::CRect( texWidth, texHeight ) );
					else
						SetRect( Base::Math::CRect( 1, 1 ) );
				}
			}
			else
			{
				glTexImage2D( m_TexTarget, mipMapLevel, internalFormat, texWidth, texHeight, 0, srcFormat, srcType, NULL );
				glTexSubImage2D( m_TexTarget, mipMapLevel, 0, 0, _spImage->GetWidth( mipMapLevel ), _spImage->GetHeight( mipMapLevel ), srcFormat, srcType, pSrc );
			
				if (mipMapLevel == 0)
				{
#ifndef LINUX_GNU
					if ( m_TexTarget == GL_TEXTURE_RECTANGLE_EXT )
#else 
					if ( m_TexTarget == GL_TEXTURE_RECTANGLE_ARB )
#endif
						SetRect( Base::Math::CRect( (fp4)imgWidth,  (fp4)imgHeight ) );
					else
						SetRect( Base::Math::CRect( (fp4)imgWidth / (fp4)texWidth,  (fp4)imgHeight / (fp4)texHeight ) );
				}
			}
			
#ifdef MAC
			glTexParameteri(m_TexTarget, GL_TEXTURE_STORAGE_HINT_APPLE , save5);
			glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, save4);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, save3);
			glPixelStorei(GL_UNPACK_ALIGNMENT, save2);
#endif
		}
				
		m_bufferCache = _spImage->GetStorageBuffer();
					
		mipMapLevel++;
	}

	m_bDirty = true;

	VERIFYGL;

	glBindTexture( m_TexTarget, 0 );
	
	return true;
}

/*
*/
bool	CTextureFlatGL::Bind( const uint32 _index )
{
	glActiveTextureARB( GL_TEXTURE0 + _index );
	glEnable( m_TexTarget );
	glBindTexture( m_TexTarget, m_TexID );
	
	m_bDirty = false;
	
	VERIFYGL;
	return true;
}

/*
*/
bool	CTextureFlatGL::Unbind( const uint32 _index )
{
	glActiveTextureARB( GL_TEXTURE0 + _index );
	glBindTexture( m_TexTarget, 0 );
	glDisable( m_TexTarget );
	VERIFYGL;
	return true;
}

}
