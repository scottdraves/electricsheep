/*
	LOG.H
	Author: Stef.

	Implements singleton logging class.
*/
#ifndef	_LOG_H_
#define	_LOG_H_

#include	"boost/thread.hpp"
#include	"base.h"
#include	"SmartPtr.h"
#include	"Singleton.h"
#include	"LuaState.h"

#ifdef LINUX_GNU
#include <stdio.h>
#endif

namespace	Base
{

/*
	CLog.

*/
MakeSmartPointers( CLog );

class	CLog : public CSingleton<CLog>
{
	friend class CSingleton<CLog>;

	boost::mutex	m_Lock;

	static const uint32	m_MaxMessageLength = 4096;
	static char s_MessageSpam[ m_MaxMessageLength ];
	static char s_MessageType [ m_MaxMessageLength ];
	static size_t s_MessageSpamCount;

	//	Private constructor accessible only to CSingleton.
	CLog();

	//	No copy constructor or assignment operator.
    NO_CLASS_STANDARDS( CLog );

	bool	m_bActive;

	//	The luastate that will do all the actual work.
	Base::Script::CLuaState	*m_pState;

	FILE *m_pFile;
	//FILE *m_pStdout;

	//	Temporary storage vars.
	std::string	m_File;
	std::string m_Function;
	uint32		m_Line;

	void	Log( const char *_pType, /*const char *_file, const uint32 _line, const char *_pFunc,*/ const char *_pStr );

	public:
			bool	Startup();
			bool	Shutdown( void );
			virtual ~CLog();

			const char *Description()	{	return "Logger";	};

			void	Attach( const std::string &_location, const uint32 _level = 0 );
			void	Detach( void );

			void	SetInfo( const char *_pFileStr, const uint32 _line, const char *_pFunc );

			void	Debug( const char *_pFmt, ... );
			void	Info( const char *_pFmt, ... );
			void	Warning( const char *_pFmt, ... );
			void	Error( const char *_pFmt, ... );
			void	Fatal( const char *_pFmt, ... );

			//	Provides singleton access.
			static CLog *Instance( const char* /*_pFileStr*/, const uint32 /*_line*/, const char* /*_pFunc*/ )
			{
				static	CLog	log;

				if( log.SingletonActive() == false )
					printf( "Trying to access shutdown singleton %s\n", log.Description() );

				//	Annoying, smartpointer lock&unlock the mutex, then return to someone who will do the same...
				//log.SetInfo( _pFileStr, _line, _pFunc );
				return( &log );
			}
};

};

#define	g_Log	::Base::CLog::Instance( __FILE__, __LINE__, __FUNCTION__ )

#endif
