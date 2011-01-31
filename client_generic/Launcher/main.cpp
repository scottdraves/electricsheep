#ifndef LINUX_GNU
#include <windows.h>
#endif
#include <string>
#include <stdio.h>

//	Grab a string from the registry.
HRESULT RegGetString( HKEY hKey, LPCTSTR szValueName, LPTSTR * lpszResult )
{
	#define	MAXBUF	4096

	//	Given a HKEY and value name returns a string from the registry.
	//	Upon successful return the string should be freed using free().
	//	eg. RegGetString(hKey, TEXT("my value"), &szString);

	DWORD dwType = 0, dwDataSize = 0, dwBufSize = 0;
	LONG lResult;

	//	In case we fail set the return string to null...
	if( lpszResult != NULL )
		*lpszResult = NULL;

	//	Check input parameters...
	if( hKey == NULL || lpszResult == NULL )
		return E_INVALIDARG;

	//	Get the length of the string in bytes (placed in dwDataSize)...
	lResult = RegQueryValueEx(hKey, szValueName, 0, &dwType, NULL, &dwDataSize );

	//	Check result and make sure the registry value is a string(REG_SZ)...
	if( lResult != ERROR_SUCCESS )
		return HRESULT_FROM_WIN32( lResult );
	else if( dwType != REG_SZ )
		return DISP_E_TYPEMISMATCH;

	//	Allocate memory for string - We add space for a null terminating character...
	dwBufSize = dwDataSize + (1 * sizeof(TCHAR) );
	*lpszResult = (TCHAR *)malloc( dwBufSize );

	if( *lpszResult == NULL )
		return E_OUTOFMEMORY;

	//	Now get the actual string from the registry...
	lResult = RegQueryValueEx( hKey, szValueName, 0, &dwType, (LPBYTE)*lpszResult, &dwDataSize );

	//	Check result and type again. If we fail here we must free the memory we allocated...
	if( lResult != ERROR_SUCCESS )
	{
		free( *lpszResult );
		return HRESULT_FROM_WIN32( lResult );
	}
	else if( dwType != REG_SZ )
	{
		free( *lpszResult );
		return DISP_E_TYPEMISMATCH;
	}

	//	We are not guaranteed a null terminated string from RegQueryValueEx so explicitly null terminate the returned string...
	(*lpszResult)[ (dwBufSize / sizeof(TCHAR)) - 1 ] = TEXT( '\0' );

	return NOERROR;
}


//	Get the application installation directory from the registry. (stored by the installer).
std::string	InstallationDirectory()
{
	std::string appdir = ".\\";

	HKEY key;
	if( !RegOpenKey( HKEY_LOCAL_MACHINE, "SOFTWARE\\ElectricSheep", &key ) )
	{
		LPTSTR temp;
		if( RegGetString( key, TEXT("InstallDir"), &temp ) == NOERROR )
		{
			appdir = temp;
			free( temp );
		}
		else
			printf( "Unable to fetch SOFTWARE\\ElectricSheep\\InstallDir" );
	}
	else
		printf( "Unable to fetch SOFTWARE\\ElectricSheep" );


	size_t len = appdir.size();
	if( appdir[ len - 1 ] != '\\' )
		appdir.append( "\\" );

	return appdir;
}

//	Remove exename from params.
char *ParseCmdLine()
{
    char	*lpszCommandLine = GetCommandLine();
	printf( "Cmdline %s\n", lpszCommandLine );

    //	Skip past program name (first token in command line).
    if( *lpszCommandLine == '"' )
    {
        //	Scan, and skip over, subsequent characters until  another double-quote or a null is encountered.
        while( *lpszCommandLine && (*lpszCommandLine != '"') )
            lpszCommandLine++;

        //	If we stopped on a double-quote (usual case), skip over it.
        if( *lpszCommandLine == '"' )
            lpszCommandLine++;
    }
    else
    {
        //	First token wasn't a quote.
        while ( *lpszCommandLine > ' ' )
            lpszCommandLine++;
    }

    //	Skip past any white space preceeding the second token.
    while ( *lpszCommandLine && (*lpszCommandLine <= ' ') )
        lpszCommandLine++;

	return lpszCommandLine;
}

//
HWND	g_ClientWin;
int CALLBACK EnumWindowsProc( HWND hwnd, LPARAM param )
{
	DWORD pID;
	DWORD TpID = GetWindowThreadProcessId( hwnd, &pID );
	if( TpID == (DWORD)param )
	{
		g_ClientWin = hwnd;
		return false;
	}
	return true;
}

//
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    FILE *stream ;
	if( (stream = freopen( "c:\\es_launcher.log", "w", stdout ) ) == NULL )
		exit(-1);

	time_t	curTime;
	char	timeStamp[32] = { 0 };

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	std::string appdir = InstallationDirectory();
	std::string starter = appdir + "Client.exe";

	char *c = ParseCmdLine();
	char cmdLine[ MAX_PATH ];
	_snprintf( cmdLine, MAX_PATH, " %s", c );

	bool	bConfig = false;
	if( *c == 0 )
		bConfig = true;
	else
	{
		if( *c=='-' || *c=='/' )
			c++;

		if( *c=='c' || *c=='C' )
			bConfig = true;
	}

	if( bConfig == true )
	{
		starter = appdir + "wlua.exe";
		_snprintf( cmdLine, MAX_PATH, " %s", "./Scripts/wx_config.lua" );
	}

	time( &curTime );	strftime( timeStamp, sizeof(timeStamp), "%H:%M:%S", localtime( &curTime ) );
	printf( "[%s] Launching %s %s\n", timeStamp, starter.c_str(), cmdLine );
	fflush( stdout );

    // Start the child process.
    if( !CreateProcess( starter.c_str(),
        cmdLine,						// Command line
        NULL,           				// Process handle not inheritable
        NULL,           				// Thread handle not inheritable
        FALSE,          				// Set handle inheritance to FALSE
        IDLE_PRIORITY_CLASS,			// No creation flags
        NULL,           				// Use parent's environment block
        appdir.c_str(),           		// Use parent's starting directory
        &si,            				// Pointer to STARTUPINFO structure
        &pi )           				// Pointer to PROCESS_INFORMATION structure
    )
    {
        printf( "CreateProcess failed (%d)\n", GetLastError() );
		fflush( stdout );
		fclose( stream );
        return 0;
    }

	time( &curTime );	strftime( timeStamp, sizeof(timeStamp), "%H:%M:%S", localtime( &curTime ) );
	printf( "[%s] WaitForInputIdle...\n", timeStamp );
	WaitForInputIdle( pi.hProcess, INFINITE );

	time( &curTime );	strftime( timeStamp, sizeof(timeStamp), "%H:%M:%S", localtime( &curTime ) );
	printf( "[%s] EnumWindows...\n", timeStamp );
	EnumWindows( &EnumWindowsProc, pi.dwThreadId );
	//SetParent( g_ClientWin, GetParent( NULL ) );

	time( &curTime );	strftime( timeStamp, sizeof(timeStamp), "%H:%M:%S", localtime( &curTime ) );
	printf( "[%s] WaitForSingleObject...\n", timeStamp );
    //	Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

	time( &curTime );	strftime( timeStamp, sizeof(timeStamp), "%H:%M:%S", localtime( &curTime ) );
	printf( "[%s] Done...\n", timeStamp );

    //	Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

	fclose( stream );

    return 0;
}
