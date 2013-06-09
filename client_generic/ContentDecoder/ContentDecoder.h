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
#ifndef _CONTENTDECODER_H
#define _CONTENTDECODER_H


//	FFmpeg headers.
extern "C"{
#if defined(WIN32) || defined(MAC) || defined (LINUX_GNU)
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
#else
	#include "avcodec.h"
	#include "avformat.h"
	#include "swscale.h"
#endif
}

#include	"base.h"
#include	<string>
#include	<queue>
#include	"boost/thread/thread.hpp"
#include	"boost/thread/mutex.hpp"
#include	"boost/thread/xtime.hpp"
#include	"boost/bind.hpp"
#include	"Frame.h"
#include	"Playlist.h"
#include	"BlockingQueue.h"

namespace ContentDecoder
{

struct sOpenVideoInfo
{
	sOpenVideoInfo() 
	:	m_pFrame(NULL),
		m_pFormatContext(NULL),
		m_pVideoCodecContext(NULL),
		m_pVideoCodec(NULL),
		m_pVideoStream(NULL),
		m_VideoStreamID(-1),
		m_pScaler(NULL),
		m_totalFrameCount(0),
		m_CurrentFileatime(0),
		m_iCurrentFileFrameCount(0),
		m_Width(0),
		m_Height(0),
		m_Generation(0),
		m_SheepID(0),
		m_First(0),
		m_Last(0),
		m_bSpecialSheep(false),
		m_NumIterations(0),
		m_NextIsSeam(false)
		
	{ }
	
	sOpenVideoInfo( const sOpenVideoInfo* ovi)
	:	m_pFrame(NULL),
		m_pFormatContext(NULL),
		m_pVideoCodecContext(NULL),
		m_pVideoCodec(NULL),
		m_pVideoStream(NULL),
		m_VideoStreamID(-1),
		m_pScaler(NULL),
		m_totalFrameCount(0),
		m_CurrentFileatime(0),
		m_iCurrentFileFrameCount(0),
		m_Width(0),
		m_Height(0),
		m_Generation(ovi->m_Generation),
		m_SheepID(ovi->m_SheepID),
		m_First(ovi->m_First),
		m_Last(ovi->m_Last),
		m_Path(ovi->m_Path),
		m_bSpecialSheep(ovi->m_bSpecialSheep),
		m_NumIterations(ovi->m_NumIterations),
		m_NextIsSeam(false)
	{ }
	
	virtual ~sOpenVideoInfo()
	{
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
		
		if( m_pScaler )
		{
			av_free( m_pScaler );
			m_pScaler = NULL;
		}
		
		if ( m_pFrame )
		{
			av_free( m_pFrame );
			m_pFrame = NULL;
		}
	}
	
	bool IsLoop() { return (!m_bSpecialSheep && !IsEdge()); }
	
	void Reset() { m_pFormatContext = NULL; }
	
	bool IsOpen() { return (m_pFormatContext != NULL); }
	
	bool EqualsTo( sOpenVideoInfo *ovi )
	{
		return ( m_SheepID == ovi->m_SheepID && m_Generation == ovi->m_Generation );
	}
	
	bool IsEdge()
	{
		if (m_bSpecialSheep)
			return false;
		
		return !(m_First == m_Last && m_Last == m_SheepID);
	}
	
	AVFrame			*m_pFrame;
	AVFormatContext	*m_pFormatContext;
	AVCodecContext	*m_pVideoCodecContext;
	AVCodec			*m_pVideoCodec;
	AVStream		*m_pVideoStream;
	int32			m_VideoStreamID;
	SwsContext		*m_pScaler;
	uint32			m_totalFrameCount;
	time_t			m_CurrentFileatime;
	uint32			m_iCurrentFileFrameCount;
	//	These are to track changes in input stream resolution, and recreate m_pScaler if needed.
	uint32			m_Width;
	uint32			m_Height;
	uint32			m_Generation;
	uint32			m_SheepID;
	uint32			m_First;
	uint32			m_Last;
	std::string		m_Path;
	bool			m_bSpecialSheep;
	uint32			m_NumIterations;
	bool			m_NextIsSeam;
};

/*
	CContentDecoder.
	Video decoding wrapper for ffmpeg, influenced by glover.
*/
class CContentDecoder
{
	bool			m_bStop;
	uint32			m_prevLast;
	uint32			m_nextFirst;


	uint32				m_FadeIn;
	uint32				m_FadeOut;
	uint32				m_FadeCount;
	

	//	Thread & threadfunction.
	boost::thread	*m_pDecoderThread;
	void			ReadPackets();
	
	boost::thread	*m_pNextSheepThread;
	void			CalculateNextSheep();

	//	Queue for decoded frames.
	Base::CBlockingQueue<CVideoFrame *>	m_FrameQueue;
	boost::mutex	m_ForceNextMutex;
	uint32			m_CacheLow;

	//	Codec context & working objects.
	sOpenVideoInfo		*m_MainVideoInfo;
	
	sOpenVideoInfo		*m_SecondVideoInfo;
	
	PixelFormat		m_WantedPixelFormat;


	spCPlaylist		m_spPlaylist;
	
	Base::CBlockingQueue<std::string>	m_NextSheepQueue;
	Base::CBlockingQueue<std::string>	m_SheepHistoryQueue;
	
	
	uint32			m_LoopIterations;
	
	bool			m_bForceNext;
	
	spCVideoFrame	m_sharedFrame;
	boost::mutex	m_sharedFrameMutex;
	
	bool			m_bStartByRandom;

	bool			m_NoSheeps;
	
	bool			m_Initialized;
	
	bool			m_bCalculateTransitions;

	bool	Open( sOpenVideoInfo *ovi );
	sOpenVideoInfo*		GetNextSheepInfo();
	bool	NextSheepForPlaying( bool _bSkipLoop = false );
	void	Destroy();
	
	CVideoFrame *ReadOneFrame(sOpenVideoInfo *ovi);

	static int DumpError( int _err );

	public:
			CContentDecoder( spCPlaylist _spPlaylist, bool _bStartByRandom, bool _bAllowTransitions, const uint32 _queueLenght, PixelFormat _wantedPixelFormat = PIX_FMT_RGB24 );
			virtual ~CContentDecoder();

			bool	Initialized() { return m_Initialized; }
			void	Close();
			bool	Start();
			void	Stop();

			//CVideoFrame *DecodeFrame();
			void	ResetSharedFrame();
			spCVideoFrame Frame();

			bool	Stopped()	{	return m_bStop; };
			bool	Healthy()	{ return true; };

			bool	PlayNoSheepIntro()	
			{ 
				return m_NoSheeps; 
			};
			
			inline uint32	GetCurrentPlayingID()				{	return (m_MainVideoInfo != NULL) ? m_MainVideoInfo->m_SheepID : 0;	};
			inline uint32	GetCurrentPlayingGeneration()		{	return (m_MainVideoInfo != NULL) ? m_MainVideoInfo->m_Generation : 0; };

			const uint32	QueueLength();
			
			void ClearQueue( uint32 leave = 0 );
			
			void ForceNext( bool forced = true );
			void ForcePrevious( uint32 _numPrevious );
			bool NextForced( void );
};

MakeSmartPointers( CContentDecoder );

}

#endif
