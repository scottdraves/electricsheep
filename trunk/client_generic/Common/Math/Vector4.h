#ifndef	_VECTOR4_H
#define	_VECTOR4_H

#include	"base.h"
#include	"MathBase.h"
#include	<algorithm>

namespace	Base
{

namespace	Math
{
/*#ifdef	USE_SIMD_SSE
	#include	"Vector4_SSE.h"
	typedef	Base::Math::CVector4_SSE	CVector4;
#else*/
	#include	"Vector4_X86.h"
	typedef	Base::Math::CVector4_x86	CVector4;
//#endif
};

};

#endif
