#ifndef _TEXTUREFLATGL_H
#define _TEXTUREFLATGL_H

#include "TextureFlat.h"

namespace	DisplayOutput
{

const uint32 kRectTexture = 0x80000000;

/*
	CTextureFlatGL.

*/
class CTextureFlatGL : public CTextureFlat
{
	GLuint	m_TexID;
	
	GLenum	m_TexTarget;
	
#ifdef MAC
	CGLContextObj cgl_ctx;
#endif
	

	public:
			CTextureFlatGL( const uint32 _flags = 0 );
			virtual ~CTextureFlatGL();

			bool	Upload( spCImage _spImage );
			bool	Bind( const uint32 _index );
			bool	Unbind( const uint32 _index );
};

MakeSmartPointers( CTextureFlatGL );

}

#endif
