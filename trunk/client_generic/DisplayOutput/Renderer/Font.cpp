#include	<string>
#include	"base.h"
#include	"Font.h"
#include	"Log.h"

class CFont;

namespace	DisplayOutput
{


/*
*/
CFontDescription::CFontDescription() :
m_Height( 10 ), m_Style( Normal ), m_bItalic( false ), m_bUnderline( false ), m_bAntiAliased( true ), m_TypeFace( "Trebuchet MS" )
{
}

/*
*/
CFontDescription::~CFontDescription()
{
}

/*
*/
bool CFontDescription::operator == (const CFontDescription &_rhs ) const
{
	return( (m_Height == _rhs.m_Height) && (m_Style == _rhs.m_Style) && (m_bItalic == _rhs.m_bItalic) &&
			(m_bUnderline == _rhs.m_bUnderline) && (m_bAntiAliased == _rhs.m_bAntiAliased) &&
			(m_TypeFace == _rhs.m_TypeFace) );
}


/*
	CBaseFont().

*/
CBaseFont::CBaseFont()
{
	m_FontDescription.TypeFace( "..." );
}

/*
	~CBaseFont().

*/
CBaseFont::~CBaseFont()
{
}

};
