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
#include	<boost/filesystem.hpp>
#include	<string>
#include	<sys/stat.h>
#include	"ContentDecoder.h"
#include	"Playlist.h"
#include	"Log.h"
#include	"Timer.h"
#include	"Settings.h"

using namespace boost;

namespace ContentDecoder
{

/*
	CContentDecoder.

*/
CContentDecoder::CContentDecoder( spCPlaylist _spPlaylist, bool _bStartByRandom, bool _bCalculateTransitions, const uint32 _queueLenght, AVPixelFormat _wantedFormat )
{
	g_Log->Info( "CContentDecoder()" );
	m_FadeCount = static_cast<uint32>(g_Settings()->Get("settings.player.fadecount", 30));
	//	We want errors!
	av_log_set_level( AV_LOG_ERROR );

    m_pScaler = NULL;
    m_ScalerWidth = 0;
    m_ScalerHeight = 0;
    
	m_bStartByRandom = _bStartByRandom;
	
	m_bCalculateTransitions = _bCalculateTransitions;

	m_pDecoderThread = NULL;
	
	m_FrameQueue.setMaxQueueElements(_queueLenght);
	
	m_NextSheepQueue.setMaxQueueElements(10);

	m_WantedPixelFormat = _wantedFormat;

	m_bStop = true;

	m_spPlaylist = _spPlaylist;
	
	m_bForceNext = false;
		
	m_sharedFrame = NULL;
	
	m_Initialized = false;
	m_NoSheeps = true;

	m_FadeIn = m_FadeCount;
	m_FadeOut = 0;
	m_prevLast = 0;
	
	m_MainVideoInfo = NULL;//new sMainVideoInfo();
	m_SecondVideoInfo = NULL;
}

/*
*/
CContentDecoder::~CContentDecoder()
{
}

/*
*/
void	CContentDecoder::Destroy()
{
	g_Log->Info( "Destroy()" );
	
	if (m_MainVideoInfo != NULL)
	{
		SAFE_DELETE(m_MainVideoInfo);
	}
	
	if (m_SecondVideoInfo != NULL)
	{
		SAFE_DELETE(m_SecondVideoInfo);
	}
    
    if( m_pScaler )
    {
        g_Log->Info( "deleting m_pScalar" );
        sws_freeContext( m_pScaler );
        m_pScaler = NULL;
    }
}

/*
	DumpError().

*/
int	CContentDecoder::DumpError( int _err )
{
	if( _err < 0 )
	{
		switch( _err )
		{
			case AVERROR_INVALIDDATA:	g_Log->Error( "Error while parsing header" );	break;
			case AVERROR(EIO):			g_Log->Error( "I/O error occured. Usually that means that input file is truncated and/or corrupted." );	break;
			case AVERROR(ENOMEM):		g_Log->Error( "Memory allocation error occured" );	break;
			case AVERROR(ENOENT):		/*g_Log->Error( "No such file or directory" );  legal, will be warned in Open()*/	break;
			default:					g_Log->Error( "Error while opening file" );	break;
		}
	}

    return _err;
}

/*
*/
bool	CContentDecoder::Open( sOpenVideoInfo *ovi )
{
	if (ovi == NULL)
		return false;
	
	boost::filesystem::path sys_name( ovi->m_Path );

	const std::string &_filename = sys_name.string();
	
	ovi->m_iCurrentFileFrameCount = 0;
	ovi->m_totalFrameCount = 0;
	struct stat fs;
	if ( !::stat( _filename.c_str(), &fs ) )
	{
		ovi->m_CurrentFileatime = fs.st_atime;
	}
	g_Log->Info( "Opening: %s", _filename.c_str() );

	if( DumpError( avformat_open_input( &ovi->m_pFormatContext, _filename.c_str(), NULL, NULL ) ) < 0 )
	{
		g_Log->Warning( "Failed to open %s...", _filename.c_str() );
		return false;
	}

	if( DumpError( avformat_find_stream_info( ovi->m_pFormatContext, NULL ) ) < 0 )
	{
		g_Log->Error( "av_find_stream_info failed with %s...", _filename.c_str() );
		return false;
	}

	//	Find video stream;
	ovi->m_VideoStreamID = -1;
    for( uint32 i=0; i<ovi->m_pFormatContext->nb_streams; i++ )
    {
        if( ovi->m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
        {
            ovi->m_pVideoStream = ovi->m_pFormatContext->streams[i];
            ovi->m_VideoStreamID = static_cast<int32>(i);
            break;
        }
    }

    if( ovi->m_VideoStreamID == -1 )
    {
        g_Log->Error( "Could not find video stream in %s", _filename.c_str() );
        return false;
    }

	//	Find video codec.
    ovi->m_pVideoCodecContext = ovi->m_pFormatContext->streams[ ovi->m_VideoStreamID ]->codec;
    if( ovi->m_pVideoCodecContext == NULL )
    {
        g_Log->Error( "Video CodecContext not found for %s", _filename.c_str() );
        return false;
    }

    ovi->m_pVideoCodec = avcodec_find_decoder( ovi->m_pVideoCodecContext->codec_id );

    if( ovi->m_pVideoCodec == NULL )
    {
        ovi->m_pVideoCodecContext = NULL;
        g_Log->Error( "Video Codec not found for %s", _filename.c_str() );
        return false;
    }

    ovi->m_pFormatContext->flags |= AVFMT_FLAG_IGNIDX;		//	Ignore index.

    if( DumpError( avcodec_open2( ovi->m_pVideoCodecContext, ovi->m_pVideoCodec, NULL ) ) < 0 )
    {
        g_Log->Error( "avcodec_open failed for %s", _filename.c_str() );
        return false;
    }
	
    ovi->m_pFrame = av_frame_alloc();
	
	if (ovi->m_pVideoStream->nb_frames > 0)
		ovi->m_totalFrameCount = static_cast<uint32>(ovi->m_pVideoStream->nb_frames);
	else
		ovi->m_totalFrameCount = uint32(((((double)ovi->m_pFormatContext->duration/(double)AV_TIME_BASE)) / av_q2d(ovi->m_pVideoStream->avg_frame_rate) + .5));
		
	ovi->m_ReadingTrailingFrames = false;

	g_Log->Info( "Open done()" );

    return true;
}

/*
*/
void	CContentDecoder::Close()
{
	g_Log->Info( "Closing..." );

	Stop();

	ClearQueue();

	Destroy();

	m_spPlaylist = NULL;

	g_Log->Info( "closed..." );

}

/*
*/
/*void	CContentDecoder::Size( uint32 &_width, uint32 &_height )
{
	if( m_pVideoCodecContext[ m_CurrentLayer ] )
	{
		_width = (uint32)m_pVideoCodecContext[ m_CurrentLayer ]->width;
		_height = (uint32)m_pVideoCodecContext[ m_CurrentLayer ]->height;
	}
}*/

sOpenVideoInfo*	CContentDecoder::GetNextSheepInfo()
{
	std::string name;

	sOpenVideoInfo *retOVI = NULL;
	
	bool sheepfound = false;
	
	while ( !sheepfound && m_NextSheepQueue.pop(name, true) )
	{
		if ( name.empty() )
			break;

		uint32 Generation, ID, First, Last;
		std::string fname;
											
		sheepfound = true;
		
		retOVI = new sOpenVideoInfo;
		
		retOVI->m_Path.assign(name);
			
		if( m_spPlaylist->GetSheepInfoFromPath( name, Generation, ID, First, Last, fname ) )
		{				
			boost::filesystem::path p( name );
			
			if ( !boost::filesystem::exists( p ) )
			{
				sheepfound = false;
				continue;
			}
			
			std::string xxxname( name );
			xxxname.replace(xxxname.size() - 3, 3, "xxx");
			
			if ( boost::filesystem::exists( p/xxxname ) )
			{
				sheepfound = false;
				continue;
			}

			retOVI->m_SheepID = ID;
			retOVI->m_Generation = Generation;
			retOVI->m_First = First;
			retOVI->m_Last = Last;
			retOVI->m_bSpecialSheep = false;
		}
		else
		{
			retOVI->m_bSpecialSheep = true;			
		}
	}
	
	return retOVI;
}

/*
	Next().
	Advance to next playlist entry.
*/
bool	CContentDecoder::NextSheepForPlaying( int32 _forceNext )
{

	if( m_spPlaylist == NULL )
	{
		g_Log->Warning("Playlist == NULL");
		return( false );
	}
	
	if (_forceNext != 0 )
    {
        std::string secondPath("");
        
        if (m_SecondVideoInfo != NULL && m_SecondVideoInfo->m_NumIterations == 0)
           secondPath.assign(m_SecondVideoInfo->m_Path);
        
        SAFE_DELETE(m_SecondVideoInfo);
        
        if (_forceNext < 0)
        {
            uint32 _numPrevious = (uint32)(-_forceNext);
            
            if ( m_SheepHistoryQueue.size() > 0 )
            {
                if (!secondPath.empty())
                    m_NextSheepQueue.push( secondPath, false, false );
                
                std::string name;
                
                for ( uint32 i = 0; i < _numPrevious; i++ )
                {
                    while ( m_SheepHistoryQueue.pop( name, false, false ) )
                    {
                        m_NextSheepQueue.push( name, false, false );
                        
                        break;
                    }
                }
            }
        }
        else
        {
            if (!secondPath.empty())
                m_NextSheepQueue.push(secondPath, false, false);
        }
    }
				
	SAFE_DELETE(m_MainVideoInfo);
	
	m_MainVideoInfo = m_SecondVideoInfo;
	
	m_SecondVideoInfo = NULL;
	
	if (m_MainVideoInfo == NULL)
	{
		m_MainVideoInfo = GetNextSheepInfo();
		
		if (m_MainVideoInfo == NULL)
			return false;
	}
	
	if (!m_MainVideoInfo->m_bSpecialSheep)
	{
		if (m_SecondVideoInfo == NULL)
		{
			if (m_MainVideoInfo->IsLoop() && m_LoopIterations > 0 && m_MainVideoInfo->m_NumIterations < (m_LoopIterations - 1))
			{
				m_SecondVideoInfo = new sOpenVideoInfo(m_MainVideoInfo);
				m_SecondVideoInfo->m_NumIterations++;
			}
			else
				m_SecondVideoInfo = GetNextSheepInfo();

		}
	}
	else
		m_NoSheeps = true;
	
	if (!m_MainVideoInfo->IsOpen())
	{
		if (!Open( m_MainVideoInfo ))
			return false;
	}
	else
	{
		//if the video was already open (m_SecondVideoInfo previously),
		//we need to assure seamless continuation
		m_MainVideoInfo->m_NextIsSeam = true;
	}
	
	if (m_MainVideoInfo->IsOpen())
	{
		if (m_MainVideoInfo->m_NumIterations == 0)
		{
			while( m_SheepHistoryQueue.size() > 50 )
			{
				std::string tmpstr;
				
				m_SheepHistoryQueue.pop( tmpstr );
			}
			
			m_SheepHistoryQueue.push( m_MainVideoInfo->m_Path );
			
			m_spPlaylist->ChooseSheepForPlaying( m_MainVideoInfo->m_Generation, m_MainVideoInfo->m_SheepID );
		}
		
		m_NoSheeps = false;
	}
	else
		return false;
		
	if (m_bCalculateTransitions && m_SecondVideoInfo != NULL && !m_SecondVideoInfo->IsOpen() && m_MainVideoInfo->m_Last != m_SecondVideoInfo->m_SheepID && m_MainVideoInfo->m_SheepID != m_SecondVideoInfo->m_First && m_MainVideoInfo->m_Last != m_SecondVideoInfo->m_First && (m_MainVideoInfo->m_Generation / 10000) == (m_SecondVideoInfo->m_Generation / 10000))
	{
		Open( m_SecondVideoInfo );
	}

	return true;
}

/*
	CalculateNextSheep().
	Thread function.
*/
void	CContentDecoder::CalculateNextSheep()
{
	try {
	
		uint32 _curID = 0;
		
		bool bRebuild = true;
		
		while (!m_bStop)
		{
			this_thread::interruption_point();
			
			if ( m_spPlaylist == NULL )
			{
				thread::sleep( get_system_time() + posix_time::milliseconds(100) );
				continue;
			}
			
			std::string _spath;
			bool _enoughSheep = true;
			
			if( m_spPlaylist->Next( _spath, _enoughSheep, _curID, bRebuild, m_bStartByRandom ) )
			{
				bRebuild = false;
				
				uint32 Generation, ID, First, Last;
				std::string fname;
				
				if ( m_spPlaylist->GetSheepInfoFromPath( _spath, Generation, ID, First, Last, fname ) )
				{
					boost::filesystem::path p( _spath );
				
					if ( !bRebuild && !boost::filesystem::exists( _spath ) )
					{
						bRebuild = true;
						continue;
					}

					_curID = ID;
				}
				
				if ( !_enoughSheep )
				{
					while( !m_NextSheepQueue.empty() )
					{
						if ( !m_NextSheepQueue.waitForEmpty() )
						{
							break;
						}
					}
				}
				
				m_NextSheepQueue.push( _spath );
			}
			else
				bRebuild = true;
			
			m_Initialized = true;
			thread::sleep( get_system_time() + posix_time::milliseconds(100) );
			//this_thread::yield();
		}
	}
	catch(thread_interrupted const&)
	{
	}
}

CVideoFrame *CContentDecoder::ReadOneFrame(sOpenVideoInfo *ovi)
{
	if (ovi == NULL)
		return NULL;
					
	AVFormatContext	*pFormatContext = ovi->m_pFormatContext;

	if( !pFormatContext )
        return NULL;

    AVPacket packet;
    int	frameDecoded = 0;
	AVFrame *pFrame = ovi->m_pFrame;
    AVCodecContext	*pVideoCodecContext = ovi->m_pVideoCodecContext;
	CVideoFrame *pVideoFrame = NULL;

	while(true)
    {
		av_init_packet(&packet);
		
		packet.data = NULL;
		packet.size = 0;
        
		if (!ovi->m_ReadingTrailingFrames)
		{
			if ( av_read_frame( pFormatContext, &packet ) < 0 )
			{
				ovi->m_ReadingTrailingFrames = true;
				av_free_packet(&packet);
				continue;
			}
		}
		
        if( packet.stream_index != ovi->m_VideoStreamID )
        {
            g_Log->Error("Mismatching stream ID");
			break;
		}
		

#if (!defined(LINUX_GNU) || defined(HAVE_AVC_VID2))
        int32 bytesDecoded = avcodec_decode_video2( pVideoCodecContext, pFrame, &frameDecoded, &packet );
#else
        int32 bytesDecoded = avcodec_decode_video( pVideoCodecContext, pFrame, &frameDecoded, packet.data, packet.size );
#endif
                        
        if ( bytesDecoded < 0 )
		{
            g_Log->Warning( "Failed to decode video frame: bytesDecoded < 0" );
			break;
		}

		//at the beginning of each file we can get few frames with frameDecoded==0 (multi-thread delay)
		//all frames will be delayed and the few remaining frames come at the end when ovi->m_ReadingTrailingFrames == true
		//only when frameDecoded == 0 and ovi->m_ReadingTrailingFrames == true, we are finally done with the file.
		if ( frameDecoded != 0 || ovi->m_ReadingTrailingFrames )
        {
            break;
        }
        
		av_free_packet(&packet);
    }

    //	Do we have a fresh frame?
    if( frameDecoded != 0 )
    {
        //	If the decoded video has a different resolution, delete the scaler to trigger it to be recreated.
        if( m_ScalerWidth != (uint32)pVideoCodecContext->width || m_ScalerHeight != (uint32)pVideoCodecContext->height )
        {
            g_Log->Info( "size doesn't match, recreating" );

            if( m_pScaler )
            {
                g_Log->Info( "deleting m_pScalar" );
                sws_freeContext( m_pScaler );
                m_pScaler = NULL;
            }
        }

        //	Make sure scaler is created.
        if( m_pScaler == NULL )
        {
            g_Log->Info( "creating m_pScaler" );

            m_pScaler = sws_getContext(	pVideoCodecContext->width, pVideoCodecContext->height, pVideoCodecContext->pix_fmt,
                                            pVideoCodecContext->width, pVideoCodecContext->height, m_WantedPixelFormat, SWS_BICUBIC, NULL, NULL, NULL );

            //	Store width & height now...
            m_ScalerWidth = static_cast<uint32>(pVideoCodecContext->width);
            m_ScalerHeight = (uint32)pVideoCodecContext->height;

            if( m_pScaler == NULL )
                g_Log->Warning( "scaler == null" );
        }

        pVideoFrame = new CVideoFrame( pVideoCodecContext, m_WantedPixelFormat, std::string(pFormatContext->filename) );
        AVFrame	*pDest = pVideoFrame->Frame();

        sws_scale( m_pScaler, pFrame->data, pFrame->linesize, 0, pVideoCodecContext->height, pDest->data, pDest->linesize );
        
        if ( pVideoCodecContext->refcounted_frames )
            av_frame_unref( pFrame );
        
		ovi->m_iCurrentFileFrameCount++;

        pVideoFrame->SetMetaData_SheepID( ovi->m_SheepID );
        pVideoFrame->SetMetaData_SheepGeneration( ovi->m_Generation );
        pVideoFrame->SetMetaData_IsEdge( ovi->IsEdge() );
        pVideoFrame->SetMetaData_atime( ovi->m_CurrentFileatime );
        pVideoFrame->SetMetaData_IsSeam( ovi->m_NextIsSeam );
        pVideoFrame->SetMetaData_FrameIdx( ovi->m_iCurrentFileFrameCount );
        pVideoFrame->SetMetaData_MaxFrameIdx( ovi->m_totalFrameCount );
        ovi->m_NextIsSeam = false;
    }

    av_free_packet( &packet );
    return pVideoFrame;
}

/*
	ReadPackets().
	Thread function.
*/
void	CContentDecoder::ReadPackets()
{	
	try {
	
		g_Log->Info( "Packet thread started..." );
		
		if( !NextSheepForPlaying() )
			return;

		while( true )
		{			
			this_thread::interruption_point();

            int32 nextForced = NextForced();
			
			if (nextForced != 0)
				ForceNext( 0 );
				
			bool bDoNextSheep = true;
				
			if (nextForced == 0)
			{
				CVideoFrame *pMainVideoFrame = ReadOneFrame(m_MainVideoInfo);
				
				if (pMainVideoFrame != NULL)
				{
					CVideoFrame *pSecondVideoFrame = NULL;
					
#define kTransitionFrameLength	60
				
					if (m_SecondVideoInfo != NULL && m_SecondVideoInfo->IsOpen() && m_MainVideoInfo->m_iCurrentFileFrameCount >= (m_MainVideoInfo->m_totalFrameCount - kTransitionFrameLength))
						pSecondVideoFrame = ReadOneFrame(m_SecondVideoInfo);
					
					if (pSecondVideoFrame != NULL)
					{
						pMainVideoFrame->SetMetaData_SecondFrame(pSecondVideoFrame);
						 
						if (m_SecondVideoInfo->m_iCurrentFileFrameCount < kTransitionFrameLength)
							pMainVideoFrame->SetMetaData_TransitionProgress((fp4)m_SecondVideoInfo->m_iCurrentFileFrameCount * 100.f / ((fp4)kTransitionFrameLength - 1.f));
						else 
							pMainVideoFrame->SetMetaData_TransitionProgress(100.f);
					}
					else
						pMainVideoFrame->SetMetaData_TransitionProgress(0.f);
					
					m_FrameQueue.push( pMainVideoFrame );
					
					bDoNextSheep = false;
					
				}
			}
			
			if (bDoNextSheep)
			{					
				g_Log->Info( "calling Next()" );
				
				NextSheepForPlaying( nextForced );
				
				if ( nextForced != 0 )
					ClearQueue();
			}
		}

	}
	catch(thread_interrupted const&)
	{
	}
	
	printf( "decoder thread ending..." );

	g_Log->Info( "Ending decoder thread..." );
}

void CContentDecoder::ResetSharedFrame()
{
	m_sharedFrame = NULL;
}

/*
	Frame().
	Pop a frame from the decoder queue.
	User must free this resource!
*/
spCVideoFrame CContentDecoder::Frame()
{
	//mutex::scoped_lock lock( m_sharedFrameMutex );
	
	if ( m_sharedFrame.IsNull() )
	{
		CVideoFrame *tmp = NULL;
	   
		if ( !m_FrameQueue.pop( tmp, false ) )
		{
			tmp = NULL;
		}
	   
		m_sharedFrame = tmp;
	}
	else
		m_sharedFrame->CopyBuffer();


	return m_sharedFrame;
}


/*
	Start.
	Start decoding playlist.
*/
bool	CContentDecoder::Start()
{
	if( m_spPlaylist == NULL )
	{
		g_Log->Warning( "No playlist..." );
		return false;
	}
	
	//abs is a protection against malicious negative numbers giving large integer when converted to uint32
    m_LoopIterations = static_cast<uint32>(g_Settings()->Get( "settings.player.LoopIterations", 2 ));

	//	Start by opening, so we have a context to work with.
	m_bStop = false;

	m_pNextSheepThread = new thread( bind( &CContentDecoder::CalculateNextSheep, this ) );
	
#ifdef WIN32
	SetThreadPriority( (HANDLE)m_pNextSheepThread->native_handle(), THREAD_PRIORITY_BELOW_NORMAL );
	SetThreadPriorityBoost( (HANDLE)m_pNextSheepThread->native_handle(), TRUE );
#else
	struct sched_param sp;
	sp.sched_priority = 8; //Foreground NORMAL_PRIORITY_CLASS - THREAD_PRIORITY_BELOW_NORMAL
	pthread_setschedparam( (pthread_t)m_pNextSheepThread->native_handle(), SCHED_RR, &sp );
#endif

	while ( Initialized() == false )
		thread::sleep( get_system_time() + posix_time::milliseconds(100) );
		
	m_pDecoderThread = new thread( bind( &CContentDecoder::ReadPackets, this ) );
	
	int retry = 0;
	
	CVideoFrame *tmp = NULL;
   
	while ( retry < 10 && !m_FrameQueue.peek( tmp, false ) && !m_bStop)
	{
		thread::sleep( get_system_time() + posix_time::milliseconds(100) );
		retry++;
	}

	if (tmp == NULL)
		return false;

	return true;
}

/*
*/
void	CContentDecoder::Stop()
{
	m_bStop = true;

	if( m_pDecoderThread )
	{
		m_pDecoderThread->interrupt();
		m_pDecoderThread->join();
		SAFE_DELETE( m_pDecoderThread );
	}
	
	if( m_pNextSheepThread )
	{
		m_pNextSheepThread->interrupt();
		m_pNextSheepThread->join();
		SAFE_DELETE( m_pNextSheepThread );
	}
}

void CContentDecoder::ClearQueue( uint32 leave )
{
	while ( m_FrameQueue.size() > leave )
	{
		CVideoFrame *vf;
		
		if ( m_FrameQueue.pop( vf, false, false ) )
		{
			delete vf;
		}
	}
}

/*
*/
uint32	CContentDecoder::QueueLength()
{
	return (uint32)m_FrameQueue.size();
}

/*
*/
void CContentDecoder::ForceNext( int32 forced )
{ 
    upgrade_lock<boost::shared_mutex> lock( m_ForceNextMutex );
	m_bForceNext = forced;
};

/*
*/
int32 CContentDecoder::NextForced( void )
{ 
	shared_lock<boost::shared_mutex> lock( m_ForceNextMutex );
	return m_bForceNext;
};



}
