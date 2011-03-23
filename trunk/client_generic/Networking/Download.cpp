#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Log.h"
#include "Networking.h"

namespace	Network
{

/*
	CFileDownloader().
	Constructor.
*/
CFileDownloader::CFileDownloader( const std::string &_name ) : CCurlTransfer( _name ), m_Data()
{
	m_Data.reserve(10*1024*1024);
}

/*
	~CFileDownloader().
	Destructor.
*/
CFileDownloader::~CFileDownloader()
{
}

/*
	customWrite().
	Appends incoming data to string.
*/
int32 CFileDownloader::customWrite( void *_pBuffer, size_t _size, size_t _nmemb, void *_pUserData )
{
	CFileDownloader *pOut = (CFileDownloader *)_pUserData;
	if( !pOut )
	{
		g_Log->Info( "Error, no _pUserData." );
		return -1;
	}

	pOut->m_Data.append( (char *)_pBuffer, _size * _nmemb );
	return (int32)(_size * _nmemb);
}


/*
	Perform().
	Download specific Perform function.
*/
bool	CFileDownloader::Perform( const std::string &_url )
{
	m_Data = "";

	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_WRITEDATA, this ) ) )	return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_WRITEFUNCTION, &CFileDownloader::customWrite ) ) )	return false;

	return CCurlTransfer::Perform( _url );
}

/*
	Save().
	Saves completed data to file.
*/
bool	CFileDownloader::Save( const std::string &_output )
{
	std::ofstream out( _output.c_str(), std::ios::out | std::ios::binary );
	if( out.bad() )
	{
		g_Log->Info( "Failed to open output file." );
		return false;
	}

	out << m_Data;
	out.close();

	g_Log->Info( "%s saved.", _output.c_str() );
	return true;
}


/*
	CFileDownloader_TimeCondition().
	Constructor.
*/
CFileDownloader_TimeCondition::CFileDownloader_TimeCondition( const std::string &_name ) : CFileDownloader( _name )
{
}

/*
	~CFileDownloader_TimeCondition
	Destructor.
*/
CFileDownloader_TimeCondition::~CFileDownloader_TimeCondition()
{
}

/*
	Perform().

*/
bool CFileDownloader_TimeCondition::Perform( const std::string &_url, const time_t _lastTime )
{
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_TIMECONDITION, this ) ) )	return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_TIMEVALUE, _lastTime ) ) )	return false;
	return CFileDownloader::Perform( _url );
}

};
