///////////////////////////////////////////////////////////////////////////////
//
//    electricsheep for windows - collaborative screensaver
//    Copyright 2003 Nicholas Long <nlong@cox.net>
//	  electricsheep for windows is based of software
//	  written by Scott Draves <source@electricsheep.org>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
///////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include	"AlignedBuffer.h"

namespace	Base
{

uint32 CReusableAlignedBuffers::s_PageSize = 0;

/*
	CReusableBuffers().

*/
CReusableAlignedBuffers::CReusableAlignedBuffers()
{
	for ( uint32 i = 0; i < sizeof(m_BufferCache)/sizeof(*m_BufferCache); i++ )
	{
		m_BufferCache[i].seed = 0;
		m_BufferCache[i].size = 0;
		m_BufferCache[i].ptr = NULL;
	}
	
	m_Seed = 0;
}

/*
	~CReusableAlignedBuffers().

*/
CReusableAlignedBuffers::~CReusableAlignedBuffers()
{
	for ( uint32 i = 0; i < sizeof(m_BufferCache)/sizeof(*m_BufferCache); i++ )
	{
		RealFree( m_BufferCache[i].ptr,  m_BufferCache[i].size );
		
		m_BufferCache[i].seed = 0;
		m_BufferCache[i].size = 0;
		m_BufferCache[i].ptr = NULL;
	}
	
	SingletonActive( false );
}

/*
	Allocate().

*/
uint8 *CReusableAlignedBuffers::Allocate( uint32 size )
{
	{
		boost::mutex::scoped_lock locker( m_CacheLock );

		for ( uint32 i = 0; i < sizeof(m_BufferCache)/sizeof(*m_BufferCache); i++ )
		{
			BufferElement *elptr = m_BufferCache + i;
			
			if ( elptr->ptr != NULL && elptr->size == size )
			{
				uint8* retval = elptr->ptr;
				
				elptr->ptr = NULL;
				
				return retval;
			}

		}
	}
	
#ifdef WIN32
	//no valloc so malloc for now on WIN32. Needs to be implemented properly
	return (uint8 *)malloc( size + GetPageSize() - 1 );
#else
	return (uint8 *)valloc( size );
	//m_Buffer = (uint8 *)mmap( NULL, size, PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0 );
#endif
}

void CReusableAlignedBuffers::Free( uint8 *buffer, uint32 size )
{
	if ( buffer == NULL )
		return;
		
	boost::mutex::scoped_lock locker( m_CacheLock );
	
	uint32 minseed = 0xFFFFFFFF;
	uint32 mini = 0;
		
	for ( uint32 i = 0; i < sizeof(m_BufferCache)/sizeof(*m_BufferCache); i++ )
	{
		if ( m_BufferCache[i].ptr == NULL )
		{		
			mini = i;
			break;
		}
				
		//find the entry with the minimal seed, i.e. inserted the earliest
		if (minseed > m_BufferCache[i].seed)
		{
			mini = i;
			minseed = m_BufferCache[i].seed;
		}
	}
	
	//did not find - really allocate and set to mini
	
	BufferElement *elptr = m_BufferCache + mini;
	
	RealFree( elptr->ptr, elptr->size );
	
	elptr->ptr = buffer;
	
	elptr->size = size;
	
	elptr->seed = m_Seed++;
}

uint8* CReusableAlignedBuffers::Reallocate( uint8 *buffer, uint32 size )
{
	return (uint8*)realloc( buffer, size + GetPageSize() - 1 );
}


void CReusableAlignedBuffers::RealFree( uint8 *buffer, uint32 size )
{
	if ( buffer != NULL )
		free( buffer );
}

/*
	CAlignedBuffer().

*/
CAlignedBuffer::CAlignedBuffer() : m_Buffer( NULL )
{
}


/*
	CAlignedBuffer( size ).

*/
CAlignedBuffer::CAlignedBuffer( uint32 size ) : m_Buffer( NULL )
{
	Allocate( size );
}



/*
	~CAlignedBuffer().

*/
CAlignedBuffer::~CAlignedBuffer()
{
	Free();
}

/*
	Allocate().

*/
bool CAlignedBuffer::Allocate( uint32 size )
{
	CReusableAlignedBuffers* rab = g_ReusableAlignedBuffers;
	
	if (rab == NULL)
		return false;

	m_Buffer = rab->Allocate( size );
	
	unsigned long mask = CReusableAlignedBuffers::GetPageSize() - 1;
	
	m_BufferAlignedStart = (uint8*)((unsigned long)(m_Buffer + mask) & ~mask);

	m_Size = size;
	
	return ( m_Buffer != NULL );
}

/*
	Reallocate().

*/
bool CAlignedBuffer::Reallocate( uint32 size )
{
	CReusableAlignedBuffers* rab = g_ReusableAlignedBuffers;
	
	if (rab == NULL)
		return false;
	
	m_Buffer = (uint8 *)rab->Reallocate( m_Buffer, size );
	
	unsigned long mask = CReusableAlignedBuffers::GetPageSize() - 1;
	
	m_BufferAlignedStart = (uint8*)((unsigned long)(m_Buffer + mask) & ~mask);

	m_Size = size;
	
	return ( m_Buffer != NULL );
}


/*
	Free().

*/
void CAlignedBuffer::Free( void )
{
	CReusableAlignedBuffers* rab = g_ReusableAlignedBuffers;
	
	if ( rab != NULL )
		rab->Free( m_Buffer, m_Size );
	else
		CReusableAlignedBuffers::RealFree( m_Buffer, m_Size );
}

/*
	GetBufferPtr().

*/
uint8 *CAlignedBuffer::GetBufferPtr( void ) const
{
	return m_BufferAlignedStart;
}

};
