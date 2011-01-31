#include	<inttypes.h>
#include	<vector>
#include	<list>

#include	<boost/thread.hpp>
#include	<boost/bind.hpp>

#include	<curl/curl.h>
#include	<curl/types.h>
#include	<curl/easy.h>

#include	"ContentDownloader.h"
#include	"Shepherd.h"
#include	"Networking.h"
#include	"Voting.h"
#include	"Log.h"
#include	"Settings.h"
#include	"Player.h"
#if defined(WIN32) && defined(_MSC_VER)
#include "../msvc/msvc_fix.h"
#endif
/*
*/
CVote::CVote()
{
	m_Clock = 0;
	m_Timer.Reset();
	m_pThread = NULL;
	m_Votings.clear( 0 );
};

/*
*/
CVote::~CVote()
{
	if( m_pThread != NULL )
	{
	    m_pThread->interrupt();
		try
		{
			m_pThread->join();
		}
		catch (boost::thread_interrupted const &)
		{}
		SAFE_DELETE( m_pThread );
	}
};

/*
*/
void	CVote::ThreadFunc()
{
    try
    {
        VotingInfo vi;
		
		const char *pServerName = ContentDownloader::Shepherd::serverName();
		
		if ( pServerName == NULL )
			return;
			
		const char *pUniqueID = ContentDownloader::Shepherd::uniqueID();
		
		if ( pUniqueID == NULL )
			return;
		
		std::string serverName = pServerName;
		std::string uniqueID = pUniqueID;

		while( m_Votings.pop( vi, true ) )
		{
			static const char* votedesc[] = { "Negative", "Positive"	};

			g_Log->Info( "%s vote for sheep %d", votedesc[ vi.vtype ], vi.vid );

			Network::CCurlTransfer *spRequest = new Network::CCurlTransfer( std::string( votedesc[ vi.vtype ] ) + " vote" );

			char url[ MAX_PATH ];

			int voteDir = -1;
			if( vi.vtype == 1 )
				voteDir = 1;

			snprintf( url, MAX_PATH, "%scgi/vote.cgi?id=%d&vote=%d&u=%s", serverName.c_str(), vi.vid, voteDir, uniqueID.c_str() );

			//	Server responds (correctly) with a "302 Moved", so we need to allow that.
			spRequest->Allow( 302 );
			
			//If we were interrupted before this point, we don't need to start the unnecessary transfer.
			boost::this_thread::interruption_point();

			//	Send it...
			if( !spRequest->Perform( url ) )
			{
				g_Log->Error( "Failed to vote (%s).\n", url );

				if( spRequest->ResponseCode() == 401 )
					g_ContentDownloader().ServerFallback();
			}

			boost::this_thread::interruption_point();

			SAFE_DELETE( spRequest );
			g_Log->Info( "done voting..." );
		}
    }
    catch (boost::thread_interrupted const &)
    {
    }
}

/*
	Vote().
	Spawn a thread which sends a vote to the sheep server.
*/
bool	CVote::Vote( const int32 _id, const uint8 _type, const fp4 _duration )
{
	const fp8 time = m_Timer.Time();

	//	Check frequency.
	if( time - m_Clock > _duration )
	{
		if( _type == 0 && g_Settings()->Get( "settings.content.negvotedeletes", true ) )
		{
			//	Queue this sheep to be deleted.
			g_Player().Delete( _id );
		}

		m_Clock = time;
    }
	else
	{
		g_Log->Warning( "Voting is not allowed this frequently, be patient..." );
		return false;
	}

	VotingInfo vi;
	
	vi.vid = _id;
	vi.vtype = _type;

	m_Votings.push(vi);
	
	if (m_pThread == NULL)
	{
		//	Spawn the thread.
		m_pThread = new boost::thread( boost::bind( &CVote::ThreadFunc, this ) );
	}

	return true;
}
