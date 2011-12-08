#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#ifdef	WIN32
#include	<windows.h>
#endif

#include	"base.h"
#include	"SmartPtr.h"
#include	"Exception.h"
#include	"Log.h"
#include	"Timer.h"
#include	<sstream>

#ifdef LINUX_GNU
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace	Base
{

/*
	CProcessForker().
	Forks a process with optional env-vars.

	todo; remove non std::string stuff, just messy..
*/
#ifdef WIN32
static void RecreateProcess(std::string parameters)
{
	STARTUPINFOA si = {0};
	si.cb = sizeof(STARTUPINFOA);
	PROCESS_INFORMATION pi = {0};
	char temp[MAX_PATH] = {0};
	char winfolder[MAX_PATH] = {0};
	GetWindowsDirectoryA(winfolder, sizeof(winfolder));
	strcpy(temp,std::string(winfolder + std::string("\\es.scr ") + parameters).c_str());
	if (CreateProcessA(0, temp, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) != 0)
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
}

class	CProcessForker
{
	static const uint32 m_BUFSIZE = 8192;

	//	Safety.
	static const uint32 m_NAMESIZE = 32;
	static const uint32 m_VALUESIZE = 256;

	bool	m_bTerminate;
	bool	m_bWaiting;
	HANDLE	m_hProcess;

	const char 	*m_pAppName;
	const char	*m_pAppDir;

	char	*m_pEnv;
	char	*m_pParams;
	LPSTR	m_pCurrentVar;
	LPSTR	m_pCurrentParam;

	public:
			CProcessForker( const char *_pProcessName, bool _bTerminate = true, const char *_wd = NULL ) :
				m_bWaiting(false), m_pAppName( _pProcessName ), m_bTerminate( _bTerminate ), m_pAppDir(_wd)
			{
				m_pEnv = NULL;
				m_pParams = NULL;
				m_pCurrentVar = NULL;
				m_pCurrentParam = NULL;
				m_hProcess = INVALID_HANDLE_VALUE;
			}

			~CProcessForker()
			{
				if( m_bTerminate )
				{
					Terminate();
				}

				SAFE_DELETE( m_pEnv );
			}

			bool	PushEnv( const std::string &_name, const std::string &_value )
			{
				printf( "PushEnv( %s, %s )\n", _name.c_str() , _value.c_str() );

				if( !m_pEnv )
				{
					m_pEnv = new char[ m_BUFSIZE ];
					m_pCurrentVar = (LPSTR)m_pEnv;
				}

				if( _name.size() > m_NAMESIZE )
				{
					g_Log->Error( "Oversized env name: %s", _name.c_str() );
					//ThrowArgs(( "Oversized env name: %s", _name.c_str() ));
				}

				if( _value.size() > m_VALUESIZE )
				{
					g_Log->Error( "Oversized env value: %s", _value.c_str() );
					//ThrowArgs(( "Oversized env value: %s", _value.c_str() ));
				}

				std::stringstream stm;
				stm << _name << "=" << _value;

				if( strcpy( m_pCurrentVar, stm.str().c_str() ) == NULL )
					return false;

				//	Offset please.
				m_pCurrentVar += lstrlenA( m_pCurrentVar ) + 1;
				return true;
			}

			bool	PushParam( const std::string &_param )
			{
				printf( "PushParam( %s )\n", _param.c_str() );

				if( !m_pParams )
				{
					m_pParams = new char[ m_BUFSIZE ];
					m_pCurrentParam = (LPSTR)m_pParams;
				}

				std::stringstream stm;
				stm << " " << _param;

				if( strcpy( m_pCurrentParam, stm.str().c_str() ) == NULL )
					return false;

				//	Offset please.
				m_pCurrentParam += lstrlenA( m_pCurrentParam );
				return true;
			}

			void	Execute()
			{
				//	Make sure envblock is terminated.
				if( m_pEnv )
					*m_pCurrentVar = (CHAR)0;

				//	Create the child process, specifying a new environment block.
				STARTUPINFOA	si;
				ZeroMemory( &si, sizeof(STARTUPINFOA) );
				si.cb = sizeof(STARTUPINFOA);

				DWORD	dwFlags = CREATE_NO_WINDOW | IDLE_PRIORITY_CLASS;

				PROCESS_INFORMATION	pi;

				char cmdLine[ m_BUFSIZE ] = {0};

				if( m_pParams )
				{
					strncpy( cmdLine, m_pParams, MAX_PATH ); //m_pParams starts by " "...
					g_Log->Info( "Starting %s%s...", m_pAppName, cmdLine );
				}
				else
					g_Log->Info( "Starting %s...", m_pAppName );

				if( !CreateProcessA( m_pAppName, cmdLine, NULL, NULL, TRUE, dwFlags, (LPVOID)m_pEnv, (CHAR *)m_pAppDir, &si, &pi ) )
				{
					CHAR	msg[ MAX_PATH + 1 ];
					int32	err = GetLastError();

					FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), msg, MAX_PATH, 0 );
					g_Log->Error( "Process failed with: %s", msg );
				}
				else
				{
					CloseHandle( pi.hThread );
					m_hProcess = pi.hProcess;
				}
			}

			/*void	ExecuteCred()
			{
				DWORD dwErr = CredUIPromptForCredentials(
                                         &cui,
                                         _T("Tool.exe"),
                                         NULL,
                                         0,
                                         szName,
                                         CREDUI_MAX_USERNAME_LENGTH+1,
                                         szPwd,
                                         CREDUI_MAX_PASSWORD_LENGTH+1,
                                         &fSave,
                                         CREDUI_FLAGS_USERNAME_TARGET_CREDENTIALS |
                                         CREDUI_FLAGS_REQUEST_ADMINISTRATOR |
                                         CREDUI_FLAGS_EXPECT_CONFIRMATION
                                         );
				//if (!dwErr)
				{
					TCHAR	szUserName[ CREDUI_MAX_USERNAME_LENGTH + 1 ];
					TCHAR szDomainName[ CREDUI_MAX_DOMAIN_TARGET_LENGTH  + 1 ];

					DWORD dwError = CredUIParseUserName( szName, szUserName,
					CREDUI_MAX_USERNAME_LENGTH,
					szDomainName,
					CREDUI_MAX_DOMAIN_TARGET_LENGTH);

					std::wstring strCommandLine(L"Tool.exe");

					STARTUPINFO	strctStartInfo;
					ZeroMemory(&strctStartInfo, sizeof(STARTUPINFO));
					strctStartInfo.cb			= sizeof(STARTUPINFO);

					PROCESS_INFORMATION	strctProcInfo;
					ZeroMemory(&strctProcInfo, sizeof(PROCESS_INFORMATION));

					CreateProcessWithLogonW(
					szUserName,
					szDomainName,
					szPwd,
					LOGON_WITH_PROFILE,
					NULL,
					&strCommandLine[0],
					0,
					NULL,
					NULL,
					&strctStartInfo,
					&strctProcInfo));

					CloseHandle(strctProcInfo.hThread);
					CloseHandle(strctProcInfo.hProcess);

					SecureZeroMemory(pszName, sizeof(pszName));
					SecureZeroMemory(pszPwd, sizeof(pszPwd));
				}
			}*/


			//
			int32	Wait()
			{
				m_bWaiting = true;
				bool bExitWhile = false;
				DWORD prc = 0;

				while( !bExitWhile )
				{
					DWORD rc = 0;
					if (m_hProcess == INVALID_HANDLE_VALUE)
						rc = WAIT_FAILED;
					else
						rc = MsgWaitForMultipleObjects( 1, &m_hProcess, FALSE, INFINITE, QS_ALLINPUT );
					switch( rc )
					{
						case WAIT_FAILED:
							g_Log->Error( "Failed waiting on process handle" );
							bExitWhile = true;
						break;
						case WAIT_OBJECT_0:
							//g_Log->Info( "WAIT_OBJECT_0..." );
							bExitWhile = true;
							GetExitCodeProcess( m_hProcess, &prc );
							CloseHandle( m_hProcess );
							m_hProcess = INVALID_HANDLE_VALUE;
							break;

						case WAIT_OBJECT_0 + 1:
							HWND hwnd = NULL;
							MSG msg;

							//g_Log->Info( "WAIT_OBJECT_0+1..." );

							//	Remove any messages that may be in the queue. If the queue contains any mouse or keyboard messages, end the operation.
							while( PeekMessage( &msg, hwnd,  0, 0, PM_REMOVE ) )
							{
								TranslateMessage( &msg );
								DispatchMessage( &msg );
							}
							break;
					}

					//	Sleep for a quarter of a second.
					Base::CTimer::Wait( 0.25 );
				}

				g_Log->Info( "Fork done..." );
				m_bWaiting = false;
				return prc;
			}

			void Terminate( void )
			{
				if( m_hProcess != INVALID_HANDLE_VALUE )
				{
					g_Log->Info( "Terminating process..." );
					TerminateProcess( m_hProcess, 0 );
					if( !m_bWaiting )
					{
						g_Log->Info( "Closing handle..." );
						CloseHandle( m_hProcess );
						m_hProcess = INVALID_HANDLE_VALUE;
					}
				}
			}
};

#else
#include	<signal.h>

class	CProcessForker
{
	static const uint32 m_BUFSIZE = 8192;

	//	Safety.
	static const uint32 m_NAMESIZE = 32;
	static const uint32 m_VALUESIZE = 256;

	static const uint32 m_MaxEnvCount = 256;
	static const uint32 m_MaxParamCount = 20;

	bool	m_bTerminate;
	bool	m_bWaiting;
	int		m_ChildPID;

	const char 	*m_pAppName;
	const char	*m_pAppDir;

	char	*m_pEnv[m_MaxEnvCount + 1];
	char	*m_pParam[m_MaxParamCount + 1];
	uint32	m_EnvSize;
	uint32  m_ParamSize;

	public:
			CProcessForker( const char *_pProcessName, bool _bTerminate = true, const char *_wd = NULL ) :
				m_bWaiting(false), m_pAppName( _pProcessName ), m_bTerminate( _bTerminate ), m_pAppDir(_wd)
			{
				m_pEnv[0] = NULL;
				m_EnvSize = 0;
				
				char *procName = NULL;
				uint32 paramSize = 0;
				
				if ( _pProcessName != NULL )
				{
					procName = new char[strlen(_pProcessName) + 1];
					strcpy( procName, _pProcessName );
					paramSize = 1;
				}
				
				m_pParam[0] = procName;
				m_pParam[1] = NULL;
				m_ParamSize = paramSize;

				m_ChildPID = -1;
			}

			~CProcessForker()
			{
				if ( m_bTerminate)
				{
					Terminate();
				}

				for (uint32 i=0; i < m_EnvSize; i++)
				{
					SAFE_DELETE_ARRAY(m_pEnv[i]);
				}

				for (uint32 i=0; i < m_ParamSize; i++)
				{
					SAFE_DELETE_ARRAY(m_pParam[i]);
				}
			}

			bool	PushEnv( const std::string &_name, const std::string &_value )
			{
				printf( "PushEnv( %s, %s )\n", _name.c_str() , _value.c_str() );

				if (m_EnvSize >= m_MaxEnvCount)
					return false;

				if( _name.size() > m_NAMESIZE )
				{
					g_Log->Error( "Oversized env name: %s", _name.c_str() );
					//ThrowArgs(( "Oversized env name: %s", _name.c_str() ));
				}

				if( _value.size() > m_VALUESIZE )
				{
					g_Log->Error( "Oversized env value: %s", _value.c_str() );
					//ThrowArgs(( "Oversized env value: %s", _value.c_str() ));
				}

				std::stringstream stm;
				stm << _name << "=" << _value;

				char *env = new char[stm.str().size() + 1];

				if( strcpy( env, stm.str().c_str() ) == NULL )
					return false;

				m_pEnv[m_EnvSize++] = env;
				m_pEnv[m_EnvSize] = NULL;

				return true;
			}

			bool	PushParam( const std::string &_param )
			{
				printf( "PushParam( %s )\n", _param.c_str() );

				if (m_ParamSize >= m_MaxParamCount)
					return false;

				char *param = new char[_param.size() + 1];

				if( strcpy( param, _param.c_str() ) == NULL )
					return false;

				m_pParam[m_ParamSize++] = param;
				m_pParam[m_ParamSize] = NULL;

				return true;
			}


			void	Execute()
			{
				int pid;
				if ( ( pid = fork() ) == 0 )
				{
#ifdef MAC
					setpriority(PRIO_PROCESS, 0, PRIO_MAX);
#else
					setpriority(PRIO_PROCESS, 0, PRIO_MAX);
#endif
					execve( m_pAppName, m_pParam, m_pEnv );
				}
				else
				{
					m_ChildPID = pid;

/*#ifdef MAC
					setpriority(PRIO_PROCESS, pid, 19);
#else
					setpriority(PRIO_PROCESS, pid, 19);
#endif*/
				}

			}

			//
			int32	Wait()
			{
				if ( m_ChildPID == -1)
					return 0;

				int stat_loc;

				waitpid( m_ChildPID, &stat_loc, 0 );

				m_ChildPID = -1;

				return WEXITSTATUS(stat_loc);
			}

			void Terminate( void )
			{
				g_Log->Info( "Terminating process..." );

				if ( m_ChildPID != -1 )
				{
					kill( m_ChildPID, SIGTERM );
					m_ChildPID = -1;
				}

			}
};
#endif

MakeSmartPointers( CProcessForker );

};

#endif // PROCESS_H_INCLUDED
