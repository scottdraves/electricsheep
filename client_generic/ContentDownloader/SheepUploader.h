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
#ifndef _SHEEPUPLOADER_H_
#define _SHEEPUPLOADER_H_

#include "base.h"

namespace ContentDownloader
{

//
// Description:
//		This class uploads the sheep frame that
// is currently on disk.
//
class SheepUploader
{
	int32	fAnimID;
	int32	fGen;
	int32	fJob;
	int32	fTime;
	bool	fHasMessage;
	char	*fSheepFile;

	protected:
			//	Uploads the sheep from disk.
			bool uploadSheep();

	public:
			SheepUploader();
			virtual ~SheepUploader();

			//	Sets/gets the sheep ID for the frame on disk.
			void setSheepID( const int32 &animID ) { fAnimID = animID; }
			int32 sheepID() const { return fAnimID; }

			// sets/gets the sheep file on disk
			void setSheepFile( const char *fullFileName );
			const char *sheepFile() const { return fSheepFile; }

			// sets/gets the sheep generation for the frame on disk
			void setSheepGeneration( const int32 &gen ) { fGen = gen; }
			int32 sheepGeneration() const { return fGen; }

			// sets/gets the job for the frame on disk.
			void setSheepJob( const int32 &job )	{	fJob = job;	}
			int32 sheepJob() const { return fJob; }

			// sets/gets the sheep frame for the frame on disk
			void setSheepTime( const int32 &time )	{	fTime = time;	};
			int32 sheepTime() { return fTime ; };

			// sets/gets if the server has sent a message
			void setHasMessage(const bool &hasMessage) { fHasMessage = hasMessage; }
			bool hasMessage() const { return fHasMessage; }

			friend class SheepGenerator;
};

};

#endif
