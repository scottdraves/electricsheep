#include <math.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "Log.h"
#include "Networking.h"

namespace	Network
{

/*
	CCurlTransfer().
	Constructor.
*/
CCurlTransfer::CCurlTransfer( const std::string &_name ) : m_Name( _name ), m_Status( "Idle" ), m_AverageSpeed("0 kb/s")
{
	g_Log->Info( "CCurlTransfer(%s)", _name.c_str() );
	memset(errorBuffer, 0, CURL_ERROR_SIZE);
	m_pCurl = curl_easy_init();
	if( !m_pCurl )
		g_Log->Info( "Failed to init curl instance." );
		
	m_pCurlM = curl_multi_init();
	if( !m_pCurlM )
		g_Log->Info( "Failed to init curl multi instance." );
		
	if ( m_pCurl != NULL && m_pCurlM != NULL )
		curl_multi_add_handle( m_pCurlM, m_pCurl );
}

/*
	~CCurlTransfer().
	Destructor.
*/
CCurlTransfer::~CCurlTransfer()
{
	//g_NetworkManager->Remove( this );
	g_Log->Info( "~CCurlTransfer()" );
	
	if ( m_pCurlM != NULL && m_pCurl != NULL )
		curl_multi_remove_handle(m_pCurlM, m_pCurl);

	if( m_pCurl != NULL )
	{
		curl_easy_cleanup( m_pCurl );
		m_pCurl = NULL;
	}
	
	if ( m_pCurlM != NULL )
	{
		curl_multi_cleanup( m_pCurlM );
		m_pCurlM = NULL;
	}
}

/*
	Verify.
	User throughout these classes to verify curl integrity.
*/
bool CCurlTransfer::Verify( CURLcode _code )
{
	g_Log->Info( "Verify(%d)", _code );

	if( _code != CURLE_OK )
	{
		Status( curl_easy_strerror( _code ) );
		return false;
	}

	return true;
}

/*
	VerifyM.
	User throughout these classes to verify curl multi integrity.
*/
bool CCurlTransfer::VerifyM( CURLMcode _code )
{
	g_Log->Info( "VerifyM(%d)", _code );

	if( _code != CURLM_OK && _code != CURLM_CALL_MULTI_PERFORM)
	{
		Status( curl_multi_strerror( _code ) );
		return false;
	}

	return true;
}


/*
*/
int CCurlTransfer::customProgressCallback( void *_pUserData, fp8 _downTotal, fp8 _downNow, fp8 _upTotal, fp8 _upNow )
{
	//g_Log->Info( "customProgressCallback()" );

	if (!g_NetworkManager->SingletonActive() || g_NetworkManager->IsAborted())
		return -1;
	
	CCurlTransfer *pOut = static_cast<CCurlTransfer *>(_pUserData);
	if( !pOut )
	{
		g_Log->Info( "Error, no _pUserData" );
		return -1;
	}

	if (g_NetworkManager)
		g_NetworkManager->UpdateProgress( pOut, ((_downTotal+_upTotal) > 0) ? ((_downNow+_upNow) / (_downTotal+_upTotal) * 100) : 0, _downNow + _upNow );
	return 0;
}

/*
	Proxy().
	Set proxy info used during transfer.
*/
void	CManager::Proxy( const std::string &_url, const std::string &_userName, const std::string &_password )
{
	g_Log->Info( "Proxy()" );

	if( _url == "" )
		return;

	boost::mutex::scoped_lock locker( m_Lock );

    //  Set proxy url now, which will allow a non-user/pass proxy to be used, as reported on the forum.
	m_ProxyUrl = _url;

	if( _userName == "" || _password == "" )
		return;

	std::stringstream pu;
	pu << _userName << ":" << _password;
	m_ProxyUserPass = pu.str();
}

/*
	Login().
	Set authentication user/pass for transfer.	Default method is basic.
*/
void	CManager::Login( const std::string &_userName, const std::string &_password )
{
	g_Log->Info( "Login()" );

	if( _userName == "" || _password == "" )
		return;

	boost::mutex::scoped_lock locker( m_Lock );

	std::stringstream pu;
	pu << Encode( _userName ) << ":" << Encode( _password );
	m_UserPass = pu.str();
}

/*
	Logout().
	Clears authentication user/pass.
*/
void	CManager::Logout()
{
	g_Log->Info( "Logout()" );
	boost::mutex::scoped_lock locker( m_Lock );
	m_UserPass = "";
}

bool	CCurlTransfer::InterruptiblePerform()
{
	CURLMcode _code;
	int running_handles, running_handles_last;
	fd_set fd_read, fd_write, fd_except;
	int max_fd;
	long timeout;
	struct timeval tval;

	_code = curl_multi_perform( m_pCurlM, &running_handles );
	
	if ( !VerifyM(_code) )
		return false;
	
	if ( running_handles == 0 )
		return true;

	running_handles_last = running_handles;
	
	while( 1 )
	{
		while ( _code == CURLM_CALL_MULTI_PERFORM )
		{
			if (!g_NetworkManager->SingletonActive() || g_NetworkManager->IsAborted())
				return false;
			
			_code = curl_multi_perform (m_pCurlM, &running_handles );
		}

		if ( !VerifyM( _code ) )
			return false;

		if ( running_handles < running_handles_last )
			break;

		FD_ZERO( &fd_read );
		FD_ZERO( &fd_write );
		FD_ZERO( &fd_except );

		_code = curl_multi_fdset( m_pCurlM, &fd_read, &fd_write, &fd_except, &max_fd );
		
		if ( !VerifyM( _code ) )
			return false;
			
		timeout = -1;
		
#ifdef CURL_MULTI_TIMEOUT
		_code = curl_multi_timeout( m_pCurlM, &timeout );

		if ( !VerifyM( _code ) )
			return false;
#endif

		if (timeout == -1)
			timeout = 100;

		tval.tv_sec = timeout / 1000;
		tval.tv_usec = timeout % 1000 * 1000;

		int err;
		
		if (!g_NetworkManager->SingletonActive() || g_NetworkManager->IsAborted())
			return false;

		while ( ( err = select( max_fd + 1, &fd_read, &fd_write, &fd_except, &tval ) ) < 0 )
		{
#ifndef WIN32
			if ( err != EINTR )
			{
				return false;
			}
#endif
			if (!g_NetworkManager->SingletonActive() || g_NetworkManager->IsAborted())
				return false;
		}

		_code = CURLM_CALL_MULTI_PERFORM;
    }

	return true;
}


/*
	Perform().
	Do the actual transfer.
*/
bool	CCurlTransfer::Perform( const std::string &_url )
{
	if (!g_NetworkManager->SingletonActive() || g_NetworkManager->IsAborted())
		return false;

	std::string url = _url;

	g_Log->Info( "Perform(%s)", url.c_str() );
	if( !m_pCurl )
		return false;


	g_Log->Info( "0x%x", m_pCurl );

#ifdef	DEBUG
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_VERBOSE, 1 ) ) )	return false;
#endif

	//	Ask manager to prepare this transfer for us.
	if( !Verify( g_NetworkManager->Prepare( m_pCurl ) ) )	return false;

	g_Log->Info( "Performing '%s'", url.c_str() );
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_URL, url.c_str() ) ) )	return false;

	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_NOPROGRESS, 0 )	) )	return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_PROGRESSFUNCTION, &CCurlTransfer::customProgressCallback ) ) )	return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_PROGRESSDATA, this ) ) )	return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_ERRORBUFFER, errorBuffer ) ) )	return false;
	
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_FOLLOWLOCATION, 1 ) ) )	return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_MAXREDIRS, 5 ) ) )	return false;

	Status( "Active" );
	//if( !Verify( curl_easy_perform( m_pCurl ) ) )
	if ( !InterruptiblePerform() )
	{
		g_Log->Warning( errorBuffer );
		Status( "Failed" );
		return false;
	}
	else
		Status( "Completed" );

	//	Need to do this to trigger the strings to propagate.
	g_NetworkManager->UpdateProgress( this, 100, 0 );

	if( !Verify( curl_easy_getinfo( m_pCurl, CURLINFO_RESPONSE_CODE, &m_HttpCode ) ) ) return false;
	if( m_HttpCode != 200 )
	{
		//	Check if the response code is allowed.
		std::vector< uint32 >::const_iterator it = std::find( m_AllowedResponses.begin(), m_AllowedResponses.end(), m_HttpCode );
		if( it == m_AllowedResponses.end() )
		{
			switch ( m_HttpCode )
			{
				case 401:
					Status( "Authentication failed\n" );
					break;
					
				case 404:
					Status( "File not found on server\n" );
					break;
					
				default:
					{
						std::stringstream st;
						st << "Invalid server response [" << m_HttpCode << "]\n";
						
						Status( st.str() );
					}
					break;
			}
			
			//	Todo, (or not) print the remaining ones :)
			return false;
		}
	}

	fp8 speedUp = 0;
	fp8 speedDown = 0;
	if( !Verify( curl_easy_getinfo( m_pCurl, CURLINFO_SPEED_UPLOAD, &speedDown ) ) ) return false;
	if( !Verify( curl_easy_getinfo( m_pCurl, CURLINFO_SPEED_DOWNLOAD, &speedUp ) ) ) return false;

	std::stringstream statusres;
	statusres << "~" << (uint32)((speedUp+speedDown)/1000) << " kb/s";
	m_AverageSpeed = statusres.str();

	g_Log->Info( "Perform() complete" );

	return true;
}

/*
	CManager().
	Constructor.
*/
CManager::CManager()
{
}

/*
	Startup().
	Init network manager.
*/
bool	CManager::Startup()
{
	curl_global_init( CURL_GLOBAL_DEFAULT );

	m_UserPass = "";
	m_ProxyUrl = "";
	m_ProxyUserPass = "";
	
	m_Aborted = false;
	
	return true;
}

/*
	Startup().
	De-init network manager.
*/
bool	CManager::Shutdown()
{
	curl_global_cleanup();
	return true;
}

/*
	Prepare()-
	Called from CCurlTransfer::Perform().
	Sets proxy & http authentication.
*/
CURLcode	CManager::Prepare( CURL *_pCurl )
{
	g_Log->Info( "Prepare()" );

	boost::mutex::scoped_lock locker( m_Lock );

	CURLcode code = CURLE_OK;

	//	Set http authentication if there is one.
	if( m_UserPass != "" )
	{
		curl_easy_setopt(_pCurl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC); 

		code = curl_easy_setopt( _pCurl, CURLOPT_USERPWD, m_UserPass.c_str() );
	
		if( code != CURLE_OK )
			return code;
	}

	//	Set proxy url and user/pass if they're defined.
	if( m_ProxyUrl != "" )
	{
		code = curl_easy_setopt( _pCurl, CURLOPT_PROXY, m_ProxyUrl.c_str() );
		if( code != CURLE_OK )	return code;

		if( m_ProxyUserPass != "" )
			code = curl_easy_setopt( _pCurl, CURLOPT_PROXYUSERPWD, m_ProxyUserPass.c_str() );
	}

	return code;
}

/*
	Abort().
	Sets the abort flags then used by any transfer to abort itself.
*/
void	CManager::Abort( void )
{
	boost::mutex::scoped_lock locker( m_Lock );
	
	m_Aborted = true;
}

/*
	Abort().
	Sets the abort flags then used by any transfer to abort itself.
*/
bool	CManager::IsAborted( void )
{
	boost::mutex::scoped_lock locker( m_Lock );
	
	return m_Aborted;
}


/*
	UpdateProgress().
	Called from the callback of each CCurlTransfer instance to update progress.
*/
void	CManager::UpdateProgress( CCurlTransfer *_pTransfer, const fp8 _percentComplete, const fp8 _bytesTransferred )
{
	boost::mutex::scoped_lock locker( m_Lock );

	//g_Log->Info( "UpdateProgress()" );

	if( !_pTransfer )
		return;

	std::stringstream tmp;
	tmp << _pTransfer->Name() << " (" << _pTransfer->Status() << ")";
	if( _pTransfer->Status() == "Active" )
	{
		tmp << ": " << (int32)_percentComplete << "%";
		if (_bytesTransferred > 1024 * 1024)
			tmp << std::fixed << std::setprecision(1) << " (" << (_bytesTransferred/(1024.0 * 1024)) << " MB)";
		else if (_bytesTransferred > 1024)
			tmp << std::fixed << std::setprecision(0) << " (" << (_bytesTransferred/(1024.0)) << " kB)";
		else
			tmp << std::fixed << std::setprecision(0) << " (" << _bytesTransferred << " B)";
	}

	//g_Log->Info( "Setting progress!" );
	m_ProgressMap[ _pTransfer->Name() ] = tmp.str();
}

/*
	Status().
	Creates a string with status for all active transfers.
*/
std::string CManager::Status()
{
	boost::mutex::scoped_lock locker( m_Lock );

    std::string res = "";

	if( m_ProgressMap.size() == 0 )
		return res;

    std::map<std::string, std::string>::iterator iter;
	for( iter=m_ProgressMap.begin(); iter != m_ProgressMap.end(); )
	{
		std::string s = iter->second;

		std::string::size_type loc = s.find( "Completed", 0 );
		if( loc != std::string::npos )
		{
			std::map<std::string, std::string>::iterator next = iter;
			++next;
			m_ProgressMap.erase( iter );
			iter = next;
		}
		else
		{
			res += s + "\n";
			++iter;
		}
	}

	if (res.size() > 0)
		res.erase(res.size()-1);
	return res;
}

/*
	Remove().
	Removes transfer from progressmap.
*/
void	CManager::Remove( CCurlTransfer *_pTransfer )
{
	g_Log->Info( "Remove()" );
	boost::mutex::scoped_lock locker( m_Lock );
	m_ProgressMap.erase( _pTransfer->Name() );
}


/*
	Encode().
	Url encode a string, returns a new string.
*/
std::string CManager::Encode( const std::string &_src )
{
	g_Log->Info( "Encode()" );

	const uint8	dec2hex[ 16 + 1 ] = "0123456789ABCDEF";
	const uint8	*pSrc = (const uint8 *)_src.c_str();
	const size_t srcLen= _src.length();
	uint8 *const pStart = new uint8[ srcLen * 3 ];
	uint8 *pEnd = pStart;
	const uint8 * const srcEnd = pSrc + srcLen;

	for( ; pSrc<srcEnd; ++pSrc )
	{
		if( isalnum( *pSrc ) )
			*pEnd++ = *pSrc;
		else
		{
			//	Escape this char.
			*pEnd++ = '%';
			*pEnd++ = dec2hex[ *pSrc >> 4 ];
			*pEnd++ = dec2hex[ *pSrc & 0x0F ];
		}
	}

   std::string sResult( (char *)pStart, (char *)pEnd );
   delete [] pStart;
   return sResult;
}

};
