///////////////////////////////////////////////////////////////////////////////
//
//    electricsheep for windows - collaborative screensaver
//    Copyright 2003 Nicholas Long <nlong@cox.net>
//	  electricsheep for windows is based of software
//	  written by Scott Draves <source@electricsheep.org>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _SHEPHERD_H_
#define _SHEPHERD_H_

#include	"base.h"
#include	"SmartPtr.h"
#include	<queue>
#include	"boost/thread/mutex.hpp"
#include	"boost/detail/atomic_count.hpp"
#include	"Timer.h"
#include	"Sheep.h"
#include	"Log.h"
#include	"BlockingQueue.h"

#ifdef WIN32
#define PATH_SEPARATOR_C '\\'
#else
#define PATH_SEPARATOR_C '/'
#endif

namespace ContentDownloader
{

class	CMessageBody
{
	public:
			CMessageBody( const std::string &_str, const fp8 _duration ) : m_Msg( _str ), m_Duration(_duration)	{};
			~CMessageBody()	{};

			std::string m_Msg;
			fp8	m_Duration;
};

MakeSmartPointers( CMessageBody );

class	CTimedMessageBody
{
	public:
			CTimedMessageBody( const std::string &_str, const fp8 _duration ) : m_Msg( _str ), m_Duration(_duration)	{ m_Timer.Reset(); };
			~CTimedMessageBody()	{};

			bool TimedOut()
			{
				if (m_Timer.Time() > m_Duration)
					return true;

				return false;
			}
			Base::CTimer m_Timer;
			std::string m_Msg;
			fp8	m_Duration;
};

MakeSmartPointers( CTimedMessageBody );


/*
	Shepherd.
	This class is responsible for managing the threads to create, render, and download sheep.
*/
class Shepherd
{
	typedef struct _SHEPHERD_MESSAGE
	{
		char	*text;
		int		length;
		time_t	expire;
	}	SHEPHERD_MESSAGE;

	//	Gets all sheep in path.
	static bool getSheep( const char *path, SheepArray *sheep );

	static uint64 s_ClientFlockBytes;
	static uint64 s_ClientFlockCount;

	static uint64 s_ClientFlockGoldBytes;
	static uint64 s_ClientFlockGoldCount;

	static char *fRootPath;
	static char *fMpegPath;
	static char *fXmlPath;
	static char *fJpegPath;
	static char *fRedirectServerName;
	static char *fServerName;
	static char *fProxy;
	static char *fProxyUser;
	static char *fProxyPass;
	static int	fSaveFrames;
	static int	fUseProxy;
	static int	fCacheSize;
	static int	fCacheSizeGold;
	static int	fFuseLen;
	static int	fRegistered;
	static char *fPassword;
	static char *fUniqueID;
	static bool fShutdown;
	static int fChangeRes;
	static int fChangingRes;
	static char *s_Role;
	static boost::detail::atomic_count	*renderingFrames;
	static boost::detail::atomic_count	*totalRenderedFrames;
	static bool m_RenderingAllowed;


	static std::queue<spCMessageBody>	m_MessageQueue;
	static boost::mutex	m_MessageQueueMutex;

	static std::vector<spCTimedMessageBody>	m_OverflowMessageQueue;
	static boost::mutex s_OverflowMessageQueueMutex;

	static boost::mutex	s_ShepherdMutex;
	
	static boost::mutex	s_DownloadStateMutex;

	static boost::mutex	s_RenderStateMutex;

	static boost::mutex	s_ComputeServerNameMutex;
	
	static boost::mutex	s_GetServerNameMutex;
	
	static boost::mutex s_RoleMutex;
	

	static time_t s_LastRequestTime;

	static Base::CBlockingQueue<char *> fStringsToDelete;
	
	static std::string s_DownloadState;

	static std::string s_RenderState;
	
	static bool s_IsDownloadStateNew;
	
	static bool s_IsRenderStateNew;

	public:
			Shepherd();
			~Shepherd();

			static void initializeShepherd();

			//
			static void	setUseProxy( const int &useProxy )			{	fUseProxy = useProxy;	}
			static int	useProxy()									{	return fUseProxy;	}

			static void setProxy(const char *proxy);
			static const char *proxy();
			static void setProxyUserName( const char *userName );
			static const char *proxyUserName();
			static void setProxyPassword( const char *password );
			static const char *proxyPassword();

			//
			static void	setSaveFrames(const int &saveFrames)		{	fSaveFrames = saveFrames;	}
			static int		saveFrames()							{	return fSaveFrames;	}

			//
			static void	setRegistered( const int &registered )		{	fRegistered = registered;	}
			static int		registered()							{	return fRegistered;	}

			//
			static int		cacheSize( const int getGenerationType )	
			{	
				switch (getGenerationType)
				{
				case 0 :
					return fCacheSize;	
					break;
				case 1 :
					return fCacheSizeGold;	
					break;
				default:
					g_Log->Error("Getting cache size for unknown generation type %d", getGenerationType);
					return 0;
				};
			}
			static void	setCacheSize( const int &size, const int getGenerationType )
			{

				/*if( (size < 300) && (size > 0) )	fCacheSize = 300;
				else*/	
				switch (getGenerationType)
				{
				case 0 :
					fCacheSize = size;
					break;
				case 1 :
					fCacheSizeGold = size;
					break;
				default:
					g_Log->Error("Setting cache size for unknown generation type %d", getGenerationType);
				};
			}

			//
			static void setChangeRes(const int &changeRes)		{	fChangeRes = changeRes;	}
			static int changeRes()								{	return fChangeRes;	}
			static void setChangingRes(int state)				{	fChangingRes = state;	}
			static void incChangingRes()						{	fChangingRes++;	}
			static int getChangingRes()							{	return fChangingRes;	}

			//
			static void notifyShepherdOfHisUntimleyDeath();
			static void setRootPath( const char *path );
			static const char *rootPath();
			static const char *mpegPath();
			static const char *xmlPath();
			static const char *jpegPath();

			static void setRole( const char *role );
			static const char *role();

			//	Gets/sets the host name of the server to use.
			static void setRedirectServerName( const char *server );
			static const char *serverName( bool allowServerQuery = true );

			//	Gets/sets the registration password.
			static void setPassword( const char *password );
			static const char *password();

			//computes MD5 hash
			static std::string computeMD5( const std::string& str );

			//	Overlay text management for the renderer.
			static void addMessageText( const char *s, size_t len, time_t timeout );


			//	Called from generators.
			static void FrameStarted();
			static void	FrameCompleted();
			static int	FramesRendering();
			static int	TotalFramesRendered();
			static bool	RenderingAllowed();
			static void SetRenderingAllowed(bool _yesno);

			static const bool	AddOverflowMessage( const std::string _msg )
			{
				boost::mutex::scoped_lock lockthis( s_OverflowMessageQueueMutex );
				m_OverflowMessageQueue.push_back( new CTimedMessageBody( _msg, 60. ) );
				return true;
			}

			static const bool	PopOverflowMessage( std::string &_dst )
			{
				boost::mutex::scoped_lock lockthis( s_OverflowMessageQueueMutex );
				if( m_OverflowMessageQueue.size() > 10 )
				{
					m_OverflowMessageQueue.erase(m_OverflowMessageQueue.begin(), m_OverflowMessageQueue.begin() + m_OverflowMessageQueue.size() - 10);
				}
				if( m_OverflowMessageQueue.size() > 0 )
				{
					bool del = false;
					size_t deletestart = 0;
					size_t deleteend = 0;
					for (size_t ii = 0; ii != m_OverflowMessageQueue.size(); ++ii)
					{
						spCTimedMessageBody msg = m_OverflowMessageQueue[ii];
						_dst += msg->m_Msg;
						if (msg->TimedOut())
						{
							if (del == false)
							{
								deletestart = ii;
								deleteend = ii;
							} 
							else
							{
								deleteend = ii;
							}
							del = true;
						}
						_dst += "\n";
					}
					_dst.erase(_dst.find_last_of("\n"));
					
					if (del == true)
					{
						m_OverflowMessageQueue.erase(m_OverflowMessageQueue.begin() + deletestart, m_OverflowMessageQueue.begin() + deleteend + 1);
					}

					return true;
				}
				return false;
			}

			static const bool	QueueMessage( const std::string _msg, const fp8 _duration )
			{
				boost::mutex::scoped_lock lockthis( m_MessageQueueMutex );
				m_MessageQueue.push( new CMessageBody( _msg, _duration ) );
				return true;
			}

			static const bool	PopMessage( std::string &_dst, fp8 &_duration )
			{
				boost::mutex::scoped_lock lockthis( m_MessageQueueMutex );
				if( m_MessageQueue.size() > 0 )
				{
					spCMessageBody msg = m_MessageQueue.front();
					_dst = msg->m_Msg;
					_duration = msg->m_Duration;
					m_MessageQueue.pop();
					return true;
				}
				return false;
			}


			//	Methods to check for filename extensions.
			static int filenameIsMpg(const char *name);
			static int filenameIsXxx(const char *name);
			static int filenameIsTmp(const char *name);

			//	Method to get all of the sheep the exist on the client.
			static bool getClientFlock(SheepArray *sheep);
			static uint64 GetFlockSizeMBsRecount(const int generationtype);

			//	Sets/Gets the unique id for this Shepherd.
			static void	setUniqueID( const char *uniqueID );
			static const char	*uniqueID();
			
			static void setDownloadState( const std::string& state );
			static std::string downloadState( bool& isnew );
	
			static void setRenderState( const std::string& state );
			static std::string renderState( bool& isnew );

			static void subClientFlockBytes(uint64 removedbytes, const int generationtype)
			{
				if ( generationtype == 0 )
					s_ClientFlockBytes -= removedbytes;
				if ( generationtype == 1 )
					s_ClientFlockGoldBytes -= removedbytes;
			}
			static void subClientFlockCount(const int generationtype)
			{
				if ( generationtype == 0 )
					--s_ClientFlockCount;
				if ( generationtype == 1 )
					--s_ClientFlockGoldCount;
			}

			static uint64 getClientFlockMBs(const int generationtype)
			{
				if ( generationtype == 0 )
					return s_ClientFlockBytes/1024/1024;
				if ( generationtype == 1 )
					return s_ClientFlockGoldBytes/1024/1024;
				return 0;
			}
			static uint64 getClientFlockCount(const int generationtype)
			{
				if ( generationtype == 0 )
					return s_ClientFlockCount;
				if ( generationtype == 1 )
					return s_ClientFlockGoldCount;
				return 0;
			}
};

};
#endif
