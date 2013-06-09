#include <assert.h>
#include <inttypes.h>
#include <string>

#include "base.h"
#include "Log.h"
#include "MathBase.h"
#include "Exception.h"
#include "DisplayOutput.h"
#include "TextureFlat.h"

namespace	DisplayOutput
{

/*
*/
CTextureFlat::CTextureFlat( const uint32 _flags ) : CTexture( _flags ), m_spImage( NULL ), m_bDirty(false), m_texRect( Base::Math::CRect( 1, 1 ) )
{
}

/*
*/
CTextureFlat::~CTextureFlat()
{
}

}
