#ifdef	WIN32
#include	<windows.h>
#endif
#include 	<string>
#include 	<vector>
#include	<list>

#include	"base.h"
#include	"Log.h"
#include	"clientversion.h"
#include	"Settings.h"
#include	"Networking.h"

#include	"Shepherd.h"
#include	"ContentDownloader.h"
#include	"SheepDownloader.h"
#include	"SheepGenerator.h"
#ifdef MAC
#include <CoreServices/CoreServices.h>
#endif

namespace ContentDownloader
{

//	Generators.
std::vector<class SheepGenerator *> gGenerators;
std::vector<boost::thread *> gGeneratorThreads;

/*
*/
CContentDownloader::CContentDownloader()
{
}

//
static std::string generateID()
{
    uint8	*salt;
    uint32	u;
	char id[17];
	id[16] = 0;

#ifdef WIN32
    SYSTEMTIME syst;
    GetSystemTime(&syst);
    salt = ((unsigned char *)&syst) + sizeof(SYSTEMTIME) - 8;
#else
	timeval cur_time;
	gettimeofday(&cur_time, NULL);
	
	salt = (unsigned char*)&cur_time;
#endif

	for( u=0; u<16; u++ )
	{
		unsigned r = rand();
		r = r ^ (salt[u>>1] >> ((u&1)<<2));
		r &= 15;
		if( r < 10 )
			r += '0';
		else
			r += 'A' - 10;

		id[u] = r;
	}

	return 	std::string( id );
}

//
void	CContentDownloader::ServerFallback()
{
	Shepherd::setRegistered( false );
	g_NetworkManager->Logout();
}

/*
*/
const bool	CContentDownloader::Startup( const bool _bPreview, bool _bReadOnlyInstance )
{
	g_Log->Info( "Attempting to start contentdownloader...", _bPreview );
	Shepherd::initializeShepherd();

	Shepherd::setRedirectServerName( g_Settings()->Get( "settings.content.redirectserver", std::string(REDIRECT_SERVER) ).c_str() );
	
	std::string root = g_Settings()->Get( "settings.content.sheepdir", g_Settings()->Root() + "content" );
	
	if (root.empty())
	{
		root = g_Settings()->Root() + "content";
		g_Settings()->Set( "settings.content.sheepdir", root );
	}
	
	Shepherd::setRootPath( root.c_str() );
	Shepherd::setCacheSize( g_Settings()->Get( "settings.content.cache_size", 2000 ), 0 );
	Shepherd::setCacheSize( g_Settings()->Get( "settings.content.cache_size_gold", 2000 ), 1 );
	if (g_Settings()->Get( "settings.content.unlimited_cache", false) == true)
		Shepherd::setCacheSize( 0, 0 );
	if (g_Settings()->Get( "settings.content.unlimited_cache_gold", false) == true)
		Shepherd::setCacheSize( 0, 1 );
	Shepherd::setPassword( g_Settings()->Get( "settings.content.password_md5", std::string("") ).c_str() );
	Shepherd::setUniqueID( g_Settings()->Get( "settings.content.unique_id", generateID() ).c_str() );
	Shepherd::setUseProxy( g_Settings()->Get( "settings.content.use_proxy", false ) );
	Shepherd::setRegistered( g_Settings()->Get( "settings.content.registered", false ) );
	Shepherd::setProxy( g_Settings()->Get( "settings.content.proxy", std::string("") ).c_str() );
	Shepherd::setProxyUserName( g_Settings()->Get( "settings.content.proxy_username", std::string("") ).c_str() );
	Shepherd::setProxyPassword( g_Settings()->Get( "settings.content.proxy_password", std::string("") ).c_str() );

	Shepherd::setSaveFrames( g_Settings()->Get( "settings.generator.save_frames", false ) );
	SheepGenerator::setNickName( g_Settings()->Get( "settings.generator.nickname", std::string("") ).c_str() );
	SheepGenerator::setURL( g_Settings()->Get( "settings.generator.user_url", std::string("") ).c_str() );

    m_gDownloader = NULL;
		
	if( g_Settings()->Get( "settings.content.download_mode", true ) && _bReadOnlyInstance == false)
	{
		m_gDownloader = new SheepDownloader();
		g_Log->Info( "Starting download thread..." );
		m_gDownloadThread = new boost::thread( boost::bind( &SheepDownloader::shepherdCallback, m_gDownloader ) );
#ifdef WIN32
		SetThreadPriority( (HANDLE)m_gDownloadThread->native_handle(), THREAD_PRIORITY_BELOW_NORMAL );
		SetThreadPriorityBoost( (HANDLE)m_gDownloadThread->native_handle(), TRUE );
#else
		struct sched_param sp;
		int esnRetVal = 0;
		sp.sched_priority = 6; //Background NORMAL_PRIORITY_CLASS - THREAD_PRIORITY_BELOW_NORMAL
		esnRetVal = pthread_setschedparam( (pthread_t)m_gDownloadThread->native_handle(), SCHED_RR, &sp );
#endif
	}
	else
		g_Log->Warning( "Downloading disabled." );

	if( g_Settings()->Get( "settings.generator.enabled", true ) && _bReadOnlyInstance == false)
	{
		//	Create the generators based on the number of processors.
		uint32 ncpus = 1;
		if( g_Settings()->Get( "settings.generator.all_cores", false ) )
		{
#ifdef WIN32
			SYSTEM_INFO sysInfo;
			GetSystemInfo( &sysInfo );
			ncpus = (uint32)sysInfo.dwNumberOfProcessors;
#else
	#ifdef MAC
			ncpus = MPProcessors();
        #else
#ifdef LINUX_GNU
			ncpus = sysconf( _SC_NPROCESSORS_ONLN );
#endif
	#endif
#endif
		}

		if (ncpus > 1)
			--ncpus;
		uint32 i;
		for( i=0; i<ncpus; i++ )
		{
			g_Log->Info( "Starting generator for core %d...", i );
			gGenerators.push_back( new SheepGenerator() );
			gGenerators[i]->setGeneratorId( i );
		}

		g_Log->Info( "Starting generator threads..." );

		for( i=0; i<gGenerators.size(); i++ )
		{
			gGeneratorThreads.push_back( new boost::thread( boost::bind( &SheepGenerator::shepherdCallback, gGenerators[i] ) ) );
#ifdef WIN32
			SetThreadPriority( (HANDLE)gGeneratorThreads[i]->native_handle(), THREAD_PRIORITY_IDLE );
			SetThreadPriorityBoost( (HANDLE)gGeneratorThreads[i]->native_handle(), FALSE );
#else
			struct sched_param sp;
			sp.sched_priority = 1; //THREAD_PRIORITY_IDLE - THREAD_PRIORITY_IDLE
			pthread_setschedparam( (pthread_t)gGeneratorThreads[i]->native_handle(), SCHED_RR, &sp );
#endif
		}
	}
	else
		g_Log->Warning( "Generators disabled..." );

	g_Log->Info( "...success" );

	return true;
}

/*
*/
const bool	CContentDownloader::Shutdown( void )
{
	//	Terminate the threads.
    g_Log->Info( "Terminating download thread." );
	
	g_NetworkManager->Abort();
	
	if( m_gDownloadThread  && m_gDownloader )
	{
		m_gDownloader->Abort();
		m_gDownloadThread->interrupt();
		
		m_gDownloadThread->join();
		
		SAFE_DELETE( m_gDownloadThread );
	}
	
	SAFE_DELETE( m_gDownloader );
	
	for( unsigned int i=0; i<gGeneratorThreads.size(); i++ )
	{
		//give to everybody time to abort
		gGenerators[i]->Abort();
		gGeneratorThreads[i]->interrupt();
	}

	for( unsigned int i=0; i<gGeneratorThreads.size(); i++ )
	{
		

		gGeneratorThreads[i]->join();

		SAFE_DELETE( gGeneratorThreads[i] );
		SAFE_DELETE( gGenerators[i] );
	}
	
	gGenerators.clear();
	gGeneratorThreads.clear();

	//	Notify the shepherd that the app is about to close so that he can properly clean up his threads
	Shepherd::notifyShepherdOfHisUntimleyDeath();

	return true;
}

/*
*/
CContentDownloader::~CContentDownloader()
{
	//	Mark singleton as properly shutdown, to track unwanted access after this point.
	SingletonActive( false );
}

};
