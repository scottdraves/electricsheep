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
#ifdef WIN32
#include <windows.h>
#endif
#include <vector>
#include <list>

#include "Log.h"
#include "clientversion.h"
#include "ContentDownloader.h"
#include "Networking.h"
#include "Shepherd.h"
#include "SheepUploader.h"
#if defined(WIN32) && defined(_MSC_VER)
#include "../msvc/msvc_fix.h"
#endif

namespace	ContentDownloader
{

//
SheepUploader::SheepUploader()
{
	fAnimID = -1;
	fGen = -1;
	fJob = -1;
	fTime = -1;
	fHasMessage = false;
	fSheepFile = NULL;
}

//
SheepUploader::~SheepUploader()
{
	if(fSheepFile)
	{
		delete [] fSheepFile;
	}
}

//	Sets the sheep file to the given filename.
void SheepUploader::setSheepFile( const char *fullFileName )
{
	if( fSheepFile )
		SAFE_DELETE_ARRAY( fSheepFile );

	if( fullFileName )
	{
		fSheepFile = new char[ strlen( fullFileName ) + 1 ];
		strcpy( fSheepFile, fullFileName );
	}
}

//	Upload the sheep frame that exists on disk.
bool SheepUploader::uploadSheep()
{
	g_Log->Info( "uploadSheep(): %s", fSheepFile );

	//	Validate the file and get the file size of the file.
	long fileSize = -1;
	FILE *f = fopen( fSheepFile, "rb" );
	if ( f != NULL )
	{
		fseek( f, 0, SEEK_END );
		fileSize = ftell( f );
		fclose( f );
	}
	else
	{
		g_Log->Error( "Frame doesn't exist(%s)!", fSheepFile );
		return false;
	}
    
    if  (fileSize <= 0)
        return false;

	
	// Use only the file base name as uploader name, not the whole path.
	char *upname = strrchr( fSheepFile, PATH_SEPARATOR_C );
	
	if ( upname == NULL )
		upname = fSheepFile;
	else
		upname++;
	
	Network::spCFileUploader spUpload = new Network::CFileUploader( upname );
	char url[ MAX_PATH ];
    snprintf( url, MAX_PATH, "%scgi/put?j=%d&id=%d&s=%ld&g=%d&v=%s&u=%s",	ContentDownloader::Shepherd::serverName(),
																					fJob,
																					fAnimID,
																					fileSize,
																					fGen,
																					CLIENT_VERSION,
																					Shepherd::uniqueID() );

	if( !spUpload->PerformUpload( url, fSheepFile, static_cast<uint32>(fileSize) ) )
	{
		g_Log->Error( "Failed to upload %s.\n", url );

		if( spUpload->ResponseCode() == 401 )
			g_ContentDownloader().ServerFallback();

		return false;
	}

	return true;
}

};
