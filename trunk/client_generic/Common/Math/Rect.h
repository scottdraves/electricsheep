#ifndef	_RECT_H
#define	_RECT_H

#include	"base.h"
#include	"Vector2.h"

namespace	Base
{
namespace	Math
{

/*
	CRect.

*/
class CRect
{
	public:
			CRect();
			CRect( const CRect& r );
			CRect( fp4 _w, fp4 _h );
			CRect( fp4 _x0, fp4 _y0, fp4 _x1, fp4 _y1 );
			CRect( const CVector2 &_a, const CVector2 &_b );

			//	'IsNull' is not very clear, should be 'IsInvalid' or 'IsSingularity' or something like that.
			bool	IsNull() const;

			//
			fp4		Width() const;
			fp4		Height() const;

			//	Force integer.
			int32	iWidth() const;
			int32	iHeight() const;

			//
			fp4		Aspect( void ) const;
			fp4		Area( void ) const;
			uint32	iArea( void ) const;

			//
			bool	IsNormalized() const;
			CRect	&Normalize();

			//	Boolean things.
			CRect	Intersection( const CRect &_r ) const;
			CRect	Union( const CRect &_r ) const;
			bool	Surrounds( const CRect &_r ) const;
			bool	Inside( const CVector2 &_p ) const;

			//
			fp4 m_X0, m_Y0;
			fp4	m_X1, m_Y1;
};


/*
*/
inline	CRect::CRect() : m_X0( 0.0f ), m_Y0( 0.0f ), m_X1( 0.0f ), m_Y1( 0.0f )
{
}

/*
*/
inline CRect::CRect( const CRect &_r ) : m_X0( _r.m_X0 ), m_Y0( _r.m_Y0 ), m_X1( _r.m_X1 ), m_Y1( _r.m_Y1 )
{
}

/*
*/
inline	CRect::CRect( fp4 _w, fp4 _h ) : m_X0(0), m_Y0(0), m_X1(_w), m_Y1(_h)
{
}

/*
*/
inline	CRect::CRect( fp4 _x0, fp4 _y0, fp4 _x1, fp4 _y1 ) : m_X0(_x0), m_Y0(_y0), m_X1(_x1), m_Y1(_y1)
{
}

/*
*/
inline CRect::CRect( const CVector2 &_a, const CVector2 &_b ) : m_X0( _a.m_X ), m_Y0( _a.m_Y ), m_X1( _b.m_X ), m_Y1( _b.m_Y )
{
}

/*
*/
inline bool CRect::IsNull() const
{
	return( (m_X0 == m_X1) && (m_Y0 == m_Y1) );
}

/*
*/
inline fp4 CRect::Width() const
{
	return( m_X1 - m_X0 );
}

/*
*/
inline fp4 CRect::Height() const
{
	return( m_Y1 - m_Y0 );
}

/*
*/
inline int32 CRect::iWidth() const
{
	return( (int32)Width() );
}

/*
*/
inline int32 CRect::iHeight() const
{
	return( (int32)Height() );
}

/*
*/
inline bool CRect::IsNormalized() const
{
	return( (Width() >= 0.0f) && (Height() >= 0.0f) );
}

/*
*/
inline bool CRect::Surrounds( const CRect &_r ) const
{
	return( (m_X0 < _r.m_X0) && (m_Y0 < _r.m_Y0) && (m_X1 > _r.m_X1) && (m_Y1 > _r.m_Y1) );
}

/*
*/
inline bool	CRect::Inside( const CVector2 &_p ) const
{
	if( (_p.m_X < m_X0) || (_p.m_X > m_X1) ) return( false );
	if( (_p.m_Y < m_Y0) || (_p.m_Y > m_Y1) ) return( false );

	return( true );
}

/*
*/
inline fp4 CRect::Aspect( void ) const
{
	return( Height() / Width() );
}

/*
*/
inline fp4 CRect::Area( void ) const
{
	return( Width() * Height() );
}

/*
*/
inline uint32 CRect::iArea( void ) const
{
	return( iWidth() * iHeight() );
}

};

};

#endif
