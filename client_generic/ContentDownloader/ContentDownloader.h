#ifndef	_CONTENT_DOWNLOADER_H_
#define	_CONTENT_DOWNLOADER_H_

#include	"boost/thread/thread.hpp"
#include	"base.h"
#include	"SmartPtr.h"
#include	"Singleton.h"

#ifndef	MAX_PATH
#define	MAX_PATH 4096
#endif


namespace	ContentDownloader
{

/**
	CContentDownloader.
	Singleton class to handle downloading of sheep.
*/
class	CContentDownloader : public Base::CSingleton<CContentDownloader>
{
	friend class Base::CSingleton<CContentDownloader>;

	//	Private constructor accessible only to CSingleton.
	CContentDownloader();

	//	No copy constructor or assignment operator.
	NO_CLASS_STANDARDS( CContentDownloader );

	//	Downloader.
	class SheepDownloader	*m_gDownloader;
	boost::thread			*m_gDownloadThread;

	public:
			bool	Startup( const bool _bPreview, bool _bReadOnlyInstance = false );
			bool	Shutdown( void );

			const char *Description()	{	return "Content Downloader";	};

			std::string ServerMessages();

			//	Called if network said unauthorized, will fallback everything to unregistered server.
			void	ServerFallback();

			virtual ~CContentDownloader();
};

};

/*
	Helper for less typing...

*/
inline ContentDownloader::CContentDownloader &g_ContentDownloader( void )	{	return( ContentDownloader::CContentDownloader::Instance() );	}

#endif
