#ifndef	__LINKPOOL_H_
#define	__LINKPOOL_H_

#include "Log.h"
#include "pool.h"

/*

	Typical freelist type static pool template.
	Uses unallocated memory to keep a list of nodes
*/

namespace	Memory
{

//	Unallocated memory in the linkpool will use this to point to fresh mem.
struct CChunk
{
	CChunk	*m_pNext;
};

/*
	CLinkPool.
	When allocation fails, the pool will grow according to TGrow, and try the allocation a second time. (4 seems reasonable... )
*/
template <size_t _size, size_t TGrow = 4> class CLinkPool : public CPoolBase
{
	bool	m_bSingletonActive;

	//	No copy constructor or assignment operator.
	CLinkPool( const CLinkPool & );
	void operator = ( const CLinkPool & );

	//	Private constructor & destructor.
	CLinkPool() : m_bSingletonActive( true )
	{
		if( _size < sizeof(CChunk) )
			g_Log->Info( "Pool<%d> created with %d bytes overhead!", sizeof(CChunk) - _size );
		else
			g_Log->Info( "Pool<%d> created", _size );
	};

	virtual ~CLinkPool()
	{
		m_bSingletonActive = false;
		Purge();
	}

	//	Minimum size per entry is sizeof( CChunk ).
	static size_t Pad( size_t _wantedSize )	{	return _wantedSize >= sizeof( CChunk ) ? _wantedSize: sizeof( CChunk );	}

	//	Root pointer.
	static CChunk	*m_pMemoryPool;

	//
	inline void Push( CChunk *_pNode )
	{
		ASSERT( _pNode );
		_pNode->m_pNext = m_pMemoryPool;
		m_pMemoryPool = _pNode;
	}

	//
	inline CChunk	*Pop()
	{
		CChunk	*pNode = m_pMemoryPool;
		if( pNode )
			m_pMemoryPool = pNode->m_pNext;

		return pNode;
	}

	unsigned int	m_Count;

	public:
			//	Make sure there are _number of objects in the pool.
			void	Reserve( const unsigned int _number )
			{
				g_Log->Info( "Reserving %d objects for pool of size %d...", _number, _size );

				//	Preallocate.
				for( unsigned int i=0; i<_number; i++ )
					Push( reinterpret_cast<CChunk *>( AllocSys( Pad( _size ) ) ) );

				m_Count = 0;
			}

			//
			void	*Allocate()
			{
				//	Unlink a chunk from the pool.
				void *pMem = Pop();
				if( pMem == NULL )
				{
					//	Try growing a bit.
					Reserve( TGrow );

					//	...and try again...
					pMem = Pop();
				}

				return pMem;
			}

			//	Deallocate, ie give memory back to the pool.
			void	Deallocate( void *_pData )
			{
				//	Link chunk back mem into pool.
				Push( reinterpret_cast<CChunk *>(_pData) );
			}

			//	Purge the pool, give back all it's memory.
			virtual void Purge()
			{
				while( m_pMemoryPool )
				{
					char *pData = (char *)Pop();
					if( pData )
						delete pData;
				}
			}

			//	Used below to track access.
			bool	SingletonActive( void	)	{	return( m_bSingletonActive );	};

			//	Return instance.
			static CLinkPool &Instance()
			{
				static CLinkPool instance;

				//if( instance.SingletonActive() == false )
					//printf( "Trying to access shutdown singleton Memory::CLinkPool<%d>\n", _size );

				return( instance );
			}
};

//
template <size_t _size, size_t TGrow> CChunk *CLinkPool<_size, TGrow>::m_pMemoryPool = NULL;

};

#endif
