/*
	3x3 and 4x4 matrix classes.
*/
#ifndef	_MATRIX_H_
#define	_MATRIX_H_

#include	"base.h"
#include	"MathBase.h"

namespace	Base
{

namespace	Math
{
/*#ifdef	USE_SIMD_SSE

	#include	"Matrix3x3_SSE.h"
	typedef	DisplayOutput::Math::CMatrix3x3_SSE	CMatrix3x3;

	#include	"Matrix4x4_SSE.h"
	typedef	DisplayOutput::Math::CMatrix4x4_SSE	CMatrix4x4;

#else*/

	#include	"Matrix3x3_x86.h"
	typedef	Base::Math::CMatrix3x3_x86	CMatrix3x3;

	#include	"Matrix4x4_x86.h"
	typedef	Base::Math::CMatrix4x4_x86	CMatrix4x4;

//#endif

};

};

#endif
