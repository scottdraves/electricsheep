#ifndef	_NETWORKING_H_
#define	_NETWORKING_H_

#include	<map>
#include	<vector>
#include	<curl/curl.h>
#include	<curl/easy.h>
#include	<curl/multi.h>
#include	<boost/thread.hpp>

#include	"base.h"
#include	"SmartPtr.h"
#include	"Singleton.h"

#ifdef LINUX_GNU
#undef Status
#endif

namespace	Network
{

/*
	CCurlTransfer.
	Baseclass for curl handled transfers.
*/
class	CCurlTransfer
{
	friend class CManager;


	//	Identifier;
	std::string	m_Name;
	std::string m_Status;
	std::string m_AverageSpeed;
	long		m_HttpCode;
    char errorBuffer[ CURL_ERROR_SIZE ];

	//	Map of respons codes allowed. This is checkd on failed perform's.
	std::vector< uint32 > m_AllowedResponses;

	protected:
		CURL		*m_pCurl;
		CURLM		*m_pCurlM;

		bool	Verify( CURLcode _code );
		bool	VerifyM( CURLMcode _code );
		void	Status( const std::string &_status )	{	m_Status = _status; };

	public:
		CCurlTransfer( const std::string &_name );
		virtual ~CCurlTransfer();

		static int32 customProgressCallback( void *_pUserData, fp8 _downTotal, fp8 _downNow, fp8 _upTotal, fp8 _upNow );

		virtual bool	InterruptiblePerform();
		
		virtual bool	Perform( const std::string &_url );

		//	Add a response code to the list of allowed ones.
		void	Allow( const uint32 _code )				{	m_AllowedResponses.push_back( _code );	}

		const std::string	&Name() const				{	return m_Name;			};
		const std::string	&Status() const				{	return m_Status;		};
		long 			ResponseCode() const		{	return m_HttpCode;		};
		const std::string	SpeedString() const			{	return m_AverageSpeed;	};
};

//
class	CFileDownloader : public CCurlTransfer
{
	std::string	m_Data;

	public:
		static int32 customWrite( void *_pBuffer, size_t _size, size_t _nmemb, void *_pUserData );

		CFileDownloader( const std::string &_name );
		virtual ~CFileDownloader();

		virtual bool	Perform( const std::string &_url );
		bool	Save( const std::string &_output );

		const std::string &Data()								{ return m_Data;			};
};

//
class	CFileDownloader_TimeCondition : public CFileDownloader
{
	public:
		CFileDownloader_TimeCondition( const std::string &_name );
		virtual ~CFileDownloader_TimeCondition();

		bool	PerformDownloadWithTC( const std::string &_url, const time_t _lastTime );
};

//
class	CFileUploader : public CCurlTransfer
{
	public:
		CFileUploader( const std::string &_name );
		virtual ~CFileUploader();

		bool	PerformUpload( const std::string &_url, const std::string &_file, const uint32 _filesize );
};

//	Def some smart pointers for these.
MakeSmartPointers( CCurlTransfer );
MakeSmartPointers( CFileDownloader );
MakeSmartPointers( CFileDownloader_TimeCondition );
MakeSmartPointers( CFileUploader );


/*
	CManager().
	The main manager.
*/
MakeSmartPointers( CManager );
class	CManager : public Base::CSingleton<CManager>
{
	friend class Base::CSingleton<CManager>;

	boost::mutex	m_Lock;

	//	Private constructor accessible only to CSingleton.
	CManager();

	//	No copy constructor or assignment operator.
    NO_CLASS_STANDARDS( CManager );

	//	To keep track of progress.
	std::map< std::string, std::string> m_ProgressMap;

	//	User/password for http authentication.
	std::string	m_UserPass;

	//	Proxy url and corresponding user/pass.
	std::string	m_ProxyUrl, m_ProxyUserPass;

	bool			m_Aborted;

	public:
			bool	Startup();
			bool	Shutdown();
			virtual ~CManager()	{m_bSingletonActive = false;};

			const char *Description()	{	return "Network manager";	};

			//	Called by CCurlTransfer destructors.
			void	Remove( CCurlTransfer *_pTransfer );

			//	Session wide proxy settings & user/pass.
			void	Proxy( const std::string &_url, const std::string &_userName, const std::string &_password );
			void	Login( const std::string &_userName, const std::string &_password );
			void	Logout();

			//	Called by CCurlTransfer prior to each Perform() call to handle proxy & authentication.
			CURLcode Prepare( CURL *_pCurl );

			//	Used by the transfers to update progress.
			void	UpdateProgress( CCurlTransfer *_pTransfer, const fp8 _percentComplete, const fp8 _bytesTransferred );

			// Used to abort any curl transfer

			void	Abort( void );
			bool	IsAborted( void );

			//	Fills in a vector of status strings for all active transfers.
			std::string Status();

			//	Urlencode string.
			static std::string Encode( const std::string &_src );

			//	Threadsafe.
			static CManager *Instance( const char * /*_pFileStr*/, const uint32 /*_line*/, const char * /*_pFunc*/ )
			{
				//printf( "g_NetworkManager( %s(%d): %s )\n", _pFileStr, _line, _pFunc );
				//fflush( stdout );

				static	CManager networkManager;

				if( networkManager.SingletonActive() == false )
				{
					printf( "Trying to access shutdown singleton %s\n", networkManager.Description() );
				}

				return( &networkManager );
			}
};

};

//	Helper for less typing...
#define	g_NetworkManager	Network::CManager::Instance( __FILE__, __LINE__, __FUNCTION__ )


#endif
