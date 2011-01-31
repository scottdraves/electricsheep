/*
	2 component vector class.
*/
#ifndef	_VECTOR2_H
#define	_VECTOR2_H

#include	"base.h"
#include	"MathBase.h"

namespace	Base
{

namespace	Math
{
/*#ifdef	USE_SIMD_SSE
	#include	"Vector2_SSE.h"
	typedef	Base::Math::CVector2_SSE	CVector2;
#else*/
	#include	"Vector2_X86.h"
	typedef	Base::Math::CVector2_x86	CVector2;
//#endif

};

};

#endif
