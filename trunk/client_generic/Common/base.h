/*
	BASE.H
	Author: Keffo.

	Self-explanatory. Should be included first of all.
*/
#ifndef	_BASE_H_
#define	_BASE_H_

#ifdef LINUX_GNU
#include <inttypes.h>
#endif

//	Standard signed types.
typedef	signed char			int8;
typedef	signed short		int16;
typedef	signed int			int32;

//	Standard unsigned types.
typedef	unsigned char		uint8;
typedef	unsigned short		uint16;
typedef	unsigned int		uint32;

//	64bit standard types.
#ifdef _MSC_VER
typedef	__int64				int64;
typedef	unsigned __int64	uint64;
#else
typedef	int64_t				int64;
typedef	uint64_t			uint64;
#endif
//	Single & double precision floating points.
typedef	float				fp4;
typedef	double				fp8;

//	Lazy.
#define	isBit( v, b )	((v)&(b))
#define setBit( v, b )	((v)|=(b))
#define remBit( v, b )	((v)&=~(b))

/*
	Helper macros to make some stuff easier.

*/
#define SAFE_DELETE(p)			{ if( p )	{ delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if( p )	{ delete[] (p);     (p)=NULL; } }
#define SAFE_RELEASE(p)			{ if( p )	{ (p)->Release(); (p)=NULL; } }

//	Duh.
#define NO_DEFAULT_CTOR(_x) _x()

//	Disallows the compiler defined copy constructor.
#define NO_COPY_CTOR(_x)    _x( const _x& )

//	Disallows the compiler defined assignment operator.
#define NO_ASSIGNMENT(_x)   void operator = ( const _x& )

//	Lazy.
#define NO_CLASS_STANDARDS(_x)	\
		NO_ASSIGNMENT(_x);	\
		NO_COPY_CTOR(_x)


//
#define PureVirtual 0

#ifndef	NULL
#define	NULL 0
#endif

/*
	Assert disco.

*/
#ifdef DEBUG
	#include <assert.h>
	#define ASSERT	assert
#else
#define ASSERT(b)
#endif

#endif
