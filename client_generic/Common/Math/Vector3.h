/*
	3 component vector class.
*/
#ifndef	_VECTOR3_H
#define	_VECTOR3_H

#include	"base.h"
#include	"MathBase.h"

namespace	Base
{

namespace	Math
{
/*#ifdef	USE_SIMD_SSE
	#include	"Vector3_SSE.h"
	typedef	Base::Math::CVector3_SSE	CVector3;
#else*/
	#include	"Vector3_X86.h"
	typedef	Base::Math::CVector3_x86	CVector3;
//#endif

};

};

#endif
