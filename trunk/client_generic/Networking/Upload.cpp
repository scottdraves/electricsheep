#include <stdio.h>
#include <string>
#include "Log.h"
#include "Networking.h"

namespace	Network
{

/*
	CFileUploader().
	Constructor.
*/
CFileUploader::CFileUploader( const std::string &_name ) : CCurlTransfer( _name )
{
}

/*
	~CFileUploader().
	Destructor.
*/
CFileUploader::~CFileUploader()
{
}

/*
	Perform().
	Upload specific Perform function.
*/
bool	CFileUploader::Perform( const std::string &_url, const std::string &_file, const uint32 _fileSize )
{
	//	Open input file to transfer.
	FILE *pFile = fopen( _file.c_str(), "rb" );
	if( !pFile )
	{
		g_Log->Info( "Failed to open %s", _file.c_str() );
		return false;
	}
	
	struct curl_slist *slist = NULL;
	slist = curl_slist_append(slist, "Expect:");
	if (slist != NULL)
		if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_HTTPHEADER, slist) ) ) return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_INFILE, pFile ) ) )	return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_INFILESIZE, _fileSize ) ) ) return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_READFUNCTION, fread ) ) )	return false;
	if( !Verify( curl_easy_setopt( m_pCurl, CURLOPT_UPLOAD, 1 ) ) ) return false;

	bool retval = CCurlTransfer::Perform( _url );
	if (pFile != NULL)
		fclose(pFile);
	
	if (slist != NULL)
	{
		curl_slist_free_all(slist);
		slist = NULL;
	}

	return retval;
}

};
