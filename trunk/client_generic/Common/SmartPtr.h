/*
	SMARTPTR.H
	Author: Stef.

	Reference counted smart pointer with garbage collection and/or object level thread synchronization.

	SmartPtr			-	The SmartPtr class
	CRefCountPtr		-	Reference Counting Garbage Collection
	CSyncPtr			-	Synchronized access without Reference Counting Garbage Collection
	CSyncRefCountPtr	-	Synchronized access with Reference Counting Garbage Collection
	CSmartPtrBase		-	The base of all above classes. Used as a common base so an assignment between different pointers is able.

	Examples on how to use these classes.

	1. Reference Counting.	(CRefCountPtr)
	2. Object Level Thread Synchronization. (CSyncPtr)
	3. Object Level Thread Synchronization & Reference Counting. (CSyncRefCountPtr)

	------

	1.	Reference Counting on CSomething
	------------------------------------

	class	CSomething
	{
		CSomething();
		~CSomething();
		.....
		void	do();
	};

	typedef	CRefCountPtr<CSomething>	LPSOMETHING;

	void	TestFunct()
	{
		LPSOMETHING	p1 = new CSomething;
		LPSOMETHING	p2 = p1;

		if( p1 == NULL )
		{
			....
		}

		p2->do();
		p1 = NULL;

	}	//	Here the object pointed by p2 WILL BE destroyed automatically


	2. Object Level Thread Synchronization for objects of CSomething
	---------------------------------------------------------

	typedef	SyncPtr<CSomething>	LPSOMETHING;

	void	TestFunct()
	{
		LPSOMETHING	p1 = new CSomething;
		LPSOMETHING	p2 = p1;

		if( p1.IsNULL() )
		{
			....
		}

		StartThread( p1 );

		p2->do();	//	Synchronized with the other thread
		p1 = NULL;

	}	//	Here the object pointed by p2 will NOT be destroyed automatically

	void	ThreadFunc( LPSOMETHING p )
	{
		p->do();	//	Synchronized with the other thread
	}//	Here the object pointed by p will NOT be destroyed automatically



	3. Object Level Thread Synchronization and Reference Counting
		for objects of CSomething
	---------------------------------------------------------

	typedef	CSyncRefCountPtr<CSomething>	LPSOMETHING;

	void	TestFunct()
	{
		LPSOMETHING	p1 = new CSomething;
		LPSOMETHING	p2 = p1;

		if( p1.IsNULL() )
		{
			....
		}

		StartThread( p1 );

		p2->do();	//	Synchronized with the other thread
		p1 = NULL;

	}	//	Here the object pointed by p2 WILL BE destroyed automatically if p in ThreadFunc has already released the object.

	void	ThreadFunc( LPSOMETHING p )
	{
		p->do();	//	Synchronized with the other thread
	}//	Here the object pointed by p WILL BE destroyed automatically if p2 in TestFunc has already released the object.


	WARNING:	Don't use a code like this

		CSomething *ptr = new CSomething();	//	A real pointer
		LPSOMETHING	p1 = ptr;
		LPSOMETHING	p2 = ptr;	This will lead to 2 (two) representation objects.
								So you will not have nor proper grabage collection nor proper thread synchronization.

		if( p1 != p2 )			The comparison will say these point to different objects even you think it is the same.
		{
			ASSERT( FALSE );
		}

	sizeof( CRITICAL_SECTION ) 24
	sizeof( CRefCountRep ) 8
	sizeof( CSyncAccessRep ) 32
	sizeof( CSyncRefCountRep ) 32
	sizeof( CSyncAccess ) 8
	sizeof( SmartPtr ) 4
	sizeof( CSomething* ) 4
	sizeof( int ) 4
*/
#ifndef _SMARTPTR_H_
#define _SMARTPTR_H_

#ifdef	WIN32
	//	If you wrap a non-class with the SmartPtr class, you will receive this warning. i.e.  int, short, etc...
	//#pragma warning( disable : 4284 )
	//	Debug symbols cannot be longer than 255 but when using templates it is usual to have debug symbols longer than 255.
	//#pragma warning( disable : 4786 )
	#include	<windows.h>
	#include	<stdio.h>
#else
	#include	<pthread.h>
#endif

#include	"base.h"

namespace Base
{
/**
	CRefCountRep().
	Representation class just for reference counting.
*/
template<class T> class CRefCountRep
{
	T		*m_pRealPtr;
	long	m_counter;

	//	Constructors and destructor
	public:
			CRefCountRep( const T *ptr );
			~CRefCountRep();

			long	incrRefCount();
			long	decrRefCount();

			T		*getPointer() const;
			T		*getRealPointer() const;

			bool	isNull() const;
};

//
template<class T> CRefCountRep<T>::CRefCountRep( const T *ptr ) : m_pRealPtr( (T*)ptr ), m_counter( 0 )
{
}

//
template<class T> CRefCountRep<T>::~CRefCountRep()
{
	ASSERT( m_counter <= 0 );
	SAFE_DELETE( this->m_pRealPtr );
}

//
template<class T> long	CRefCountRep<T>::incrRefCount()			{	m_counter++;	return( m_counter );	}
template<class T> long	CRefCountRep<T>::decrRefCount()			{	m_counter--;	return( m_counter );	}
template<class T> T		*CRefCountRep<T>::getPointer() const	{	return( m_pRealPtr );	}
template<class T> T		*CRefCountRep<T>::getRealPointer() const{	return( m_pRealPtr );	}
template<class T> bool	CRefCountRep<T>::isNull() const			{	return( m_pRealPtr == NULL );	}

//
template <class T>	class	CSyncAccess;

/**
	CSyncAccessRep().
	Representation class just for thread synchronization.
*/
template<class T> class CSyncAccessRep
{
	protected:
		T					*m_pRealPtr;
		long				m_counter;

#ifdef	WIN32
		CRITICAL_SECTION	m_CriticalSection;
		DWORD				m_ThreadID;
#else
		pthread_mutex_t		m_Mutex;
#endif

	public:
			CSyncAccessRep( const T *ptr );
			~CSyncAccessRep();

			long	incrRefCount();
			long	decrRefCount();

			CSyncAccess<T>	getPointer() const;
			T		*getRealPointer() const;

			bool	isNull() const;

			void	acquireAccess();
			void	releaseAccess();
};

//
template<class T>	CSyncAccessRep<T>::CSyncAccessRep( const T *ptr ) : m_pRealPtr( (T *)ptr ), m_counter( 0 )
{
#ifdef	WIN32
	::InitializeCriticalSection( &m_CriticalSection );
//	::InitializeCriticalSectionAndSpinCount( &m_CriticalSection, 0x80000400 );

	//::EnterCriticalSection( &m_CriticalSection );
	//m_ThreadID = GetCurrentThreadId();
	//printf( "New t-pointer, %d", m_ThreadID );
	//::LeaveCriticalSection( &m_CriticalSection );

#else
	pthread_mutex_init( &m_Mutex, NULL );
#endif
}

//
template<class T>	CSyncAccessRep<T>::~CSyncAccessRep()
{
	ASSERT( m_counter <= 0 );
#ifdef WIN32
	::DeleteCriticalSection( &m_CriticalSection );
#else
	pthread_mutex_destroy( &m_Mutex );
#endif
}

//
template<class T> long	CSyncAccessRep<T>::incrRefCount()
{
#ifdef	WIN32
	return(	::InterlockedIncrement( &m_counter ) );
#else
    //printf( "incrRefCount(), m_counter = %d\n", m_counter );
	pthread_mutex_lock( &m_Mutex );
	m_counter++;
	pthread_mutex_unlock( &m_Mutex );
	return( m_counter );
#endif
}

//
template<class T> long	CSyncAccessRep<T>::decrRefCount()
{
#ifdef	WIN32
	return( ::InterlockedDecrement( &m_counter ) );
#else
    //printf( "decrRefCount(), m_counter = %d\n", m_counter );
	pthread_mutex_lock( &m_Mutex );
	m_counter--;
	pthread_mutex_unlock( &m_Mutex );
	return( m_counter );
#endif
}


//	Object of type ACCESS (CSyncAccess<T>) will be automatically created on the stack.
template<class T> CSyncAccess<T>	CSyncAccessRep<T>::getPointer() const		{	return( this );	}

template<class T> T		*CSyncAccessRep<T>::getRealPointer() const	{	return( m_pRealPtr );	}
template<class T> bool	CSyncAccessRep<T>::isNull() const			{	return( m_pRealPtr == NULL );	}

template<class T> void	CSyncAccessRep<T>::acquireAccess()
{
#ifdef	WIN32
	::EnterCriticalSection( &m_CriticalSection );
	/*if( TryEnterCriticalSection( &m_CriticalSection ) != 0 )
	{
		printf( "CS owned by %d...\n", m_ThreadID );
	}
	else
		m_ThreadID = GetCurrentThreadId();*/
#else
	pthread_mutex_lock( &m_Mutex );
#endif
}

//
template<class T> void	CSyncAccessRep<T>::releaseAccess()
{
#ifdef	WIN32
	m_ThreadID = 0;
	::LeaveCriticalSection( &m_CriticalSection );
#else
	pthread_mutex_unlock( &m_Mutex );
#endif
}



/*
	CSyncRefCountRep.
	Representation class for Reference Counting AND Thread Synchronization.
*/
template<class T> class CSyncRefCountRep : public CSyncAccessRep<T>
{
	public:
			CSyncRefCountRep( const T *ptr );
			~CSyncRefCountRep();
};

//
template<class T> CSyncRefCountRep<T>::CSyncRefCountRep( const T *ptr ) : CSyncAccessRep<T>( ptr )
{
}

//
template<class T> CSyncRefCountRep<T>::~CSyncRefCountRep()
{
	ASSERT( this->m_counter <= 0 );
	//	This is the only change needed to make in CSyncRefCountRep<T> class to collect the garbage and
	//	in the same time to do Object Level Thread Synchronization.
	delete( this->m_pRealPtr );
}

/*
	CSyncAccess.
	The SyncAccess class. It is used as an intermediary to achieve object Level Thread Synchronization.
*/
template <class T> class	CSyncAccess
{
	typedef	CSyncAccessRep<T>	REP;

	REP		*m_rep;
	bool	m_acquired;

	public:
			CSyncAccess( const CSyncAccess<REP> &that );
			CSyncAccess( const CSyncAccess &that );
			CSyncAccess( const REP *rep );
			~CSyncAccess();

			T	*operator -> ();
};

//
template <class T> CSyncAccess<T>::CSyncAccess( const REP *rep ) : m_rep( (REP *)rep ), m_acquired( false )
{
}

//
template <class T> CSyncAccess<T>::CSyncAccess( const  CSyncAccess<REP> &that ) : m_rep( that.m_rep ), m_acquired( false )
{
}

//
template <class T> CSyncAccess<T>::~CSyncAccess()
{
	if( m_acquired )
		m_rep->releaseAccess();
}

//
template <class T> T	*CSyncAccess<T>::operator -> ()
{
	//	This is checked by SmartPtr<T>::operator -> () too.
	ASSERT( (m_rep != NULL) && (! m_rep->isNull()) );

	if( !m_acquired )
	{
		m_rep->acquireAccess();
		m_acquired = true;
	}

	return( m_rep->getRealPointer() );
}



/*
	SmartPtrBase.
	The SmartPtrBase class.
*/
class	SmartPtrBase
{
	public:
			SmartPtrBase() : m_rep( NULL )
			{};

			void	*m_rep;
};

/*
	SmartPtr.

*/
template<class T, class REP, class ACCESS = T *> class SmartPtr : public SmartPtrBase
{
	void	IncrRefCount();
	void	DecrRefCount();

	public:
			SmartPtr();
			~SmartPtr();



		//	Helper methods.
		void	CopyFrom( const SmartPtrBase &ptr );
		void	CopyFrom( const T *ptr );



			//
			SmartPtr( const SmartPtr &ptr );
			SmartPtr( const T *ptr );
			SmartPtr( const SmartPtrBase &ptr );

			//	Assignment Operators
			SmartPtr &operator = ( const SmartPtr &ptr );
			SmartPtr &operator = ( const T *ptr );
			SmartPtr &operator = ( const SmartPtrBase &ptr );

			//	Operators.
			ACCESS	operator -> ();
			T		&operator * ();

			//	Casting operator.
			operator T	*();

			//	Comparison Operators.
			bool	operator == ( const SmartPtrBase &ptr );
			bool	operator == ( const T *ptr );
			bool	operator != ( const SmartPtrBase &ptr );
			bool	operator != ( const T *ptr );

			//	Attributes.
			bool	IsNull() const;
			long	GetRefCount() const;
			REP		*GetRepPtr() const;
};

//
template<class T, class REP, class ACCESS> SmartPtr<T,REP,ACCESS>::SmartPtr()
{
}

//
template<class T, class REP, class ACCESS> SmartPtr<T,REP,ACCESS>::~SmartPtr()
{
	DecrRefCount();
}

//
template<class T, class REP, class ACCESS> SmartPtr<T,REP,ACCESS>::SmartPtr( const SmartPtr &ptr )
    : SmartPtrBase()
{
	CopyFrom( ptr );
}

//
template<class T, class REP, class ACCESS> SmartPtr<T,REP,ACCESS>::SmartPtr( const T *ptr )
    : SmartPtrBase()
{
	CopyFrom( ptr );
}

//
template<class T, class REP, class ACCESS> SmartPtr<T,REP,ACCESS>::SmartPtr( const SmartPtrBase &ptr )
{
	CopyFrom( ptr );
}

//
template<class T, class REP, class ACCESS> void	SmartPtr<T,REP,ACCESS>::CopyFrom( const SmartPtrBase &ptr )
{
	if( m_rep != ptr.m_rep )
	{
		DecrRefCount();
		m_rep = ptr.m_rep;
		IncrRefCount();
	}
}

//
template<class T, class REP, class ACCESS> void	SmartPtr<T,REP,ACCESS>::CopyFrom( const T *ptr )
{
	DecrRefCount();
	m_rep = (ptr != NULL) ? new REP( ptr ) : NULL;
	IncrRefCount();
}

//
template<class T, class REP, class ACCESS> SmartPtr<T,REP,ACCESS> &SmartPtr<T,REP,ACCESS>::operator = ( const SmartPtr &ptr )
{
	CopyFrom( ptr );
	return( *this );
}

//
template<class T, class REP, class ACCESS> SmartPtr<T,REP,ACCESS> &SmartPtr<T,REP,ACCESS>::operator = ( const T *ptr )
{
	CopyFrom( ptr );
	return( *this );
}

//
template<class T, class REP, class ACCESS> SmartPtr<T,REP,ACCESS> &SmartPtr<T,REP,ACCESS>::operator = ( const SmartPtrBase &ptr )
{
	CopyFrom( ptr );
	return( *this );
}

//
template<class T, class REP, class ACCESS> ACCESS	SmartPtr<T,REP,ACCESS>::operator -> ()
{
	ASSERT( ! IsNull() );
	return( GetRepPtr()->getPointer() );
}

//
template<class T, class REP, class ACCESS> T &SmartPtr<T,REP,ACCESS>::operator * ()
{
	ASSERT( ! IsNull() );
	return( *(GetRepPtr()->getRealPointer()) );
}

//
template<class T, class REP, class ACCESS> SmartPtr<T,REP,ACCESS>::operator T *()
{
	return(	( IsNull() ) ? NULL : GetRepPtr()->getRealPointer() );
}

//
template<class T, class REP, class ACCESS> bool	SmartPtr<T,REP,ACCESS>::operator == ( const SmartPtrBase &ptr )
{
	return( m_rep == ptr.m_rep );
}

//
template<class T, class REP, class ACCESS> bool	SmartPtr<T,REP,ACCESS>::operator == ( const T *ptr )
{
	if( ! IsNull() )
	{
		return( GetRepPtr()->getRealPointer() == ptr );
	}

	return( ptr == NULL );
}

//
template<class T, class REP, class ACCESS> bool SmartPtr<T,REP,ACCESS>::operator != ( const SmartPtrBase &ptr )
{
	return(	m_rep != ptr.m_rep );
}

//
template<class T, class REP, class ACCESS> bool SmartPtr<T,REP,ACCESS>::operator != ( const T *ptr )
{
	return(	! (operator ==( ptr )) );
}

//
template<class T, class REP, class ACCESS> bool	SmartPtr<T,REP,ACCESS>::IsNull() const
{
	return( m_rep == NULL );
}

//
template<class T, class REP, class ACCESS> long	SmartPtr<T,REP,ACCESS>::GetRefCount() const
{
	ASSERT( ! IsNull() );
	return(	GetRepPtr()->m_counter );
}

//
template<class T, class REP, class ACCESS> REP	*SmartPtr<T,REP,ACCESS>::GetRepPtr() const
{
	return(	(REP *)m_rep );
}

//
template<class T, class REP, class ACCESS> void	SmartPtr<T,REP,ACCESS>::IncrRefCount()
{
	if( ! IsNull() )
		GetRepPtr()->incrRefCount();
}

//
template<class T, class REP, class ACCESS> void	SmartPtr<T,REP,ACCESS>::DecrRefCount()
{
	if( ! IsNull() )
	{
		if( GetRepPtr()->decrRefCount() <= 0 )
		{
			REP	*rep = (REP	*)m_rep;
			delete	rep;
		}

		m_rep = NULL;
	}
}

/*
	CRefCountPtr.
	Helper class for easier use of the SmartPtr class.
*/
template<class T, class REP = CRefCountRep<T>, class ACCESS = T*> class	CRefCountPtr : public SmartPtr<T, REP, ACCESS>
{
	public:
			CRefCountPtr()	{};
			~CRefCountPtr()	{};

			//	Copy constructor.
			CRefCountPtr( const CRefCountPtr& ptr ) : SmartPtr<T,REP,ACCESS>( ptr )
			{};

			//
			CRefCountPtr( const SmartPtrBase& ptr ) : SmartPtr<T,REP,ACCESS>( ptr )
			{};

			//
			CRefCountPtr( const T* ptr ) : SmartPtr<T,REP,ACCESS>( ptr )
			{};

			//	Assignment Operators
			CRefCountPtr& operator = ( const CRefCountPtr& ptr )
			{
				this->CopyFrom( ptr );
				return( *this );
			}

			//
			CRefCountPtr &operator = ( const T *ptr )
			{
				this->CopyFrom( ptr );
				return( *this );
			}

			//
			CRefCountPtr &operator = ( const SmartPtrBase &ptr )
			{
			    SmartPtr<T, REP, ACCESS>::CopyFrom( ptr );
				return( *this );
			}
};

/*
	CSyncPtr.
	Helper class for easier use of the SmartPtr class.
*/
template<class T, class REP = CSyncAccessRep<T>, class ACCESS = CSyncAccess<T> > class	CSyncPtr : public SmartPtr<T, REP, ACCESS>
{
	public:
			CSyncPtr()	{};
			~CSyncPtr()	{};

			//	Copy constructor.
			CSyncPtr( const CSyncPtr &ptr ) : SmartPtr<T,REP,ACCESS>( ptr )
			{};

			//
			CSyncPtr( const SmartPtrBase &ptr ) : SmartPtr<T,REP,ACCESS>( ptr )
			{};

			//
			CSyncPtr( const T *ptr ) : SmartPtr<T,REP,ACCESS>( ptr )
			{};

			//	Assignment Operators.
			CSyncPtr	&operator = ( const CSyncPtr &ptr )
			{
				CopyFrom( ptr );
				return( *this );
			}

			//
			CSyncPtr	&operator = ( const T *ptr )
			{
				CopyFrom( ptr );
				return( *this );
			}

			//
			CSyncPtr	&operator = ( const SmartPtrBase &ptr )
			{
				SmartPtr<T, REP, ACCESS>::CopyFrom( ptr );
				return( *this );
			}
};

/*
	CSyncRefCountPtr.

	Thread-synchronized reference-counted smart-pointer.
*/
template<class T, class REP = CSyncRefCountRep<T>, class ACCESS = CSyncAccess<T> > class CSyncRefCountPtr : public SmartPtr<T, REP, ACCESS>
{
	public:
			CSyncRefCountPtr()	{};
			~CSyncRefCountPtr()	{};

			//	Copy constructor.
			CSyncRefCountPtr( const CSyncRefCountPtr &ptr ) : SmartPtr<T,REP,ACCESS>( ptr )
			{};

			//
			CSyncRefCountPtr( const SmartPtrBase &ptr ) : SmartPtr<T,REP,ACCESS>( ptr )
			{};

			//
			CSyncRefCountPtr( const T *ptr ) : SmartPtr<T,REP,ACCESS>( ptr )
			{};

			//	Assignment Operators.
			CSyncRefCountPtr &operator = ( const CSyncRefCountPtr &ptr )
			{
				CopyFrom( ptr );
				return(	*this );
			}

			//
			CSyncRefCountPtr	&operator = ( const T	*ptr )
			{
				CopyFrom( ptr );
				return(	*this );
			}

			//
			CSyncRefCountPtr	&operator = ( const SmartPtrBase &ptr )
			{
				SmartPtr<T, REP, ACCESS>::CopyFrom( ptr );
				return(	*this );
			}
};


/*
	Forward declaration and smart pointer def.

	Example:

		class	CRenderer;

		MakeSmartPointers( CRenderer );

		spCRenderer		m_spRenderer;		//	"smart" + "pointer" + class
		tpCRenderer		m_tpRenderer;		//	"thread" + "pointer" + class
		tspCRenderer	m_tspRenderer;		//	"thread" + "smart" + "pointer" + class
*/
#define MakeSmartPointers(CBase)	class CBase;	\
	typedef Base::CRefCountPtr<CBase> sp##CBase; \
	typedef Base::CSyncPtr<CBase> tp##CBase; \
	typedef Base::CSyncRefCountPtr<CBase> tsp##CBase; \

};

#endif
