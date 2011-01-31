#include	<stdint.h>
#include	<algorithm>
#include	"Rect.h"

namespace	Base
{

namespace	Math
{

/*
*/
CRect	&CRect::Normalize( void )
{
	fp4 t;
	if( m_X0 > m_X1 )
	{
		t = m_X0;
		m_X0 = m_X1;
		m_X1 = t;
	}
	if( m_Y0 > m_Y1 )
	{
		t = m_Y0;
		m_Y0 = m_Y1;
		m_Y1 = t;
	}

	return( *this );
}

/*
*/
CRect	CRect::Intersection( const CRect &_r ) const
{
	CRect x( std::max( m_X0, _r.m_X0 ),
			 std::max( m_Y0, _r.m_Y0 ),
			 std::min( m_X1, _r.m_X1 ),
			 std::min( m_Y1, _r.m_Y1 ) );

	if( !x.IsNormalized() )
		x = CRect();

	return( x );
}

//
CRect	CRect::Union( const CRect &_r )	const
{
	CRect x( std::min( m_X0, _r.m_X0 ),
			 std::min( m_Y0, _r.m_Y0 ),
			 std::max( m_X1, _r.m_X1 ),
			 std::max( m_Y1, _r.m_Y1 ) );

	if( !x.IsNormalized() )
		x = CRect();

	return( x );
}



};

};
