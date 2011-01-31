#ifndef	_MATHBASE_H_
#define	_MATHBASE_H_

#include	<math.h>

namespace	Base
{

namespace	Math
{

#ifndef	INFINITY
	#define	INFINITY	3.402823466e+38F
#endif

#ifndef	M_E
	#define M_E 2.71828182845904523536f
#endif

#ifndef M_PI
	#define M_PI 3.14159265358979323846f
#endif

#ifndef M_PI05
	#define M_PI05 (M_PI * 0.5f)
#endif

#ifndef M_PI2
	#define M_PI2 (M_PI * 2.0f)
#endif

#ifndef M_SQR
	#define M_SQR(_a) ( (_a)*(_a) )
#endif

#ifndef M_GOLDENRATIO	//	Fibonnaci sequence.
	#define M_GOLDENRATIO 1.61803398874989484820458683436563811772f
#endif

#ifndef EPSILON
	#define EPSILON 0.000001f
#endif

#ifndef	Random
	#define Random (((float)rand())/((float)RAND_MAX))
#endif

#ifndef M_PI05
	#define M_PI05 (M_PI * 0.5f)
#endif

#ifndef	o_M_PI
	#define	o_M_PI	(1.0f / M_PI)
#endif

#ifndef	o_M_PI2
	#define	o_M_PI2	(1.0f / M_PI2)
#endif

#ifndef	lerpMacro
	#define	lerpMacro(a,b,c)	( (a) + ( ((b) - (a) ) * (c) ) )
#endif


/*#ifndef	clamp
	#define clamp(x, lo, hi) max(min(x, hi), lo)
#endif*/

#ifndef saturate
	#define saturate(x) Clamped((x), 0, 1 )
#endif

#ifndef Deg2Rad
	#define Deg2Rad(_n)     ((_n) * 0.017453292f)
#endif

#ifndef Rad2Deg
	#define Rad2Deg(n)     ((n) * 57.29577951f)
#endif


//	Handy clamp template.
template<class T,class T1,class T2> inline T Clamped( const T &_x,const T1 &_lo,const T2 &_hi ) { return( ( _x < _lo ) ? _lo : ( _x > _hi ) ? _hi : _x ); }

//	Sine & Cosine of _angle.	For x86 roughly twice as fast as sinf() + cosf()...
inline void SinCos( const fp4 _angle, fp4 *_pSin, fp4 *_pCos )
{
//#ifdef AMD64
	*_pSin = (fp4)sinf( _angle );
	*_pCos = (fp4)cosf( _angle );
/*#else
	__asm
	{
		fld _angle;
		fsincos;
		mov eax, _pCos;
		fstp [eax];
		mov eax, _pSin;
		fstp [eax];
	};
#endif*/
}

//	Squareroot.	MAKE BETTER!
inline fp4 Sqrt( const fp4 _f )
{
//	ASSERTFP4( _f );
//	ASSERT( _f != 0.0f );
	return( sqrtf( _f ) );
}

//	Reciprocal squareroot.	(~92 clocks)	MAKE BETTER!
inline fp4	RSqrt( const fp4 _f )
{
//	ASSERTFP4( _f );
	return( 1.0f / Sqrt(_f) );
}

//	Fast approximate reciprocal squareroot.	(~31 clocks)
inline fp4 RSqrtFast( fp4 _v )
{
	//	ASSERTFP4( _v );
	//	ASSERT( _v != 0.0f );

	const fp4	v_half = _v * 0.5f;
	long i = *(long *) &_v;
	i = 0x5f3759df - (i >> 1);
	_v = *(fp4 *) &i;
	return( _v * (1.5f - v_half * _v * _v) );
}


/*
	fist().
	Faster float2int conversion.
*/
/*inline long	fist( float src )
{
	long	res = 0;
	long	*pDst = &res;

	#ifndef AMD64
	__asm {
					mov		eax, pDst
					fld		src
					fistp [eax]
				};
	#endif

	return( res );
}*/

/*
	daPowf().
	Faster powf().
*/
/*inline	float	daPowf( float x, float y )
{
	float	powed;

	#ifndef AMD64
	__asm	{
					fld y
					fld	x

					fyl2x
					fld1
					fld st(1)
					fprem
					f2xm1
					faddp st(1), st
					fscale
					fxch
					fstp st

					fstp powed
	};
	#endif

	return( powed );
}*/

/*
	ClosestPowerOfTwo().

*/
inline const unsigned int ClosestPowerOfTwo( const unsigned int _x )
{
	int	i,k;

	k = _x;
	i = -1;

	while( k != 0)
	{
		k >>= 1;
		i++;
	}

	return( 1 << (i + ((_x >> (i-1)) & 1)) );
}

/*
	UpperPowerOfTwo().

*/
inline const unsigned int UpperPowerOfTwo( const unsigned int x )
{
	unsigned int i = 1;

	while( i<x )
		i += i;

	return( i );
}

//
static inline int FloatIsNAN( const float &_Number)
{
	if (( (*((unsigned int*)&_Number)) & 0x7F800000) == 0x7F800000)
	{
		if ((*((unsigned int*)&_Number)) & 0x007FFFFF)
			return( true );
		else
			return( false );
	}
	else
		return( false );
}

//
static inline int FloatIsInfinite(const float &_Number)
{
	if (((*((unsigned int*)&_Number)) & 0x7F800000) == 0x7F800000)
	{
		if (!((*((unsigned int*)&_Number)) & 0x007FFFFF))
			return( true );
		else
			return( false );
	}
	else
		return( false );
}

//
static inline bool FloatInRange( const float& _Number, const float _Low, const float _High )
{
	return ( ( _Number >= _Low ) && ( _Number <= _High ) );
}

//
static inline int FloatIsInvalid(const float &_Number)
{
	return (((*((unsigned int*)&_Number)) & 0x7F800000) == 0x7F800000);
}

};

};

#endif
