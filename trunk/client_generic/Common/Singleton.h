/*
	SINGLETON.H
	Author: Stef.

	Singleton template.
*/
#ifndef	_SINGLETON_H_
#define	_SINGLETON_H_

#include <stdio.h>

#include	"base.h"
#include	"Exception.h"

namespace	Base
{

/*
	CSingleton.

*/
template <typename T> class CSingleton
{
	//	We do not want these.
	CSingleton( const CSingleton & );
	CSingleton & operator = ( const CSingleton & );

	protected:

		//
		bool		m_bSingletonActive;

	public:
			CSingleton() : m_bSingletonActive(true)	{};
			virtual ~CSingleton()	{};

			//	Users must implement this and set Active( false ), to catch unintended access after destruction.
			virtual bool Shutdown( void ) = PureVirtual;

			//
			void		SingletonActive( const bool _state )	{	m_bSingletonActive = _state;	};
			bool	SingletonActive( void	)				{	return( m_bSingletonActive );	};
			virtual const char *Description() = PureVirtual;

			//	Return instance.
			static T &Instance()
			{
				static T instance;

				if( instance.SingletonActive() == false )
				{
#ifndef MAC //needs to be implemented as proper singleton before this is true
					printf( "Trying to access shutdown singleton %s\n", instance.Description() );
#endif
					//ThrowArgs(( "Trying to access shutdown singleton %s", instance.Description() ));
				}

				return( instance );
			}
};

};

#endif
