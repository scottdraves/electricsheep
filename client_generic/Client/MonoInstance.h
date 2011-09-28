#ifndef _MonoInstance_H_
#define _MonoInstance_H_

#include <windows.h>
#include <Tlhelp32.h>
#include <psapi.h>

/*
	CMonoInstance().
	Make sure that you use a name that is unique for this application otherwise
	two apps may think they are the same if they are using same name for 3rd parm to CreateMutex
*/
class CMonoInstance
{
	protected:
		DWORD  m_dwLastError;
		HANDLE m_hMutex;

	public:
			bool WaitForParentPID()
			{
				PROCESSENTRY32 procentry;
				procentry.dwSize = sizeof(PROCESSENTRY32);
				HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0 );
				if (hSnapShot != INVALID_HANDLE_VALUE)
				{
					BOOL	bContinue = Process32First( hSnapShot, &procentry );
					DWORD pid = 0;

					while( bContinue == TRUE)
					{
						if	(GetCurrentProcessId() == procentry.th32ProcessID)
						{
							pid =  procentry.th32ParentProcessID;
							break;
						}
						
						procentry.dwSize = sizeof(PROCESSENTRY32) ;
						bContinue = Process32Next( hSnapShot, &procentry );
					}
					
					CloseHandle(hSnapShot);
					if (pid != 0)
					{
						HANDLE hProcess = OpenProcess( SYNCHRONIZE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
							FALSE, procentry.th32ParentProcessID ) ;

						if (hProcess != NULL)
						{
							HMODULE hMod[512];
							char szFileName[MAX_PATH];
							DWORD dwSize = 0;
							if( EnumProcessModules( hProcess, hMod, sizeof( hMod ), &dwSize ) != 0 )
							{
								if( GetModuleFileNameExA( hProcess, hMod[0], szFileName, sizeof( szFileName ) ) != 0 )
								{

									std::string extension = szFileName;
									//OutputDebugStringA(szFileName);
									//if (extension.find("es.scr") != extension.npos && extension.find("es.exe") != extension.npos)
									{
										if (WaitForSingleObject(hProcess, 5000) != WAIT_OBJECT_0)
										{
											CloseHandle (hProcess);
											return false;
										}
									} 
									CloseHandle (hProcess);
									return true;
								}
							}
							CloseHandle (hProcess);
						} 
					} else
					{
						return false;
					}
				}
				return false;
			}

			CMonoInstance( char *strMutexName )
			{
				m_hMutex = CreateMutexA( NULL, FALSE, strMutexName ); 	//	Do early.
				if (m_hMutex != NULL)
				{
					//OutputDebugStringA("CMonoInstance creating mutex 1st time");
					m_dwLastError = GetLastError();	 //	Save for use later...
					if (ERROR_ALREADY_EXISTS == m_dwLastError) // give some time for snapshot
					{
						CloseHandle( m_hMutex );
						m_hMutex = NULL;
						//OutputDebugStringA("WaitForParentPID() ERROR_ALREADY_EXISTS");
						if (WaitForParentPID() == false)
						{
							Sleep(1000);
							WaitForParentPID();
							//OutputDebugStringA("WaitForParentPID() 2");
						}
						//OutputDebugStringA("CMonoInstance creating mutex 2nd time");
						m_hMutex = CreateMutexA( NULL, FALSE, strMutexName );
						m_dwLastError = GetLastError();
					}
				} else
				{
					//OutputDebugStringA("assuming m_dwLastError = ERROR_ALREADY_EXISTS");
					m_dwLastError = ERROR_ALREADY_EXISTS;
				}
			}

			~CMonoInstance()
			{
				if( m_hMutex != NULL ) 	//	Do not forget to close handles.
				{
					//OutputDebugStringA("~CMonoInstance()");
					CloseHandle( m_hMutex );	//	Do as late as possible.
					m_hMutex = NULL;			//	Good habit to be in.
				}
			}

			BOOL IsAnotherInstanceRunning()
			{
				return( ERROR_ALREADY_EXISTS == m_dwLastError || ERROR_ACCESS_DENIED == m_dwLastError);
			}
};

#endif
