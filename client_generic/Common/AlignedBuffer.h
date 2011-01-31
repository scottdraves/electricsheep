/*
 *  AlignedBuffer.h
 *  
 *	Implements page aligned buffer
 *
 *  Created by dasvo on 2/24/09.
 *
 */

#ifndef	_ALIGNEDBUFFER_H_
#define	_ALIGNEDBUFFER_H_

#include	<stdint.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include	"base.h"
#include	"SmartPtr.h"
#include	"Singleton.h"
#include	<boost/thread.hpp>

#ifdef LINUX_GNU
#include <stdio.h>
#endif

#define BUFFER_CACHE_SIZE 20

namespace	Base
{

MakeSmartPointers( CReusableAlignedBuffers );

//idea proposed by F-D. Cami
class CReusableAlignedBuffers : public CSingleton<CReusableAlignedBuffers>
{
	typedef struct
	{
		uint32 seed;
		
		uint32 size;
		
		uint8 *ptr;
	} BufferElement;

	BufferElement m_BufferCache[ BUFFER_CACHE_SIZE ];
	
	uint32 m_Seed;
	
	static uint32 s_PageSize;
	
	boost::mutex	m_CacheLock;

	public:
		CReusableAlignedBuffers();
		
		~CReusableAlignedBuffers();
				
		uint8* Allocate( uint32 size );
		
		void Free( uint8 *ptr, uint32 size );
		
		static void RealFree( uint8* ptr, uint32 size );
		
		uint8* Reallocate ( uint8* ptr, uint32 size );
		
		static inline uint32 GetPageSize( void )
		{
			if ( s_PageSize == 0 )
			{
		#if WIN32
				SYSTEM_INFO system_info;
				GetSystemInfo (&system_info);
				s_PageSize = (uint32)system_info.dwPageSize;
		#else
				s_PageSize = getpagesize();
		#endif
			}
			
			return s_PageSize; 
		}
		
		const bool	Shutdown( void ) { return true; }
		
		const char *Description()	{	return "CReusableAlignedBuffers";	}
		
		//	Provides singleton access.
		static CReusableAlignedBuffers *Instance( const char *_pFileStr, const uint32 _line, const char *_pFunc )
		{
			static	CReusableAlignedBuffers	rab;

			if( rab.SingletonActive() == false )
			{
				printf( "Trying to access shutdown singleton %s\n", rab.Description() );
				return NULL;
			}

			return( &rab );
		}
};

#define	g_ReusableAlignedBuffers	Base::CReusableAlignedBuffers::Instance( __FILE__, __LINE__, __FUNCTION__ )

/*
	CAlignedBuffer.

*/
MakeSmartPointers( CAlignedBuffer );

class	CAlignedBuffer
{
	uint8	*m_Buffer;
	
	uint8	*m_BufferAlignedStart;
	
	uint32	m_Size;
	
	public:
			CAlignedBuffer();
			
			CAlignedBuffer( uint32 size );
			
			~CAlignedBuffer();
			
			bool Allocate(uint32 size);
			
			bool Reallocate(uint32 size);
			
			void Free( void );
			
			uint8 * GetBufferPtr( void ) const;
			
			bool IsValid() const { return ( m_Buffer != NULL ); }
};

};

#endif

