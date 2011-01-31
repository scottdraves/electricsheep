#ifndef	_POOL_H_
#define _POOL_H_

namespace	Memory
{

/*
	CPoolBase.
	Abstract baseclass defining pools.
*/
class CPoolBase
{
	public:
			virtual void Purge() = 0;

			//	System heap allocation.
			static void	*AllocSys( size_t _size );
			static void	DeallocSys( void *_pData );
};


//	Spiffy macro for declaring classes poolable.
#define POOLED( classtype, pooltype ) \
	public: \
			static void *operator new( size_t s )	{ ASSERT( s == sizeof( classtype ) ); return pooltype< sizeof(classtype)>::Instance().Allocate(); } \
			static void operator delete( void *_p )	{ if( _p )	pooltype< sizeof(classtype)>::Instance().Deallocate( _p );	} \

};

#endif
