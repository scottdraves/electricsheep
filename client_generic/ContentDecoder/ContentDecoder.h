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

/*
	CContentDecoder.
	Video decoding wrapper for ffmpeg, influenced by glover.
*/
class CContentDecoder
{
	bool			m_bStop;
	uint32			m_iCurrentFileFrameCount;
	uint32			m_prevLast;
	uint32			m_nextFirst;

	time_t			m_CurrentFileatime;

	uint32				m_FadeIn;
	uint32				m_FadeOut;
	uint32				m_FadeCount;
	
	uint32				m_totalFrameCount;

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
	AVFormatContext	*m_pFormatContext;
	AVCodecContext	*m_pVideoCodecContext;
	AVCodec			*m_pVideoCodec;
	AVStream		*m_pVideoStream;
	int32			m_VideoStreamID;
	AVFrame			*m_pFrame;

	PixelFormat		m_WantedPixelFormat;
	SwsContext		*m_pScaler;

	//	These are to track changes in input stream resolution, and recreate m_pScaler if needed.
	uint32			m_Width;
	uint32			m_Height;

	spCPlaylist		m_spPlaylist;
	
	Base::CBlockingQueue<std::string>	m_NextSheepQueue;
	Base::CBlockingQueue<std::string>	m_SheepHistoryQueue;
	
	uint32			m_CurGeneration;
	uint32			m_CurSheepID;
	bool			m_IsEdge;
	
	uint32			m_LoopIterations;
	
	bool			m_bForceNext;
	
	spCVideoFrame	m_lockedFrame;
	bool			m_bFrameLocked;
	
	bool			m_bStartByRandom;

	bool			m_NoSheeps;
	
	bool			m_Initialized;

	bool	Open( const std::string &_filename );
	bool	NextSheepForPlaying( bool _bSkipLoop = false );
	void	Destroy();

	static int DumpError( int _err );

	public:
			CContentDecoder( spCPlaylist _spPlaylist, bool _bStartByRandom, const uint32 _queueLenght, PixelFormat _wantedPixelFormat = PIX_FMT_RGB24 );
			virtual ~CContentDecoder();

			bool	Initialized() { return m_Initialized; }
			void	Close();
			bool	Start();
			void	Stop();

			//CVideoFrame *DecodeFrame();
			void	LockFrame();
			void	UnlockFrame();
			spCVideoFrame Frame();

			bool	Stopped()	{	return m_bStop; };
			bool	Healthy()	{ return true; };

			bool	PlayNoSheepIntro()	
			{ 
				return m_NoSheeps; 
			};
			
			inline uint32	GetCurrentPlayingID()				{	return m_CurSheepID;	};
			inline uint32	GetCurrentPlayingGeneration()		{	return m_CurGeneration;	};

			const uint32	QueueLength();
			
			void ClearQueue( uint32 leave = 0 );
			
			void ForceNext( bool forced = true );
			void ForcePrevious( uint32 _numPrevious );
			bool NextForced( void );
};

MakeSmartPointers( CContentDecoder );

}

#endif
