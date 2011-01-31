/*
	CException.
	Author:	Stef.

	Exception handler.

	example:

	try
	{
		uint32	value = something.Function();

		if( value == 0 )
			ThrowStr( "Something returned 0!" );
		else
			ThrowArgs( ="Something returned %d!", value ) );
	}
	catch( CException &exception )
	{
		SAFE_DELETE( pData );
		exception.ReportCatch();
	}
*/
#ifndef __EXCEPTION_H_
#define __EXCEPTION_H_

#include	<string>
#include	"base.h"

namespace Base
{

/*
	CException().

*/
class	CException
{
	CException( const char *_sz, const char *_szFile, const uint32 _line )
	{
		m_File = _szFile;
		m_String = _sz;
		m_Line = _line;
	}

	CException( const std::string &_s, const char * const _szFile, const uint32 _line )
	{
		m_File = _szFile;
		m_String = _s;
		m_Line = _line;
	}

	std::string	m_String;
	std::string	m_File;
	uint32	m_Line;

	// for PrintfToStatic :
	static std::string	m_ArgString;

	public:
			//
			void	ReportCatch( void ) const;

			std::string	Text( void ) const;

			//
			static void Throw( char *_sz, char *_szFile,const uint32 _line )
			{
				throw( CException( _sz, _szFile, _line ) );
			}

			//
			static void Throw( const std::string &_s, char *_szFile, const uint32 _line )
			{
				throw( CException( _s, _szFile, _line ) );
			}

			//
			static void Throw( char *_szFile, const uint32 _line )
			{
				throw( CException( m_ArgString, _szFile, _line ) );
			}

			//
			static void CollectArguments( const char *_String, ... );	//	Args.
};

};


//	Throw an exception with arguments. For example: ThrowArgs( ("Bad ass number %d", 42) );  DON´T FORGET THE EXTRA ()!
#define ThrowArgs( varargs ) \
	Base::CException::CollectArguments varargs, \
	Base::CException::Throw( __FILE__, __LINE__ )

//	Throw a simple CStr exception.
#define ThrowStr( str )	Base::CException::Throw( str, __FILE__, __LINE__ )

#endif
