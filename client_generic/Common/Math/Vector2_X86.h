/*
	2 component vector class.
	NOTE. Will be typedefed and included from vector2.h.
*/
#ifndef _VECTOR2_X86_H
#define _VECTOR2_X86_H

namespace	Base
{

namespace	Math
{

/*
	CVector2_x86.

*/
class	CVector2_x86
{
	public:
			CVector2_x86();
			CVector2_x86( const float _x, const float _y );
			CVector2_x86( const CVector2_x86 &_vec);
			CVector2_x86( const fp4 *_p );

			//
			void	Set( const fp4 _x, const fp4 _y );
			void	Set( const CVector2_x86 &_vec );
			void	Set( const fp4 *_p );

			//
			fp4		Len() const;
			fp4		RLen() const;
			fp4		RLenFast() const;
			fp4		LenSqr() const;

			//
			void	Normalize();
			void	NormalizeFast();

			//
			void	operator += ( const CVector2_x86 &_v0 );
			void	operator -= ( const CVector2_x86 &_v0 );
			void	operator *= ( const fp4 _s );
			void	operator /= ( const fp4 _s );

			//
			bool	IsEqual( const CVector2_x86 &_v, const fp4 _tol ) const;
			int32	Compare( const CVector2_x86 &_v, const fp4 _tol ) const;	//	-1, 0, +1

			//
			fp4		m_X, m_Y;
};

/*
*/
inline CVector2_x86::CVector2_x86() : m_X( 0.0f ), m_Y( 0.0f )
{
}

/*
*/
inline CVector2_x86::CVector2_x86( const fp4 _x, const fp4 _y ) : m_X(_x), m_Y(_y)
{
}

/*
*/
inline CVector2_x86::CVector2_x86( const CVector2_x86 &_vec) : m_X( _vec.m_X ), m_Y( _vec.m_Y )
{
}

/*
*/
inline CVector2_x86::CVector2_x86( const fp4 *_p ) : m_X( _p[0] ), m_Y( _p[1] )
{
}

/*
*/
inline void CVector2_x86::Set( const fp4 _x, const fp4 _y )
{
	m_X = _x;
	m_Y = _y;
}

/*
*/
inline void CVector2_x86::Set( const CVector2_x86 &_v )
{
	m_X = _v.m_X;
	m_Y = _v.m_Y;
}

/*
*/
inline void CVector2_x86::Set( const fp4 *_p )
{
	m_X = _p[0];
	m_Y = _p[1];
}

/*
*/
inline fp4 CVector2_x86::Len( void ) const
{
	return( (fp4)Sqrt( m_X * m_X + m_Y * m_Y ) );
}

/*
*/
inline fp4 CVector2_x86::LenSqr( void ) const
{
	return( m_X * m_X + m_Y * m_Y );
}

/*
*/
inline fp4 CVector2_x86::RLen( void ) const
{
	return( (fp4)RSqrt( m_X * m_X + m_Y * m_Y ) );
}

/*
*/
inline fp4 CVector2_x86::RLenFast( void ) const
{
	return( (fp4)RSqrtFast( m_X * m_X + m_Y * m_Y ) );
}

/*
	Normalize().
	POTENTIAL BUG.
*/
inline void CVector2_x86::Normalize( void )
{
	fp4 l = RLen();
	m_X *= l;
	m_Y *= l;
}

/*
	NormalizeFast().
	POTENTIAL BUG.
*/
inline void CVector2_x86::NormalizeFast( void )
{
	fp4 l = RLenFast();
	m_X *= l;
	m_Y *= l;
}

/*
*/
inline void CVector2_x86::operator += ( const CVector2_x86 &_v0 )
{
	m_X += _v0.m_X;
	m_Y += _v0.m_Y;
}

/*
*/
inline void CVector2_x86::operator -= ( const CVector2_x86 &_v0 )
{
	m_X -= _v0.m_X;
	m_Y -= _v0.m_Y;
}

/*
*/
inline void CVector2_x86::operator *= ( const fp4 _s )
{
	m_X *= _s;
	m_Y *= _s;
}

/*
*/
inline void CVector2_x86::operator /= ( const fp4 _s )
{
	fp4	oS = 1.0f / _s;
	m_X *= oS;
	m_Y *= oS;
}

/*
*/
inline bool  CVector2_x86::IsEqual( const CVector2_x86 &_v, const fp4 _tol ) const
{
	if( fabsf( _v.m_X - m_X ) > _tol )		return( false );
	else if( fabsf( _v.m_Y - m_Y ) > _tol )	return( false );
	return( true );
}

/*
*/
inline int32 CVector2_x86::Compare(const CVector2_x86 &_v, const fp4 _tol ) const
{
	if( fabsf( _v.m_X - m_X ) > _tol )		return( (_v.m_X > m_X ) ? +1 : -1 );
	else if( fabsf( _v.m_Y - m_X ) > _tol )	return( (_v.m_Y > m_Y ) ? +1 : -1 );
	else	return( 0 );
}

/*
*/
static inline	CVector2_x86 operator + ( const CVector2_x86 &_v0, const CVector2_x86 &_v1 )
{
	return( CVector2_x86( _v0.m_X + _v1.m_X, _v0.m_Y + _v1.m_Y ) );
}

/*
*/
static inline	CVector2_x86 operator - ( const CVector2_x86 &_v0, const CVector2_x86 &_v1 )
{
	return( CVector2_x86( _v0.m_X - _v1.m_X, _v0.m_Y - _v1.m_Y ) );
}

/*
*/
static inline	CVector2_x86 operator * ( const CVector2_x86 &_v0, const fp4 _s )
{
	return( CVector2_x86( _v0.m_X * _s, _v0.m_Y * _s ) );
}

/*
*/
static inline	CVector2_x86 operator - ( const CVector2_x86 &_v )
{
	return( CVector2_x86( -_v.m_X, -_v.m_Y ) );
}

};

};

#endif
