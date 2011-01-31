/*
	3x3 matrix class.
	NOTE. Will be typedefed and included from Matrix.h.
*/
#ifndef _MATRIX3X3_X86_H
#define _MATRIX3X3_X86_H

#include    <string.h>
#include	"Vector2_X86.h"
#include	"Vector3_X86.h"
#include	"MatrixDefs.h"

namespace	Base
{

namespace	Math
{

//	Identity
static const fp4	g_matrix3x3_x86_ident[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, };

/*
	CMatrix3x3_x86.

*/
class CMatrix3x3_x86
{
	public:
			CMatrix3x3_x86();
			CMatrix3x3_x86( const CVector3_x86 &_v0, const CVector3_x86 &_v1, const CVector3_x86 &_v2 );
			CMatrix3x3_x86( const CMatrix3x3_x86 &_mx );
			CMatrix3x3_x86( fp4 _m11, fp4 _m12, fp4 _m13, fp4 _m21, fp4 _m22, fp4 _m23, fp4 _m31, fp4 _m32, fp4 _m33 );

			//
			void	Aim( const CVector3_x86 &_from, const CVector3_x86 &_to, const CVector3_x86 &_up );
			void	AimRestricted( const CVector3_x86 &_from, const CVector3_x86 &_to, const CVector3_x86 &_up );

			//
			void	Set( fp4 _m11, fp4 _m12, fp4 _m13, fp4 _m21, fp4 _m22, fp4 _m23, fp4 _m31, fp4 _m32, fp4 _m33 );
			void	Set( const CVector3_x86 &_v0, const CVector3_x86 &_v1, const CVector3_x86 &_v2 );
			void	Set( const CMatrix3x3_x86 &_m1 );

			//
			void	Identity();
			void	Transpose();
			bool	Orthonorm( const fp4 _limit );

			//
			void	Scale( const CVector3_x86 &_s );

			//	World.
			void	Rotate_X( const fp4 _a );
			void	Rotate_Y( const fp4 _a );
			void	Rotate_Z( const fp4 _a );

			//	Local.
			void	Rotate_LX( const fp4 _a );
			void	Rotate_LY( const fp4 _a );
			void	Rotate_LZ( const fp4 _a );

			//
			void	Rotate( const CVector3_x86 &_vec, const fp4 _a );

			//
			CVector3_x86	GetX( void ) const;
			CVector3_x86	GetY( void ) const;
			CVector3_x86	GetZ( void ) const;

			//
			void operator *= ( const CMatrix3x3_x86 &_m1 );

			//
			void	Transform( const CVector3_x86 &_src, CVector3_x86 &_dst ) const;
			void	Translate( const CVector2_x86 &_t );

			fp4	m_Mat[3][3];
};

/*
*/
static inline CMatrix3x3_x86 operator * (const CMatrix3x3_x86 &_m0, const CMatrix3x3_x86 &_m1 )
{
	CMatrix3x3_x86 m2(	_m0.m_Mat[0][0]*_m1.m_Mat[0][0] + _m0.m_Mat[0][1]*_m1.m_Mat[1][0] + _m0.m_Mat[0][2]*_m1.m_Mat[2][0],
						_m0.m_Mat[0][0]*_m1.m_Mat[0][1] + _m0.m_Mat[0][1]*_m1.m_Mat[1][1] + _m0.m_Mat[0][2]*_m1.m_Mat[2][1],
						_m0.m_Mat[0][0]*_m1.m_Mat[0][2] + _m0.m_Mat[0][1]*_m1.m_Mat[1][2] + _m0.m_Mat[0][2]*_m1.m_Mat[2][2],

						_m0.m_Mat[1][0]*_m1.m_Mat[0][0] + _m0.m_Mat[1][1]*_m1.m_Mat[1][0] + _m0.m_Mat[1][2]*_m1.m_Mat[2][0],
						_m0.m_Mat[1][0]*_m1.m_Mat[0][1] + _m0.m_Mat[1][1]*_m1.m_Mat[1][1] + _m0.m_Mat[1][2]*_m1.m_Mat[2][1],
						_m0.m_Mat[1][0]*_m1.m_Mat[0][2] + _m0.m_Mat[1][1]*_m1.m_Mat[1][2] + _m0.m_Mat[1][2]*_m1.m_Mat[2][2],

						_m0.m_Mat[2][0]*_m1.m_Mat[0][0] + _m0.m_Mat[2][1]*_m1.m_Mat[1][0] + _m0.m_Mat[2][2]*_m1.m_Mat[2][0],
						_m0.m_Mat[2][0]*_m1.m_Mat[0][1] + _m0.m_Mat[2][1]*_m1.m_Mat[1][1] + _m0.m_Mat[2][2]*_m1.m_Mat[2][1],
						_m0.m_Mat[2][0]*_m1.m_Mat[0][2] + _m0.m_Mat[2][1]*_m1.m_Mat[1][2] + _m0.m_Mat[2][2]*_m1.m_Mat[2][2] );
	return( m2 );
}

/*
*/
static inline CVector3_x86 operator * ( const CMatrix3x3_x86 &_m, const CVector3_x86 &_v )
{
	return( CVector3_x86(	_m.M11*_v.m_X + _m.M21*_v.m_Y + _m.M31*_v.m_Z,
							_m.M12*_v.m_X + _m.M22*_v.m_Y + _m.M32*_v.m_Z,
							_m.M13*_v.m_X + _m.M23*_v.m_Y + _m.M33*_v.m_Z ) );
};

/*
*/
inline CMatrix3x3_x86::CMatrix3x3_x86()
{
	memcpy( &(m_Mat[0][0]), g_matrix3x3_x86_ident, sizeof( g_matrix3x3_x86_ident ) );
}

/*
*/
inline CMatrix3x3_x86::CMatrix3x3_x86( const CVector3_x86 &_v0, const CVector3_x86 &_v1, const CVector3_x86 &_v2 )
{
	M11 = _v0.m_X; M12 = _v0.m_Y; M13 = _v0.m_Z;
	M21 = _v1.m_X; M22 = _v1.m_Y; M23 = _v1.m_Z;
	M31 = _v2.m_X; M32 = _v2.m_Y; M33 = _v2.m_Z;
}

/*
*/
inline CMatrix3x3_x86::CMatrix3x3_x86( const CMatrix3x3_x86 &_m1 )
{
	memcpy( m_Mat, &(_m1.m_Mat[0][0]), 9*sizeof(fp4) );
}

/*
*/
inline CMatrix3x3_x86::CMatrix3x3_x86( fp4 _m11, fp4 _m12, fp4 _m13, fp4 _m21, fp4 _m22, fp4 _m23, fp4 _m31, fp4 _m32, fp4 _m33 )
{
	M11 = _m11; M12 = _m12; M13 = _m13;
	M21 = _m21; M22 = _m22; M23 = _m23;
	M31 = _m31; M32 = _m32; M33 = _m33;
}

/*
*/
inline void	CMatrix3x3_x86::Aim( const CVector3_x86 &_from, const CVector3_x86 &_to, const CVector3_x86 &_up )
{
	CVector3_x86 z( _from - _to );
	z.NormalizeFast();

	CVector3_x86	x( _up * z );
	x.NormalizeFast();

	CVector3_x86	y = z * x;

	M11 = x.m_X;  M12 = x.m_Y;  M13 = x.m_Z;
	M21 = y.m_X;  M22 = y.m_Y;  M23 = y.m_Z;
	M31 = z.m_X;  M32 = z.m_Y;  M33 = z.m_Z;
}

/*
*/
inline void  CMatrix3x3_x86::AimRestricted( const CVector3_x86 &_from, const CVector3_x86 &_to, const CVector3_x86 &_up )
{
	CVector3_x86 z( _from - _to );
	z.NormalizeFast();

	CVector3_x86 y( _up );
	y.NormalizeFast();

	CVector3_x86 x(y * z);
	z = x * y;

	M11 = x.m_X;  M12 = x.m_Y;  M13 = x.m_Z;
	M21 = y.m_X;  M22 = y.m_Y;  M23 = y.m_Z;
	M31 = z.m_X;  M32 = z.m_Y;  M33 = z.m_Z;
}

/*
*/
inline void  CMatrix3x3_x86::Set( fp4 _m11, fp4 _m12, fp4 _m13, fp4 _m21, fp4 _m22, fp4 _m23, fp4 _m31, fp4 _m32, fp4 _m33 )
{
	M11 = _m11; M12 = _m12; M13 = _m13;
	M21 = _m21; M22 = _m22; M23 = _m23;
	M31 = _m31; M32 = _m32; M33 = _m33;
}

/*
*/
inline void  CMatrix3x3_x86::Set( const CVector3_x86 &_v0, const CVector3_x86 &_v1, const CVector3_x86 &_v2 )
{
	M11 = _v0.m_X; M12 = _v0.m_Y; M13 = _v0.m_Z;
	M21 = _v1.m_X; M22 = _v1.m_Y; M23 = _v1.m_Z;
	M31 = _v2.m_X; M32 = _v2.m_Y; M33 = _v2.m_Z;
}

/*
*/
inline void	CMatrix3x3_x86::Set( const CMatrix3x3_x86 &_m1 )
{
	memcpy( m_Mat, &(_m1.m_Mat), 9*sizeof(fp4) );
}

/*
*/
inline void	CMatrix3x3_x86::Identity( void )
{
	memcpy( &(m_Mat[0][0]), g_matrix3x3_x86_ident, sizeof(g_matrix3x3_x86_ident) );
}

/*
*/
inline void	CMatrix3x3_x86::Transpose( void )
{
#define da_swap(x,y) { fp4 t=x; x=y; y=t; }
	da_swap( m_Mat[0][1], m_Mat[1][0] );
	da_swap( m_Mat[0][2], m_Mat[2][0] );
	da_swap( m_Mat[1][2], m_Mat[2][1] );
#undef	da_swap
}

/*
*/
inline bool	CMatrix3x3_x86::Orthonorm( const fp4 _limit )
{
	if( ((M11*M21+M12*M22+M13*M23) < _limit ) &&
		((M11*M31+M12*M32+M13*M33) < _limit ) &&
		((M31*M21+M32*M22+M33*M23) < _limit ) &&
		((M11*M11+M12*M12+M13*M13) > (1.0f-_limit) ) &&
		((M11*M11+M12*M12+M13*M13) < (1.0f+_limit) ) &&
		((M21*M21+M22*M22+M23*M23) > (1.0f-_limit) ) &&
		((M21*M21+M22*M22+M23*M23) < (1.0f+_limit) ) &&
		((M31*M31+M32*M32+M33*M33) > (1.0f-_limit) ) &&
		((M31*M31+M32*M32+M33*M33) < (1.0f+_limit) ) )
		return( true );
	else
		return( false );
}

/*
*/
inline void	CMatrix3x3_x86::Scale( const CVector3_x86 &_s )
{
	for( uint32 i=0; i<3; i++ )
	{
		m_Mat[i][0] *= _s.m_X;
		m_Mat[i][1] *= _s.m_Y;
		m_Mat[i][2] *= _s.m_Z;
	}
}

/*
*/
inline void	CMatrix3x3_x86::Rotate_X( const fp4 _a )
{
	fp4 s, c;
	SinCos( _a, &s, &c );

	for( uint32 i=0; i<3; i++ )
	{
		fp4 mi1 = m_Mat[i][1];
		fp4 mi2 = m_Mat[i][2];
		m_Mat[i][1] = mi1*c + mi2*-s;
		m_Mat[i][2] = mi1*s + mi2*c;
	}
}

/*
*/
inline void	CMatrix3x3_x86::Rotate_Y( const fp4 _a )
{
	fp4 s, c;
	SinCos( _a, &s, &c );

	for( uint32 i=0; i<3; i++ )
	{
		fp4 mi0 = m_Mat[i][0];
		fp4 mi2 = m_Mat[i][2];
		m_Mat[i][0] = mi0*c + mi2*s;
		m_Mat[i][2] = mi0*-s + mi2*c;
	}
}

/*
*/
inline void	CMatrix3x3_x86::Rotate_Z( const fp4 _a )
{
	fp4 s, c;
	SinCos( _a, &s, &c );

	for( uint32 i=0; i<3; i++ )
	{
		fp4 mi0 = m_Mat[i][0];
		fp4 mi1 = m_Mat[i][1];
		m_Mat[i][0] = mi0*c + mi1*-s;
		m_Mat[i][1] = mi0*s + mi1*c;
	}
}

/*
*/
inline void		CMatrix3x3_x86::Rotate_LX( const fp4 _a )
{
	fp4	sa, ca;
	SinCos( _a, &sa, &ca );

	CMatrix3x3_x86 rotM;

	rotM.M22 = ca; rotM.M23 = -sa;
	rotM.M32 = sa; rotM.M33 =  ca;

	(*this) = rotM * (*this);
}

/*
*/
inline void	CMatrix3x3_x86::Rotate_LY( const fp4 _a )
{
	fp4	sa, ca;
	SinCos( _a, &sa, &ca );

	CMatrix3x3_x86 rotM;
	rotM.M11 = ca;  rotM.M13 = sa;
	rotM.M31 = -sa; rotM.M33 = ca;

	(*this) = rotM * (*this);
}

/*
*/
inline void	CMatrix3x3_x86::Rotate_LZ( const fp4 _a )
{
	fp4	sa, ca;
	SinCos( _a, &sa, &ca );

	CMatrix3x3_x86 rotM;
	rotM.M11 = ca; rotM.M12 = -sa;
	rotM.M21 = sa; rotM.M22 =  ca;

	(*this) = rotM * (*this);
}

/*
	Rotate().

*/
inline void  CMatrix3x3_x86::Rotate( const CVector3_x86 &_vec, const fp4 _a )
{
	CVector3_x86	v( _vec );
	v.NormalizeFast();

	fp4	sa, ca;
	SinCos( _a, &sa, &ca );

	CMatrix3x3_x86 rotM;
	rotM.M11 = ca + (1.0f - ca) * v.m_X * v.m_X;
	rotM.M12 = (1.0f - ca) * v.m_X * v.m_Y - sa * v.m_Z;
	rotM.M13 = (1.0f - ca) * v.m_Z * v.m_X + sa * v.m_Y;
	rotM.M21 = (1.0f - ca) * v.m_X * v.m_Y + sa * v.m_Z;
	rotM.M22 = ca + (1.0f - ca) * v.m_Y * v.m_Y;
	rotM.M23 = (1.0f - ca) * v.m_Y * v.m_Z - sa * v.m_X;
	rotM.M31 = (1.0f - ca) * v.m_Z * v.m_X - sa * v.m_Y;
	rotM.M32 = (1.0f - ca) * v.m_Y * v.m_Z + sa * v.m_X;
	rotM.M33 = ca + (1.0f - ca) * v.m_Z * v.m_Z;

	(*this) = (*this) * rotM;
}

/*
*/
inline CVector3_x86	CMatrix3x3_x86::GetX( void ) const
{
	CVector3_x86 v( M11, M12, M13 );
	return( v );
}

/*
*/
inline CVector3_x86	CMatrix3x3_x86::GetY( void ) const
{
	CVector3_x86 v( M21, M22, M23 );
	return( v );
}

/*
*/
inline CVector3_x86	CMatrix3x3_x86::GetZ( void ) const
{
	CVector3_x86 v( M31, M32, M33 );
	return( v );
};

/*
*/
inline void	CMatrix3x3_x86::operator *= (const CMatrix3x3_x86 &_m1 )
{
	for( uint32 i=0; i<3; i++ )
	{
		fp4 mi0 = m_Mat[i][0];
		fp4 mi1 = m_Mat[i][1];
		fp4 mi2 = m_Mat[i][2];
		m_Mat[i][0] = mi0*_m1.m_Mat[0][0] + mi1*_m1.m_Mat[1][0] + mi2*_m1.m_Mat[2][0];
		m_Mat[i][1] = mi0*_m1.m_Mat[0][1] + mi1*_m1.m_Mat[1][1] + mi2*_m1.m_Mat[2][1];
		m_Mat[i][2] = mi0*_m1.m_Mat[0][2] + mi1*_m1.m_Mat[1][2] + mi2*_m1.m_Mat[2][2];
	}
}

/*
	Multiply source vector with matrix and store in destination vector.
	This eliminates the construction of a temp CVector3_x86 object.
*/
inline void  CMatrix3x3_x86::Transform( const CVector3_x86 &_src, CVector3_x86 &_dst ) const
{
	_dst.m_X = M11*_src.m_X + M21*_src.m_Y + M31*_src.m_Z;
	_dst.m_Y = M12*_src.m_X + M22*_src.m_Y + M32*_src.m_Z;
	_dst.m_Z = M13*_src.m_X + M23*_src.m_Y + M33*_src.m_Z;
}

/*
	Matrix is treated like a 2x2 rotational matrix and 2d-translation.
*/
inline void	CMatrix3x3_x86::Translate( const CVector2_x86 &_t )
{
	M31 += _t.m_X;
	M32 += _t.m_Y;
}

};

};

#endif
