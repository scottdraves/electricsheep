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
#include	<string>
#include	<sys/stat.h>
#include	"ContentDecoder.h"
#include	"Playlist.h"
#include	"Log.h"
#include	"Timer.h"
#include	"Settings.h"
#include	<boost/filesystem.hpp>

using namespace boost;

namespace ContentDecoder
{

/*
	CContentDecoder.

*/
CContentDecoder::CContentDecoder( spCPlaylist _spPlaylist, bool _bStartByRandom, const uint32 _queueLenght, PixelFormat _wantedFormat )
{
	g_Log->Info( "CContentDecoder()" );
	m_FadeCount = g_Settings()->Get("settings.player.fadecount", 30);
	//	We want errors!
	av_log_set_level( AV_LOG_ERROR );

	//	Register all formats and codecs.
	av_register_all();
	
	m_bStartByRandom = _bStartByRandom;

	m_pDecoderThread = NULL;
	
	m_FrameQueue.setMaxQueueElements(_queueLenght);
	
	m_NextSheepQueue.setMaxQueueElements(10);

	m_pVideoStream = NULL;
	m_pVideoCodecContext = NULL;
	m_pFormatContext = NULL;

	m_VideoStreamID = -1;

	m_pScaler = NULL;
	m_Width = 0;
	m_Height = 0;
	m_WantedPixelFormat = _wantedFormat;

	m_bStop = true;

	m_spPlaylist = _spPlaylist;
	
	m_bForceNext = false;
	
	m_CurGeneration = 0;
	m_CurSheepID = 0;
	
	m_lockedFrame = NULL;
	m_bFrameLocked = false;
	
	m_Initialized = false;
	m_NoSheeps = true;

	m_FadeIn = m_FadeCount;
	m_FadeOut = 0;
	m_prevLast = 0;
	m_totalFrameCount = 0;
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

    if( m_pVideoCodecContext )
    {
        avcodec_close( m_pVideoCodecContext );
        m_pVideoCodecContext = NULL;
    }

    if( m_pFormatContext )
    {
        av_close_input_file( m_pFormatContext );
        m_pFormatContext = NULL;
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
bool	CContentDecoder::Open( const std::string &_filename )
{
	m_iCurrentFileFrameCount = 0;
	m_totalFrameCount = 0;
	struct stat fs;
	if ( !::stat( _filename.c_str(), &fs ) )
	{
		m_CurrentFileatime = fs.st_atime;
	}
	g_Log->Info( "Opening: %s", _filename.c_str() );

	Destroy();

	if( DumpError( av_open_input_file( &m_pFormatContext, _filename.c_str(), NULL, 0, NULL ) ) < 0 )
	{
		g_Log->Warning( "Failed to open %s...", _filename.c_str() );
		return false;
	}

	if( DumpError( av_find_stream_info( m_pFormatContext ) ) < 0 )
	{
		g_Log->Error( "av_find_stream_info failed with %s...", _filename.c_str() );
		return false;
	}

	//dump_format( m_pFormatContext, 0, _filename.c_str(), false );

	//	Find video stream;
	m_VideoStreamID = -1;
    for( uint32 i=0; i<m_pFormatContext->nb_streams; i++ )
    {
#ifdef MAC
		if( m_pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO )
#else
        if( m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
#endif
        {
            m_pVideoStream = m_pFormatContext->streams[i];
            m_VideoStreamID = i;
            break;
        }
    }

    if( m_VideoStreamID == -1 )
    {
        g_Log->Error( "Could not find video stream in %s", _filename.c_str() );
        return false;
    }

	//	Find video codec.
    m_pVideoCodecContext = m_pFormatContext->streams[ m_VideoStreamID ]->codec;
    if( m_pVideoCodecContext == NULL )
    {
        m_pVideoCodecContext = NULL;
        g_Log->Error( "Video CodecContext not found for %s", _filename.c_str() );
        return false;
    }

    m_pVideoCodec = avcodec_find_decoder( m_pVideoCodecContext->codec_id );

    if( m_pVideoCodec == NULL )
    {
        m_pVideoCodecContext = NULL;
        g_Log->Error( "Video Codec not found for %s", _filename.c_str() );
        return false;
    }

	//m_pVideoCodecContext->workaround_bugs = 1;
    //m_pFormatContext->flags |= AVFMT_FLAG_GENPTS;		//	Generate pts if missing even if it requires parsing future frames.
    m_pFormatContext->flags |= AVFMT_FLAG_IGNIDX;		//	Ignore index.
    //m_pFormatContext->flags |= AVFMT_FLAG_NONBLOCK;		//	Do not block when reading packets from input.

    if( DumpError( avcodec_open( m_pVideoCodecContext, m_pVideoCodec ) ) < 0 )
    {
        g_Log->Error( "avcodec_open failed for %s", _filename.c_str() );
        return false;
    }
	
	m_totalFrameCount = uint32((((double)m_pFormatContext->duration/(double)AV_TIME_BASE)) * av_q2d(m_pVideoStream->r_frame_rate));

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

	if( m_pScaler )
	{
		av_free( m_pScaler );
		m_pScaler = NULL;
	}

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


/*
	Next().
	Advance to next playlist entry.
*/
bool	CContentDecoder::NextSheepForPlaying( bool _bSkipLoop )
{
	std::string name;

	if( m_spPlaylist == NULL )
	{
		g_Log->Warning("Playlist == NULL");
		return( false );
	}

	uint32 iter = 0;
	
	do 
	{
		bool sheepfound = false;
		
		while ( !sheepfound && m_NextSheepQueue.pop(name, true) )
		{
			if ( name.empty() )
				break;

			uint32 Generation, ID, First, Last;
			std::string fname;
												
			sheepfound = true;
				
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

				while( m_SheepHistoryQueue.size() > 50 )
				{
					std::string tmpstr;
					
					m_SheepHistoryQueue.pop( tmpstr );
				}
				
				m_SheepHistoryQueue.push( name );
				
				if ( _bSkipLoop && ID == m_CurSheepID )
				{
					iter++;
					continue;
				};
				
				if ( ( m_CurSheepID != ID || m_CurGeneration != Generation ) )
				{				
					m_spPlaylist->ChooseSheepForPlaying( Generation, ID );
									
					m_CurSheepID = ID;
					
					m_CurGeneration = Generation;
					
					m_IsEdge = !(First == Last && Last == ID);
				}
			}
			else
			{
				//here we have sheep which is not in form of XXXXX=XXXXX=XXXXX=XXXXX.avi, so it's sheep wait.avi or something.
				
				boost::filesystem::path p( name );
				
				if ( !boost::filesystem::exists( p ) )
				{
					m_NoSheeps = true;
					return true;
				}
			}
			
			_bSkipLoop = false;
			boost::filesystem::path sys_name( name );
			if ( !Open( sys_name.native_file_string() ) )
			{
				sheepfound = false;
				continue;
			} else
			{
				//m_spPlaylist->GetSheepInfoFromPath( name, Generation, ID, First, m_prevLast, fname );
				//m_NextSheepQueue.peek(name, true);
				//m_spPlaylist->GetSheepInfoFromPath( name, Generation, ID, m_nextFirst, Last, fname );
			}
			
			m_NoSheeps = false;
			
			return true;
		}
	} 
	while ( _bSkipLoop && iter < m_LoopIterations);

	return false;
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

/*
	ReadPackets().
	Thread function.
*/
void	CContentDecoder::ReadPackets()
{
	AVFrame		*pFrame = NULL;
	
	AVPacket    packet;

	av_init_packet(&packet);
	
	CVideoFrame *pVideoFrame = NULL;
	
	try {
	
		g_Log->Info( "Packet thread started..." );
		
		if( !NextSheepForPlaying() )
			return;

		pFrame = avcodec_alloc_frame();

		while( true )
		{			
			this_thread::interruption_point();

			bool bNextForced = NextForced();
			
			if (bNextForced)
				ForceNext(false);
		   
			if( !bNextForced && m_pFormatContext != NULL && av_read_frame( m_pFormatContext, &packet ) >= 0 )
			{
				//printf( "av_read_frame done" );
				if( packet.stream_index == m_VideoStreamID )
				{
					int	frameDecoded = 0;

					//printf( "calling av_dup_packet" );
					if( av_dup_packet( &packet ) < 0 )
					{
						g_Log->Warning( "av_dup_packet < 0" );
						continue;
					}

					//printf( "avcodec_decode_video(0x%x, 0x%x, 0x%x, 0x%x, %d)", m_pVideoCodecContext, pFrame, &frameDecoded, packet.data, packet.size );

#if (!defined(LINUX_GNU) || defined(HAVE_AVC_VID2))
					int32 bytesDecoded = avcodec_decode_video2( m_pVideoCodecContext, pFrame, &frameDecoded, &packet );
#else
					int32 bytesDecoded = avcodec_decode_video( m_pVideoCodecContext, pFrame, &frameDecoded, packet.data, packet.size );
#endif
					
					//g_Log->Info( "avcodec_decode_video decoded %d bytes", bytesDecoded );
					if( bytesDecoded < 0 )
						g_Log->Warning( "Failed to decode video frame: bytesDecoded < 0" );

					//	Do we have a fresh frame?
					if( frameDecoded > 0 )
					{
						//g_Log->Info( "frame decoded" );

						//if( pFrame->interlaced_frame )
							//avpicture_deinterlace( (AVPicture *)pFrame, (AVPicture *)pFrame, m_pVideoCodecContext->pix_fmt, m_pVideoCodecContext->width, m_pVideoCodecContext->height );

						//	If the decoded video has a different resolution, delete the scaler to trigger it to be recreated.
						if( m_Width != (uint32)m_pVideoCodecContext->width || m_Height != (uint32)m_pVideoCodecContext->height )
						{
							g_Log->Info( "size doesn't match, recreating" );

							if( m_pScaler )
							{
								g_Log->Info( "deleting m_pScalar" );
								av_free( m_pScaler );
								m_pScaler = NULL;
							}
						}

						//	Make sure scaler is created.
						if( m_pScaler == NULL )
						{
							g_Log->Info( "creating m_pScaler" );

							m_pScaler = sws_getContext(	m_pVideoCodecContext->width, m_pVideoCodecContext->height, m_pVideoCodecContext->pix_fmt,
														m_pVideoCodecContext->width, m_pVideoCodecContext->height, m_WantedPixelFormat, SWS_BICUBIC, NULL, NULL, NULL );

							//	Store width & height now...
							m_Width = m_pVideoCodecContext->width;
							m_Height = m_pVideoCodecContext->height;

							if( m_pScaler == NULL )
								g_Log->Warning( "scaler == null" );
						}

						//printf( "creating pVideoFrame" );
						pVideoFrame = new CVideoFrame( m_pVideoCodecContext, m_WantedPixelFormat, std::string(m_pFormatContext->filename) );
						AVFrame	*pDest = pVideoFrame->Frame();

						//printf( "calling sws_scale()" );
						sws_scale( m_pScaler, pFrame->data, pFrame->linesize, 0, m_pVideoCodecContext->height, pDest->data, pDest->linesize );

						++m_iCurrentFileFrameCount;
						
						/*if (m_totalFrameCount > 0)
						{
							//g_Log->Info("framcount %lu, %lf", (long)(((double)m_pFormatContext->duration/(double)AV_TIME_BASE)),av_q2d(m_pVideoStream->r_frame_rate));
							if (m_iCurrentFileFrameCount == m_totalFrameCount - m_FadeCount)
							{
								if (m_prevLast != m_nextFirst)
								{
									//g_Log->Info("FADING prevLast %u nextFirst %u", m_prevLast, m_nextFirst);
									m_FadeOut = m_FadeCount + 1;
								}
							}
							pVideoFrame->SetMetaData_Fade(1.f);
							if (m_FadeOut > 0)
							{
								--m_FadeOut;
								if (m_FadeOut == 0)
									m_FadeIn = 0;
								pVideoFrame->SetMetaData_Fade(fp4(m_FadeOut) / fp4(m_FadeCount));
								//g_Log->Info("FADING fadeout %u fadein %u framecount %u", m_FadeOut, m_FadeIn, m_iCurrentFileFrameCount);
							}
							if (m_FadeIn < m_FadeCount)
							{
								++m_FadeIn;
								pVideoFrame->SetMetaData_Fade(fp4(m_FadeIn) / fp4(m_FadeCount));
								//g_Log->Info("FADING fadeout %u fadein %u framecount %u", m_FadeOut, m_FadeIn, m_iCurrentFileFrameCount);
							}
						}*/

						pVideoFrame->SetMetaData_SheepID( m_CurSheepID );
						pVideoFrame->SetMetaData_SheepGeneration( m_CurGeneration );
						pVideoFrame->SetMetaData_IsEdge( m_IsEdge );
						pVideoFrame->SetMetaData_atime( m_CurrentFileatime );
						m_FrameQueue.push( pVideoFrame );
						pVideoFrame = NULL;

						//printf( "yielding..." );
						m_pDecoderThread->yield();
					}
				}

				av_free_packet( &packet );
			}
			else
			{					
				g_Log->Info( "calling Next()" );
				
				NextSheepForPlaying( bNextForced );
				
				if ( bNextForced )
					ClearQueue();
			}
		}

	}
	catch(thread_interrupted const&)
	{
	}
	
	if (pVideoFrame)
		delete pVideoFrame;
	
	av_free_packet( &packet );
	
	printf( "decoder thread ending..." );

	if( pFrame )
	{
		av_free( pFrame );
		pFrame = NULL;
	}

	g_Log->Info( "Ending decoder thread..." );
}

void CContentDecoder::LockFrame()
{
	m_bFrameLocked = true;
}

void CContentDecoder::UnlockFrame()
{
   m_bFrameLocked = false;
}

/*
	Frame().
	Pop a frame from the decoder queue.
	User must free this resource!
*/
spCVideoFrame CContentDecoder::Frame()
{
	if ( m_bFrameLocked )
		return m_lockedFrame;
   
	CVideoFrame *tmp = NULL;
   
	if ( !m_FrameQueue.pop( tmp, false ) )
	{
		tmp = NULL;
	}
   
	m_lockedFrame = tmp;
   
	return m_lockedFrame;
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
	
	m_LoopIterations = g_Settings()->Get( "settings.player.LoopIterations", 2 );

	//	Start by opening, so we have a context to work with.
	m_bStop = false;

	m_pNextSheepThread = new thread( bind( &CContentDecoder::CalculateNextSheep, this ) );
	
#ifdef WIN32
	SetThreadPriority( (HANDLE)m_pNextSheepThread->native_handle(), THREAD_PRIORITY_BELOW_NORMAL );
	SetThreadPriorityBoost( (HANDLE)m_pNextSheepThread->native_handle(), TRUE );
#else
	struct sched_param sp;
	int esnRetVal = 0;
	sp.sched_priority = 8; //Foreground NORMAL_PRIORITY_CLASS - THREAD_PRIORITY_BELOW_NORMAL
	esnRetVal = pthread_setschedparam( (pthread_t)m_pNextSheepThread->native_handle(), SCHED_RR, &sp );
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
const uint32	CContentDecoder::QueueLength()
{
	return (uint32)m_FrameQueue.size();
}

/*
*/
void CContentDecoder::ForcePrevious( uint32 _numPrevious )
{ 
	if ( m_SheepHistoryQueue.size() >= _numPrevious )
	{
		std::string name, lastname;
		uint32 i;
				
		for ( i = 0; i < _numPrevious; i++ )
		{
			lastname.assign("");
			
			while ( m_SheepHistoryQueue.pop( name, false, false ) )
			{
				if ( lastname.empty() )
					lastname = name;
					
				if ( lastname != name )
					break;
					
				m_NextSheepQueue.push( name, false, false );
				
				name.assign("");
			}
			
			if ( !name.empty() )
				m_SheepHistoryQueue.push( name );
		}
		
		ForceNext( true );
	}
};



/*
*/
void CContentDecoder::ForceNext( bool forced )
{ 
	mutex::scoped_lock lock( m_ForceNextMutex );
	m_bForceNext = forced;
};

/*
*/
bool CContentDecoder::NextForced( void )
{ 
	mutex::scoped_lock lock( m_ForceNextMutex );
	return m_bForceNext;
};



}
