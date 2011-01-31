#ifdef _MSC_VER
#ifdef WIN32
#ifndef _TEXTUREFLATDS_H
#define _TEXTUREFLATDS_H

#include "TextureFlat.h"

namespace	DisplayOutput
{

class CTextureFlatDS : public CTextureFlat
{
	LPDIRECTDRAWSURFACE7	m_pTextureDS;

	//	Internal to keep track if size or format changed.
	Base::Math::CRect	m_Size;
	DisplayOutput::eImageFormat	m_Format;
	CBackBufDD *m_BackBuffer;
	HDC	m_hdc;
	HDC m_sdc;
	public:
			CTextureFlatDS(CBackBufDD *backbuffer = NULL, const uint32 _flags = 0 );
			virtual ~CTextureFlatDS();

			virtual bool	Upload( spCImage _spImage );
			virtual bool	Bind( const uint32 _index );
			virtual bool	Unbind( const uint32 _index );
};

MakeSmartPointers( CTextureFlatDS );

}

#endif
#endif
#endif