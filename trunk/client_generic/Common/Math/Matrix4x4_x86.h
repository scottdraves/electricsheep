/*
	4x4 matrix class.
	NOTE. Will be typedefed and included from Matrix.h.
*/
#ifndef _MATRIX4X4_X86_H
#define _MATRIX4X4_X86_H

#include	"Vector3_X86.h"
#include	"Vector4_X86.h"
#include	"MatrixDefs.h"

namespace	Base
{

namespace	Math
{

//	Identity
static const fp4	g_matrix4x4_x86_ident[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, };

/*
	CMatrix4x4_x86().

*/
class	CMatrix4x4_x86
{
	public:
			CMatrix4x4_x86();
			CMatrix4x4_x86( const CVector4_x86 &_v0, const CVector4_x86 &_v1, const CVector4_x86 &_v2, const CVector4_x86 &_v3 );
			CMatrix4x4_x86( const CMatrix4x4_x86 &_m1 );
			CMatrix4x4_x86(	fp4 _m11, fp4 _m12, fp4 _m13, fp4 _m14,
							fp4 _m21, fp4 _m22, fp4 _m23, fp4 _m24,
							fp4 _m31, fp4 _m32, fp4 _m33, fp4 _m34,
							fp4 _m41, fp4 _m42, fp4 _m43, fp4 _m44);

			//
			void	Set( const CVector4_x86 &_v0, const CVector4_x86 &_v1, const CVector4_x86 &_v2, const CVector4_x86 &_v3 );
			void	Set( const CMatrix4x4_x86 &_m1 );
			void	Set(	fp4 _m11, fp4 _m12, fp4 _m13, fp4 _m14,
							fp4 _m21, fp4 _m22, fp4 _m23, fp4 _m24,
							fp4 _m31, fp4 _m32, fp4 _m33, fp4 _m34,
							fp4 _m41, fp4 _m42, fp4 _m43, fp4 _m44 );

			//
			void	Identity();
			void	Transpose();
			fp4		Determinant();
			void	Invert();
			void	InvertFast();										//	Only when 3x3rot+trans.

			//
			CVector4_x86	&GetX() const;
			CVector4_x86	&GetY() const;
			CVector4_x86	&GetZ() const;
			CVector4_x86	&GetW() const;

			//	World.
			void	Rotate_X( const fp4 _a );
			void	Rotate_Y( const fp4 _a );
			void	Rotate_Z( const fp4 _a );
			void	Rotate( const CVector3_x86 &_vec, const fp4 _a );

			//
			void	Translate( const CVector4_x86 &_t );
			void	SetTranslation( const CVector4_x86 &_t );
			void	Scale( const CVector3_x86 &_s );

			//	Lefthanded.
			void	AimLH( const CVector3_x86 &_to, const CVector3_x86 &_up );
			void	PerspectiveLH( const fp4 _fovY, const fp4 _aspect, const fp4 _zn, const fp4 _zf );
			void	PerspectiveOffCenterLH( const fp4 _minX, const fp4 _maxX, const fp4 _minY, const fp4 _maxY, const fp4 _zn, const fp4 _zf );
			void	OrthographicLH( const fp4 _w, const fp4 _h, const fp4 _zn, const fp4 _zf );

			//	Righthanded.
			void	AimRH( const CVector3_x86 &_to, const CVector3_x86 &_up );
			void	PerspectiveRH( const fp4 _fovY, const fp4 _aspect, const fp4 _zn, const fp4 _zf );
			void	PerspectiveOffCenterRH( const fp4 _minX, const fp4 _maxX, const fp4 _minY, const fp4 _maxY, const fp4 _zn, const fp4 _zf );
			void	OrthographicRH( const fp4 _w, const fp4 _h, const fp4 _zn, const fp4 _zf );

			//
			void	AimRestricted( const CVector3_x86 &_to, const CVector3_x86 &_up );

			//
			void	operator *= ( const CMatrix4x4_x86 &_m1 );
			void	MultiplyFast( const CMatrix4x4_x86 &_m1 );			//	Only when 14=24=34=0 & 44=1

			//
			CVector3_x86	Transform( const CVector3_x86 &_v) const;	//	Projecting back into w=1

			//
			void	Multiply( const CVector4_x86 &_src, CVector4_x86 &_dst ) const;
			void	Multiply( const CVector3_x86 &_src, CVector3_x86 &_dst ) const;

			//
			fp4	m_Mat[4][4];
};

/*
*/
inline CMatrix4x4_x86::CMatrix4x4_x86()
{
	memcpy( &(m_Mat[0][0]), g_matrix3x3_x86_ident, sizeof(g_matrix3x3_x86_ident) );
}

/*
*/
inline CMatrix4x4_x86::CMatrix4x4_x86( const CVector4_x86 &_v0, const CVector4_x86 &_v1, const CVector4_x86 &_v2, const CVector4_x86 &_v3 )
{
	M11 = _v0.m_X; M12 = _v0.m_Y; M13 = _v0.m_Z; M14 = _v0.m_W;
	M21 = _v1.m_X; M22 = _v1.m_Y; M23 = _v1.m_Z; M24 = _v1.m_W;
	M31 = _v2.m_X; M32 = _v2.m_Y; M33 = _v2.m_Z; M34 = _v2.m_W;
	M41 = _v3.m_X; M42 = _v3.m_Y; M43 = _v3.m_Z; M44 = _v3.m_W;
}

/*
*/
inline CMatrix4x4_x86::CMatrix4x4_x86( const CMatrix4x4_x86 &_m1 )
{
	memcpy( m_Mat, &(_m1.m_Mat[0][0]), 16 * sizeof(fp4) );
}

/*
*/
inline CMatrix4x4_x86::CMatrix4x4_x86(	fp4 _m11, fp4 _m12, fp4 _m13, fp4 _m14,
										fp4 _m21, fp4 _m22, fp4 _m23, fp4 _m24,
										fp4 _m31, fp4 _m32, fp4 _m33, fp4 _m34,
										fp4 _m41, fp4 _m42, fp4 _m43, fp4 _m44)
{
	M11 = _m11; M12 = _m12; M13 = _m13; M14 = _m14;
	M21 = _m21; M22 = _m22; M23 = _m23; M24 = _m24;
	M31 = _m31; M32 = _m32; M33 = _m33; M34 = _m34;
	M41 = _m41; M42 = _m42; M43 = _m43; M44 = _m44;
}


/*
*/
inline void  CMatrix4x4_x86::Set( const CVector4_x86 &_v0, const CVector4_x86 &_v1, const CVector4_x86 &_v2, const CVector4_x86 &_v3 )
{
	M11 = _v0.m_X; M12 = _v0.m_Y; M13 = _v0.m_Z; M14 = _v0.m_W;
	M21 = _v1.m_X; M22 = _v1.m_Y; M23 = _v1.m_Z; M24 = _v1.m_W;
	M31 = _v2.m_X; M32 = _v2.m_Y; M33 = _v2.m_Z; M34 = _v2.m_W;
	M41 = _v3.m_X; M42 = _v3.m_Y; M43 = _v3.m_Z; M44 = _v3.m_W;
}

/*
*/
inline void CMatrix4x4_x86::Set( const CMatrix4x4_x86 &_m1 )
{
	memcpy( m_Mat, &(_m1.m_Mat[0][0]), 16*sizeof(fp4) );
}

/*
*/
inline void CMatrix4x4_x86::Set(	fp4 _m11, fp4 _m12, fp4 _m13, fp4 _m14,
									fp4 _m21, fp4 _m22, fp4 _m23, fp4 _m24,
									fp4 _m31, fp4 _m32, fp4 _m33, fp4 _m34,
									fp4 _m41, fp4 _m42, fp4 _m43, fp4 _m44)
{
	M11=_m11; M12=_m12; M13=_m13; M14=_m14;
	M21=_m21; M22=_m22; M23=_m23; M24=_m24;
	M31=_m31; M32=_m32; M33=_m33; M34=_m34;
	M41=_m41; M42=_m42; M43=_m43; M44=_m44;
}

/*
*/
inline void CMatrix4x4_x86::Identity( void )
{
	memcpy( &(m_Mat[0][0]), g_matrix4x4_x86_ident, sizeof(g_matrix4x4_x86_ident) );
}

/*
*/
inline void  CMatrix4x4_x86::Transpose( void )
{
#define da_swap(x,y) { fp4 t=x; x=y; y=t; }
	da_swap( M12, M21 );
	da_swap( M13, M31 );
	da_swap( M14, M41 );
	da_swap( M23, M32 );
	da_swap( M24, M42 );
	da_swap( M34, M43 );
#undef da_swap
}

/*
*/
inline fp4	CMatrix4x4_x86::Determinant( void )
{
	return(	 (M11 * M22 - M12 * M21) * (M33 * M44 - M34 * M43)
			-(M11 * M23 - M13 * M21) * (M32 * M44 - M34 * M42)
			+(M11 * M24 - M14 * M21) * (M32 * M43 - M33 * M42)
			+(M12 * M23 - M13 * M22) * (M31 * M44 - M34 * M41)
			-(M12 * M24 - M14 * M22) * (M31 * M43 - M33 * M41)
			+(M13 * M24 - M14 * M23) * (M31 * M42 - M32 * M41) );
}

/*
*/
inline void CMatrix4x4_x86::Invert( void )
{
	fp4 s = Determinant();
	if( s == 0.0f )
		return;

	s = 1.0f / s;
	Set(	s*(M22*(M33*M44 - M34*M43) + M23*(M34*M42 - M32*M44) + M24*(M32*M43 - M33*M42)),
			s*(M32*(M13*M44 - M14*M43) + M33*(M14*M42 - M12*M44) + M34*(M12*M43 - M13*M42)),
			s*(M42*(M13*M24 - M14*M23) + M43*(M14*M22 - M12*M24) + M44*(M12*M23 - M13*M22)),
			s*(M12*(M24*M33 - M23*M34) + M13*(M22*M34 - M24*M32) + M14*(M23*M32 - M22*M33)),
			s*(M23*(M31*M44 - M34*M41) + M24*(M33*M41 - M31*M43) + M21*(M34*M43 - M33*M44)),
			s*(M33*(M11*M44 - M14*M41) + M34*(M13*M41 - M11*M43) + M31*(M14*M43 - M13*M44)),
			s*(M43*(M11*M24 - M14*M21) + M44*(M13*M21 - M11*M23) + M41*(M14*M23 - M13*M24)),
			s*(M13*(M24*M31 - M21*M34) + M14*(M21*M33 - M23*M31) + M11*(M23*M34 - M24*M33)),
			s*(M24*(M31*M42 - M32*M41) + M21*(M32*M44 - M34*M42) + M22*(M34*M41 - M31*M44)),
			s*(M34*(M11*M42 - M12*M41) + M31*(M12*M44 - M14*M42) + M32*(M14*M41 - M11*M44)),
			s*(M44*(M11*M22 - M12*M21) + M41*(M12*M24 - M14*M22) + M42*(M14*M21 - M11*M24)),
			s*(M14*(M22*M31 - M21*M32) + M11*(M24*M32 - M22*M34) + M12*(M21*M34 - M24*M31)),
			s*(M21*(M33*M42 - M32*M43) + M22*(M31*M43 - M33*M41) + M23*(M32*M41 - M31*M42)),
			s*(M31*(M13*M42 - M12*M43) + M32*(M11*M43 - M13*M41) + M33*(M12*M41 - M11*M42)),
			s*(M41*(M13*M22 - M12*M23) + M42*(M11*M23 - M13*M21) + M43*(M12*M21 - M11*M22)),
			s*(M11*(M22*M33 - M23*M32) + M12*(M23*M31 - M21*M33) + M13*(M21*M32 - M22*M31)) );
}

/*
	inverts a 4x4 matrix consisting of a 3x3 rotation matrix and a translation (eg. everything that has [0,0,0,1]
	as the rightmost column) MUCH cheaper then a real 4x4 inversion.
*/
inline void  CMatrix4x4_x86::InvertFast( void )
{
	fp4 s = Determinant();

	if( s == 0.0f )
		return;

	s = 1.0f/s;

	Set(	s * ((M22 * M33) - (M23 * M32)),
			s * ((M32 * M13) - (M33 * M12)),
			s * ((M12 * M23) - (M13 * M22)),
			0.0f,
			s * ((M23 * M31) - (M21 * M33)),
			s * ((M33 * M11) - (M31 * M13)),
			s * ((M13 * M21) - (M11 * M23)),
			0.0f,
			s * ((M21 * M32) - (M22 * M31)),
			s * ((M31 * M12) - (M32 * M11)),
			s * ((M11 * M22) - (M12 * M21)),
			0.0f,
			s * (M21*(M33*M42 - M32*M43) + M22*(M31*M43 - M33*M41) + M23*(M32*M41 - M31*M42)),
			s * (M31*(M13*M42 - M12*M43) + M32*(M11*M43 - M13*M41) + M33*(M12*M41 - M11*M42)),
			s * (M41*(M13*M22 - M12*M23) + M42*(M11*M23 - M13*M21) + M43*(M12*M21 - M11*M22)),
			1.0f );
}

/*
	M14==M24==M34==0 & M44==1
*/
inline void CMatrix4x4_x86::MultiplyFast( const CMatrix4x4_x86 &_m1 )
{
	for( uint32 i=0; i<4; i++ )
	{
		fp4 mi0 = m_Mat[i][0];
		fp4 mi1 = m_Mat[i][1];
		fp4 mi2 = m_Mat[i][2];
		m_Mat[i][0] = mi0*_m1.m_Mat[0][0] + mi1*_m1.m_Mat[1][0] + mi2*_m1.m_Mat[2][0];
		m_Mat[i][1] = mi0*_m1.m_Mat[0][1] + mi1*_m1.m_Mat[1][1] + mi2*_m1.m_Mat[2][1];
		m_Mat[i][2] = mi0*_m1.m_Mat[0][2] + mi1*_m1.m_Mat[1][2] + mi2*_m1.m_Mat[2][2];
	}

	m_Mat[3][0] += _m1.m_Mat[3][0];
	m_Mat[3][1] += _m1.m_Mat[3][1];
	m_Mat[3][2] += _m1.m_Mat[3][2];

	m_Mat[0][3] = 0.0f;
	m_Mat[1][3] = 0.0f;
	m_Mat[2][3] = 0.0f;
	m_Mat[3][3] = 1.0f;
}

/*
	Transforms a vector by the matrix, projecting the result back into w=1.
*/
inline CVector3_x86 CMatrix4x4_x86::Transform( const CVector3_x86 &_v ) const
{
	fp4 d = 1.0f / ( M14*_v.m_X + M24*_v.m_Y + M34*_v.m_Z + M44 );
	return( CVector3_x86(	(M11*_v.m_X + M21*_v.m_Y + M31*_v.m_Z + M41) * d,
							(M12*_v.m_X + M22*_v.m_Y + M32*_v.m_Z + M42) * d,
							(M13*_v.m_X + M23*_v.m_Y + M33*_v.m_Z + M43) * d ) );
}

/*
*/
inline CVector4_x86 &CMatrix4x4_x86::GetX( void ) const
{
	return( *(CVector4_x86 *)&M11 );
}

/*
*/
inline CVector4_x86 &CMatrix4x4_x86::GetY( void ) const
{
	return( *(CVector4_x86*)&M21 );
}

/*
*/
inline CVector4_x86 &CMatrix4x4_x86::GetZ( void ) const
{
	return( *(CVector4_x86*)&M31 );
}

/*
*/
inline CVector4_x86 &CMatrix4x4_x86::GetW( void ) const
{
	return( *(CVector4_x86*)&M41 );
}

/*
*/
inline void CMatrix4x4_x86::Rotate_X( const fp4 _a )
{
	fp4 s, c;
	SinCos( _a, &s, &c );

	for( uint32 i=0; i<4; i++ )
	{
		fp4 mi1 = m_Mat[i][1];
		fp4 mi2 = m_Mat[i][2];
		m_Mat[i][1] = mi1*c + mi2*-s;
		m_Mat[i][2] = mi1*s + mi2*c;
	}
}

/*
*/
inline void  CMatrix4x4_x86::Rotate_Y( const fp4 _a )
{
	fp4 s, c;
	SinCos( _a, &s, &c );

	for( uint32 i=0; i<4; i++ )
	{
		fp4 mi0 = m_Mat[i][0];
		fp4 mi2 = m_Mat[i][2];
		m_Mat[i][0] = mi0*c + mi2*s;
		m_Mat[i][2] = mi0*-s + mi2*c;
	}
}

/*
*/
inline void  CMatrix4x4_x86::Rotate_Z( const fp4 _a )
{
	fp4 s, c;
	SinCos( _a, &s, &c );

	for( uint32 i=0; i<4; i++ )
	{
		fp4 mi0 = m_Mat[i][0];
		fp4 mi1 = m_Mat[i][1];
		m_Mat[i][0] = mi0*c + mi1*-s;
		m_Mat[i][1] = mi0*s + mi1*c;
	}
}

/*
*/
inline void	CMatrix4x4_x86::Translate( const CVector4_x86 &_t )
{
	M41 += _t.m_X;
	M42 += _t.m_Y;
	M43 += _t.m_Z;
	M44 += _t.m_W;	//	<- Eh?
}

/*
*/
inline void CMatrix4x4_x86::SetTranslation( const CVector4_x86 &_t )
{
	M41 = _t.m_X;
	M42 = _t.m_Y;
	M43 = _t.m_Z;
	M44 = _t.m_W;	//	<- Eh?
}

/*
*/
inline void CMatrix4x4_x86::Scale( const CVector3_x86 &_s )
{
	for( uint32 i=0; i<4; i++ )
	{
		m_Mat[i][0] *= _s.m_X;
		m_Mat[i][1] *= _s.m_Y;
		m_Mat[i][2] *= _s.m_Z;
	}
}

/*
*/
inline void	CMatrix4x4_x86::AimRH( const CVector3_x86 &_at, const CVector3_x86 &_up )
{
	CVector3_x86 eye( M41, M42, M43 );

	CVector3_x86 zaxis = eye - _at;
	zaxis.NormalizeFast();

	CVector3_x86 xaxis = _up * zaxis;
	xaxis.NormalizeFast();

	CVector3_x86 yaxis = zaxis * xaxis;

	M11 = xaxis.m_X;  M12 = xaxis.m_Y;  M13 = xaxis.m_Z;  M14 = 0.0f;
	M21 = yaxis.m_X;  M22 = yaxis.m_Y;  M23 = yaxis.m_Z;  M24 = 0.0f;
	M31 = zaxis.m_X;  M32 = zaxis.m_Y;  M33 = zaxis.m_Z;  M34 = 0.0f;
}

/*
*/
inline void  CMatrix4x4_x86::AimLH( const CVector3_x86 &_at, const CVector3_x86 &_up )
{
	CVector3_x86 eye( M41, M42, M43 );

	CVector3_x86 zaxis = _at - eye;
	zaxis.NormalizeFast();

	CVector3_x86 xaxis = _up * zaxis;
	xaxis.NormalizeFast();

	CVector3_x86 yaxis = zaxis * xaxis;

	M11 = xaxis.m_X;  M12 = yaxis.m_X;  M13 = zaxis.m_X;  M14 = 0.0f;
	M21 = xaxis.m_Y;  M22 = yaxis.m_Y;  M23 = zaxis.m_Y;  M24 = 0.0f;
	M31 = xaxis.m_Z;  M32 = yaxis.m_Z;  M33 = zaxis.m_Z;  M34 = 0.0f;
}

/*
*/
inline void CMatrix4x4_x86::PerspectiveLH( const fp4 _fovY, const fp4 _aspect, const fp4 _zn, const fp4 _zf )
{
	fp4 h = 1.0f / tanf( _fovY * 0.5f );
	fp4 w = h / _aspect;

	M11 = w;    M12 = 0.0f; M13 = 0.0f;							M14 = 0.0f;
	M21 = 0.0f; M22 = h;    M23 = 0.0f;							M24 = 0.0f;
	M31 = 0.0f; M32 = 0.0f; M33 = _zf / (_zf - _zn);			M34 = 1.0f;
	M41 = 0.0f; M42 = 0.0f; M43 = -_zn * (_zf / (_zf - _zn));	M44 = 0.0f;
}

/*
*/
inline void CMatrix4x4_x86::PerspectiveRH( const fp4 _fovY, const fp4 _aspect, const fp4 _zn, const fp4 _zf )
{
	fp4 h = 1.0f / tanf( _fovY * 0.5f );
	fp4 w = h / _aspect;

	M11 = w;    M12 = 0.0f; M13 = 0.0f;							M14 = 0.0f;
	M21 = 0.0f; M22 = h;    M23 = 0.0f;							M24 = 0.0f;
	M31 = 0.0f; M32 = 0.0f; M33 = _zf / (_zn - _zf);			M34 = -1.0f;
	M41 = 0.0f; M42 = 0.0f; M43 = _zn * (_zf / (_zn - _zf));	M44 = 0.0f;
}

/*
*/
inline void CMatrix4x4_x86::PerspectiveOffCenterLH( const fp4 _minX, const fp4 _maxX, const fp4 _minY, const fp4 _maxY, const fp4 _zn, const fp4 _zf )
{
	M11 = 2.0f * _zn / (_maxX - _minX); M12 = 0.0f, M13 = 0.0f; M14 = 0.0f;
	M21 = 0.0f; M22 = 2.0f * _zn / (_maxY - _minY); M23 = 0.0f; M24 = 0.0f;
	M31 = (_minX + _maxX) / (_minX - _maxX); M32 = (_maxY + _minY) / (_minY - _maxY); M33 = _zf / (_zf - _zn); M34 = 1.0f;
	M41 = 0.0f; M42 = 0.0f; M43 = _zn * _zf / (_zn - _zf); M44 = 0.0f;
}

/*
*/
inline void CMatrix4x4_x86::PerspectiveOffCenterRH( const fp4 _minX, const fp4 _maxX, const fp4 _minY, const fp4 _maxY, const fp4 _zn, const fp4 _zf )
{
	M11 = 2.0f * _zn / (_maxX - _minX); M12 = 0.0f, M13 = 0.0f; M14 = 0.0f;
	M21 = 0.0f; M22 = 2.0f * _zn / (_maxY - _minY); M23 = 0.0f; M24 = 0.0f;
	M31 = (_minX + _maxX) / (_maxX - _minX); M32 = (_maxY + _minY) / (_maxY - _minY); M33 = _zf / (_zn - _zf); M34 = -1.0f;
	M41 = 0.0f; M42 = 0.0f; M43 = _zn * _zf / (_zn - _zf); M44 = 0.0f;
}

/*
*/
inline void CMatrix4x4_x86::OrthographicLH( const fp4 _w, const fp4 _h, const fp4 _zn, const fp4 _zf )
{
	M11 = 2.0f / _w; M12 = 0.0f;      M13 = 0.0f;				M14 = 0.0f;
	M21 = 0.0f;      M22 = -2.0f / _h; M23 = 0.0f;				M24 = 0.0f;
	M31 = 0.0f;      M32 = 0.0f;      M33 = 1.0f / (_zf - _zn); M34 = 0.0f;
	M41 = 0.0f;      M42 = 0.0f;      M43 = _zn / (_zn - _zf);  M44 = 1.0f;
}

/*
*/
inline void CMatrix4x4_x86::OrthographicRH( const fp4 _w, const fp4 _h, const fp4 _zn, const fp4 _zf )
{
	M11 = 2.0f / _w; M12 = 0.0f;      	M13 = 0.0f;					M14 = 0.0f;
	M21 = 0.0f;      M22 = -2.0f / _h; 	M23 = 0.0f;					M24 = 0.0f;
	M31 = 0.0f;      M32 = 0.0f;      	M33 = 1.0f / (_zn - _zf);	M34 = 0.0f;
	M41 = 0.0f;		 M42 = 0.0f;		M43 = _zn / (_zn - _zf);	M44 = 1.0f;
}

/*
*/
inline void  CMatrix4x4_x86::AimRestricted( const CVector3_x86 &_to, const CVector3_x86 &_up )
{
	CVector3_x86 from( M41, M42, M43 );
	CVector3_x86 z( from - _to );
	z.NormalizeFast();

	CVector3_x86 y( _up );
	y.NormalizeFast();

	CVector3_x86 x(y * z);
	z = x * y;

	M11=x.m_X;  M12=x.m_Y;  M13=x.m_Z;  M14=0.0f;
	M21=y.m_X;  M22=y.m_Y;  M23=y.m_Z;  M24=0.0f;
	M31=z.m_X;  M32=z.m_Y;  M33=z.m_Z;  M34=0.0f;
}

/*
*/
inline void CMatrix4x4_x86::operator *= ( const CMatrix4x4_x86 &_m1 )
{
	for( uint32 i=0; i<4; i++ )
	{
		fp4 mi0 = m_Mat[i][0];
		fp4 mi1 = m_Mat[i][1];
		fp4 mi2 = m_Mat[i][2];
		fp4 mi3 = m_Mat[i][3];

		m_Mat[i][0] = mi0*_m1.m_Mat[0][0] + mi1*_m1.m_Mat[1][0] + mi2*_m1.m_Mat[2][0] + mi3*_m1.m_Mat[3][0];
		m_Mat[i][1] = mi0*_m1.m_Mat[0][1] + mi1*_m1.m_Mat[1][1] + mi2*_m1.m_Mat[2][1] + mi3*_m1.m_Mat[3][1];
		m_Mat[i][2] = mi0*_m1.m_Mat[0][2] + mi1*_m1.m_Mat[1][2] + mi2*_m1.m_Mat[2][2] + mi3*_m1.m_Mat[3][2];
		m_Mat[i][3] = mi0*_m1.m_Mat[0][3] + mi1*_m1.m_Mat[1][3] + mi2*_m1.m_Mat[2][3] + mi3*_m1.m_Mat[3][3];
	}
}

/*
*/
inline void  CMatrix4x4_x86::Rotate( const CVector3_x86 &_vec, const fp4 _a )
{
	CVector3_x86	v( _vec );
	v.NormalizeFast();

	fp4 sa, ca;
	SinCos( _a, &sa, &ca );

	CMatrix4x4_x86 rotM;
	rotM.M11 = ca + (1.0f - ca) * v.m_X * v.m_X;
	rotM.M12 = (1.0f - ca) * v.m_X * v.m_Y - sa * v.m_Z;
	rotM.M13 = (1.0f - ca) * v.m_Z * v.m_X + sa * v.m_Y;
	rotM.M21 = (1.0f - ca) * v.m_X * v.m_Y + sa * v.m_Z;
	rotM.M22 = ca + (1.0f - ca) * v.m_Y * v.m_Y;
	rotM.M23 = (1.0f - ca) * v.m_Y * v.m_Z - sa * v.m_X;
	rotM.M31 = (1.0f - ca) * v.m_Z * v.m_X - sa * v.m_Y;
	rotM.M32 = (1.0f - ca) * v.m_Y * v.m_Z + sa * v.m_X;
	rotM.M33 = ca + (1.0f - ca) * v.m_Z * v.m_Z;

	(*this) *= rotM;
}

/*
*/
inline void CMatrix4x4_x86::Multiply( const CVector4_x86 &_src, CVector4_x86 &_dst ) const
{
	_dst.m_X = M11*_src.m_X + M21*_src.m_Y + M31*_src.m_Z + M41*_src.m_W;
	_dst.m_Y = M12*_src.m_X + M22*_src.m_Y + M32*_src.m_Z + M42*_src.m_W;
	_dst.m_Z = M13*_src.m_X + M23*_src.m_Y + M33*_src.m_Z + M43*_src.m_W;
	_dst.m_W = M14*_src.m_X + M24*_src.m_Y + M34*_src.m_Z + M44*_src.m_W;
}

/*
*/
inline void CMatrix4x4_x86::Multiply( const CVector3_x86 &_src, CVector3_x86 &_dst ) const
{
	_dst.m_X = M11*_src.m_X + M21*_src.m_Y + M31*_src.m_Z + M41;
	_dst.m_Y = M12*_src.m_X + M22*_src.m_Y + M32*_src.m_Z + M42;
	_dst.m_Z = M13*_src.m_X + M23*_src.m_Y + M33*_src.m_Z + M43;
}

/*
*/
static inline CMatrix4x4_x86 operator * ( const CMatrix4x4_x86 &_m0, const CMatrix4x4_x86 &_m1 )
{
	CMatrix4x4_x86 m2(

	_m0.m_Mat[0][0]*_m1.m_Mat[0][0] + _m0.m_Mat[0][1]*_m1.m_Mat[1][0] + _m0.m_Mat[0][2]*_m1.m_Mat[2][0] + _m0.m_Mat[0][3]*_m1.m_Mat[3][0],
	_m0.m_Mat[0][0]*_m1.m_Mat[0][1] + _m0.m_Mat[0][1]*_m1.m_Mat[1][1] + _m0.m_Mat[0][2]*_m1.m_Mat[2][1] + _m0.m_Mat[0][3]*_m1.m_Mat[3][1],
	_m0.m_Mat[0][0]*_m1.m_Mat[0][2] + _m0.m_Mat[0][1]*_m1.m_Mat[1][2] + _m0.m_Mat[0][2]*_m1.m_Mat[2][2] + _m0.m_Mat[0][3]*_m1.m_Mat[3][2],
	_m0.m_Mat[0][0]*_m1.m_Mat[0][3] + _m0.m_Mat[0][1]*_m1.m_Mat[1][3] + _m0.m_Mat[0][2]*_m1.m_Mat[2][3] + _m0.m_Mat[0][3]*_m1.m_Mat[3][3],

	_m0.m_Mat[1][0]*_m1.m_Mat[0][0] + _m0.m_Mat[1][1]*_m1.m_Mat[1][0] + _m0.m_Mat[1][2]*_m1.m_Mat[2][0] + _m0.m_Mat[1][3]*_m1.m_Mat[3][0],
	_m0.m_Mat[1][0]*_m1.m_Mat[0][1] + _m0.m_Mat[1][1]*_m1.m_Mat[1][1] + _m0.m_Mat[1][2]*_m1.m_Mat[2][1] + _m0.m_Mat[1][3]*_m1.m_Mat[3][1],
	_m0.m_Mat[1][0]*_m1.m_Mat[0][2] + _m0.m_Mat[1][1]*_m1.m_Mat[1][2] + _m0.m_Mat[1][2]*_m1.m_Mat[2][2] + _m0.m_Mat[1][3]*_m1.m_Mat[3][2],
	_m0.m_Mat[1][0]*_m1.m_Mat[0][3] + _m0.m_Mat[1][1]*_m1.m_Mat[1][3] + _m0.m_Mat[1][2]*_m1.m_Mat[2][3] + _m0.m_Mat[1][3]*_m1.m_Mat[3][3],

	_m0.m_Mat[2][0]*_m1.m_Mat[0][0] + _m0.m_Mat[2][1]*_m1.m_Mat[1][0] + _m0.m_Mat[2][2]*_m1.m_Mat[2][0] + _m0.m_Mat[2][3]*_m1.m_Mat[3][0],
	_m0.m_Mat[2][0]*_m1.m_Mat[0][1] + _m0.m_Mat[2][1]*_m1.m_Mat[1][1] + _m0.m_Mat[2][2]*_m1.m_Mat[2][1] + _m0.m_Mat[2][3]*_m1.m_Mat[3][1],
	_m0.m_Mat[2][0]*_m1.m_Mat[0][2] + _m0.m_Mat[2][1]*_m1.m_Mat[1][2] + _m0.m_Mat[2][2]*_m1.m_Mat[2][2] + _m0.m_Mat[2][3]*_m1.m_Mat[3][2],
	_m0.m_Mat[2][0]*_m1.m_Mat[0][3] + _m0.m_Mat[2][1]*_m1.m_Mat[1][3] + _m0.m_Mat[2][2]*_m1.m_Mat[2][3] + _m0.m_Mat[2][3]*_m1.m_Mat[3][3],

	_m0.m_Mat[3][0]*_m1.m_Mat[0][0] + _m0.m_Mat[3][1]*_m1.m_Mat[1][0] + _m0.m_Mat[3][2]*_m1.m_Mat[2][0] + _m0.m_Mat[3][3]*_m1.m_Mat[3][0],
	_m0.m_Mat[3][0]*_m1.m_Mat[0][1] + _m0.m_Mat[3][1]*_m1.m_Mat[1][1] + _m0.m_Mat[3][2]*_m1.m_Mat[2][1] + _m0.m_Mat[3][3]*_m1.m_Mat[3][1],
	_m0.m_Mat[3][0]*_m1.m_Mat[0][2] + _m0.m_Mat[3][1]*_m1.m_Mat[1][2] + _m0.m_Mat[3][2]*_m1.m_Mat[2][2] + _m0.m_Mat[3][3]*_m1.m_Mat[3][2],
	_m0.m_Mat[3][0]*_m1.m_Mat[0][3] + _m0.m_Mat[3][1]*_m1.m_Mat[1][3] + _m0.m_Mat[3][2]*_m1.m_Mat[2][3] + _m0.m_Mat[3][3]*_m1.m_Mat[3][3]);

	return( m2 );
}

/*
*/
static inline CVector3_x86 operator * ( const CMatrix4x4_x86 &_m, const CVector3_x86 &_v )
{
	return( CVector3_x86(	_m.M11*_v.m_X + _m.M21*_v.m_Y + _m.M31*_v.m_Z + _m.M41,
							_m.M12*_v.m_X + _m.M22*_v.m_Y + _m.M32*_v.m_Z + _m.M42,
							_m.M13*_v.m_X + _m.M23*_v.m_Y + _m.M33*_v.m_Z + _m.M43 ) );
}

/*
*/
static inline CVector4_x86 operator * ( const CMatrix4x4_x86 &_m, const CVector4_x86 &_v )
{
	return( CVector4_x86(	_m.M11*_v.m_X + _m.M21*_v.m_Y + _m.M31*_v.m_Z + _m.M41*_v.m_W,
							_m.M12*_v.m_X + _m.M22*_v.m_Y + _m.M32*_v.m_Z + _m.M42*_v.m_W,
							_m.M13*_v.m_X + _m.M23*_v.m_Y + _m.M33*_v.m_Z + _m.M43*_v.m_W,
							_m.M14*_v.m_X + _m.M24*_v.m_Y + _m.M34*_v.m_Z + _m.M44*_v.m_W ) );
}

};

};

#endif
