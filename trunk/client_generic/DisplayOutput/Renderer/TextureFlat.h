#ifndef _TEXTUREFLAT_H
#define _TEXTUREFLAT_H

#include "Texture.h"
#include "Image.h"
#include "AlignedBuffer.h"

namespace	DisplayOutput
{

/*
	CTextureFlat.

*/
class CTextureFlat : public CTexture
{
	protected:
		spCImage			m_spImage;
		bool				m_bDirty;
		Base::Math::CRect	m_texRect;
		Base::spCAlignedBuffer		m_bufferCache;

	public:
			CTextureFlat( const uint32 _flags = 0 );
			virtual ~CTextureFlat();

			virtual bool	Reupload( void ) { return Upload(m_spImage); };
			virtual	bool	Upload( spCImage _spImage ) = PureVirtual;
			virtual	bool	Bind( const uint32 _index ) = PureVirtual;
			virtual	bool	Unbind( const uint32 _index ) = PureVirtual;

			virtual bool	Dirty( void )	{	return m_bDirty;	};
	
			virtual Base::Math::CRect&	GetRect( void ) 
			{ 
				return m_texRect; 
			};
			virtual void				SetRect( const Base::Math::CRect& _rect ) 
			{ 
				m_texRect = _rect; 
			};
};

MakeSmartPointers( CTextureFlat );

}

#endif
