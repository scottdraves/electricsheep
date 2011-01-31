#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "base.h"

namespace	DisplayOutput
{

/**
	CTexture.

*/
class CTexture
{
	protected:
		uint32	m_Flags;

	public:
			CTexture( const uint32 _flags = 0 );
			virtual ~CTexture();

			virtual	bool	Bind( const uint32 _index ) = PureVirtual;
			virtual	bool	Unbind( const uint32 _index ) = PureVirtual;

			virtual bool	Dirty( void )	{	return false;	};
};

MakeSmartPointers( CTexture );

}

#endif
