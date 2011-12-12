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
#include <iostream>
#include <fstream>
#include <iterator>
#include <exception>
#include <sstream>
#include <iomanip>
#include <list>

//#include <boost/format.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>

#include <zlib.h>
#include <time.h>
#include <math.h>

#include "io.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <sys/param.h>
#include <sys/mount.h>
#endif
#ifdef LINUX_GNU
#include <sys/statfs.h>
#include <limits.h>
#define MAX_PATH PATH_MAX
#endif

#include "base.h"
#include "MathBase.h"
#include "clientversion.h"
#include "Networking.h"
#include "ContentDownloader.h"
#include "SheepDownloader.h"
#ifdef WIN32
#include "SheepGenerator.h"
#endif
#include "Shepherd.h"
#include "Settings.h"
#include "Log.h"
#include "Timer.h"
#include "PlayCounter.h"
#if defined(WIN32) && defined(_MSC_VER)
#include "../msvc/msvc_fix.h"
#endif

namespace ContentDownloader
{

using namespace std;
using namespace boost;

#define MAXBUF 1024
#define TIMEOUT 600
static const int32 MAX_TIMEOUT = 24*60*60; // 1 day
#define MIN_MEGABYTES 1024
#define MIN_READ_INTERVAL 3600

static const uint32 INIT_DELAY = 60;

// Initialize the class data
int SheepDownloader::fDownloadedSheep = 0;
int SheepDownloader::fCurrentGeneration = -1;
bool SheepDownloader::fGotList = false;
bool SheepDownloader::fListDirty = true;
time_t SheepDownloader::fLastListTime = 0;

boost::mutex SheepDownloader::s_DownloaderMutex;

/*
*/
SheepDownloader::SheepDownloader()
{
	fHasMessage = false;
	fCurrentGeneration = -1;
	m_bAborted = false;
	fGotList = false;
	fListDirty = true;
	updateCachedSheep();
	deleteCached(0, 0);
	deleteCached(0, 1);
}

/*
*/
SheepDownloader::~SheepDownloader()
{
	clearFlocks();
}

/*
*/
void	SheepDownloader::initializeDownloader()
{
}

/*
*/
void	SheepDownloader::closeDownloader()
{
}

/*
	numberOfDownloadedSheep().
	Returns the total number of downloaded sheep.
*/
int	SheepDownloader::numberOfDownloadedSheep()
{
 	boost::mutex::scoped_lock lockthis( s_DownloaderMutex );

	int returnVal;
	returnVal = fDownloadedSheep;
	return returnVal;
}

/*
	clearFlocks().
	Clears the sheep arrays for the client and the server and deletes the memory they allocated.
*/
void SheepDownloader::clearFlocks()
{
	boost::mutex::scoped_lock lockthis( s_DownloaderMutex );
	//	Clear the server flock.
	for( uint32 i=0; i < fServerFlock.size(); i++ )
		SAFE_DELETE( fServerFlock[i] );

	fServerFlock.clear();

	fGotList = false;

	fListDirty = true;

	//	Clear the client flock.
	for(unsigned i = 0; i < fClientFlock.size(); i++)
		delete fClientFlock[i];

	fClientFlock.clear();
}

void SheepDownloader::Abort( void )
{
	boost::mutex::scoped_lock lockthis( m_AbortMutex );

	m_bAborted = true;
}

//	Downloads the given sheep from the server and supplies a unique name for it based on it's ids.
bool SheepDownloader::downloadSheep( Sheep *sheep )
{
	if( sheep->downloaded() )
		return false;

	char tmp[32];
	//	To identify transfer.
	snprintf( tmp, 32, "Sheep #%d.%05d", sheep->generation(), sheep->id() );

	m_spSheepDownloader = new Network::CFileDownloader( tmp );

	Network::spCFileDownloader spDownload = m_spSheepDownloader;

	bool dlded = spDownload->Perform( sheep->URL() );

	m_spSheepDownloader = NULL;

	{
		boost::mutex::scoped_lock lockthis( m_AbortMutex );

		if ( m_bAborted )
			return false;
	}

	if( !dlded )
	{
		g_Log->Warning( "Failed to download %s.\n", sheep->URL() );

		if( spDownload->ResponseCode() == 401 )
			g_ContentDownloader().ServerFallback();

		return false;
	}

	if (sheep->fileSize() != spDownload->Data().size())
	{
		g_Log->Warning( "Failed to download %s - file size mismatch.\n", sheep->URL() );
		return false;
	}
	//	Save file.
	char filename[ MAXBUF ];
    snprintf( filename, MAXBUF, "%s%05d=%05d=%05d=%05d.avi", Shepherd::mpegPath(), sheep->generation(), sheep->id(), sheep->firstId(), sheep->lastId() );
    if( !spDownload->Save( filename ) )
    {
    	g_Log->Error( "Unable to save %s\n", filename );
    	return false;
    }

    return true;
}

#if 1//DASVO_TEST
//
void SheepDownloader::handleListElement(TiXmlElement* listElement)
{
	int cur_gen = -1;

	listElement->QueryIntAttribute("gen", &cur_gen);

	if (cur_gen <= 0 )
		g_Log->Error( "generation must be positive.\n" );

	setCurrentGeneration( cur_gen );

	TiXmlElement* pChildNode;
	for ( pChildNode = listElement->FirstChildElement(); pChildNode; pChildNode = pChildNode->NextSiblingElement() )
	{
		const char* name = pChildNode->Value();

		if( !strcmp( "sheep", name ) )
		{
			const char *state = NULL;
			if( -1 == currentGeneration() )
				g_Log->Error( "malformed list, received sheep without generation set.\n" );

			//	Create a new sheep and parse the attributes.
			Sheep *newSheep = new Sheep();
			newSheep->setGeneration( currentGeneration() );

			const char *a;
			if (a = pChildNode->Attribute("id"))		newSheep->setId(atoi(a));
			if (a = pChildNode->Attribute("type"))		newSheep->setType(atoi(a));
			if (a = pChildNode->Attribute("time"))		newSheep->setFileWriteTime(atoi(a));
			if (a = pChildNode->Attribute("size"))		newSheep->setFileSize(atol(a));
			if (a = pChildNode->Attribute("rating"))	newSheep->setRating(atoi(a));
			if (a = pChildNode->Attribute("first"))		newSheep->setFirstId(atoi(a));
			if (a = pChildNode->Attribute("last"))		newSheep->setLastId(atoi(a));
			if (a = pChildNode->Attribute("state"))		state = a;
			if (a = pChildNode->Attribute("url"))		newSheep->setURL(a);
			// TODO: fix malformed state == NULL potential error
			if( !strcmp(state, "done") && (0 == newSheep->type()) )
                fServerFlock.push_back(newSheep);
			else
			{
				if( !strcmp( state, "expunge" ) )
				{
					char buf[ MAXBUF ];
					sprintf(buf, "%s%05d=%05d=%05d=%05d.avi", Shepherd::mpegPath(), newSheep->generation(), newSheep->id(), newSheep->firstId(), newSheep->lastId() );
					remove( buf );
				}

				SAFE_DELETE( newSheep );
			}
		}
		else if( !strcmp( "message", name ) )
		{
			setHasMessage( true );

			const char *txt = pChildNode->GetText();
			if (txt)
				Shepherd::addMessageText( txt, strlen(txt), 30 );

		}
		else if( !strcmp( "error", name ) )
		{
			char server_error_type[ MAXBUF ];

			const char *a = pChildNode->Attribute("type");

			if( a )
			{
				if( !strcmp( a, "unauthenticated" ) )
					strncpy( server_error_type, "Invalid Nickname or Password", MAXBUF );
				else
					strncpy( server_error_type, a, MAXBUF );

				Shepherd::addMessageText( server_error_type, strlen( server_error_type ), 180 ); //	3 minutes
			}
		}
	}
}
#else
//
void SheepDownloader::getEndElement(void *userData, const char *name)
{
	SheepDownloader *dl = (SheepDownloader *)userData;
    if( !strcmp( "message", name ) )
		dl->setHasMessage( false );
}

//
void SheepDownloader::characterHandler(void *userData, const char *s, int len)
{
	SheepDownloader *dl = (SheepDownloader *)userData;
    if( dl->hasMessage() )
		Shepherd::addMessageText( s, len, 30 );
}

//
void SheepDownloader::listStartElement(void *userData, const char *name, const char **atts)
{
	SheepDownloader *dl = (SheepDownloader *)userData;
	int i = 0;

    if( !strcmp( "list", name ) )
	{
		while( atts[i] )
		{
			if( !strcmp( atts[i], "gen" ) )
			{
				dl->setCurrentGeneration( atoi( atts[i+1] ) );
				if (dl->currentGeneration() <= 0 )
					g_Log->Error( "generation must be positive.\n" );

				break;
			}
			i += 2;
		}
    }
	else if( !strcmp( "sheep", name ) )
	{
		const char *state = NULL;
		if( -1 == dl->currentGeneration() )
			g_Log->Error( "malformed list, received sheep without generation set.\n" );

		//	Create a new sheep and parse the attributes.
		Sheep *newSheep = new Sheep();
		newSheep->setGeneration( dl->currentGeneration() );

		while( atts[i] )
		{
			const char *a = atts[i+1];
			if (!strcmp(atts[i], "id"))				newSheep->setId(atoi(a));
			else if (!strcmp(atts[i], "type"))		newSheep->setType(atoi(a));
			else if (!strcmp(atts[i], "time"))		newSheep->setFileWriteTime(atoi(a));
			else if (!strcmp(atts[i], "size"))		newSheep->setFileSize(atol(a));
			else if (!strcmp(atts[i], "rating"))	newSheep->setRating(atoi(a));
			else if (!strcmp(atts[i], "first"))		newSheep->setFirstId(atoi(a));
			else if (!strcmp(atts[i], "last"))		newSheep->setLastId(atoi(a));
			else if (!strcmp(atts[i], "state"))		state = a;
			else if (!strcmp(atts[i], "url"))		newSheep->setURL(a);
			i += 2;
		}
		if( !strcmp(state, "done") && (0 == newSheep->type()) )
            dl->fServerFlock.push_back(newSheep);
		else
		{
			if( !strcmp( state, "expunge" ) )
			{
				char buf[ MAXBUF ];
				sprintf(buf, "%s%05d=%05d=%05d=%05d.avi", Shepherd::mpegPath(), newSheep->generation(), newSheep->id(), newSheep->firstId(), newSheep->lastId() );
				remove( buf );
			}

			SAFE_DELETE( newSheep );
		}
    }
	else if( !strcmp( "message", name ) )	dl->setHasMessage( true );
	else if( !strcmp( "error", name ) )
	{
		char server_error_type[ MAXBUF ];

		while( atts[i] )
		{
			if( !strcmp( atts[i], "type" ) )
			{
				if( !strcmp( atts[i+1], "unauthenticated" ) )
					strncpy( server_error_type, "Invalid Nickname or Password", MAXBUF );
				else
					strncpy( server_error_type, atts[i+1], MAXBUF );

				Shepherd::addMessageText( server_error_type, strlen( server_error_type ), 180 ); //	3 minutes
			}

			i += 2;
		}
    }
}
#endif

//	Parse the sheep list and create the server sheep.
void SheepDownloader::parseSheepList()
{
 	boost::mutex::scoped_lock lockthis( s_DownloaderMutex );

	char pbuf[MAXBUF];

	//	Open the file.
	sprintf(pbuf, "%slist_%s.xml", Shepherd::xmlPath(), Shepherd::role());

	TiXmlDocument doc(pbuf);
	if (doc.LoadFile())
	{
		TiXmlHandle hDoc(&doc);
		TiXmlElement* listElement;

		listElement=hDoc.FirstChild( "list" ).Element();
		
		//only if there is at least one child in the list, we delete the list from previous request
		if (listElement && listElement->FirstChildElement() != NULL)
		{
			fGotList = false;

			SheepArray::iterator it = fServerFlock.begin();
			while(it != fServerFlock.end())
			{
				delete *it;
				it++;
			}
			fServerFlock.clear();

			handleListElement(listElement);
		}
		else 
		{
			g_Log->Error( "There are no sheep in the downloaded list, is that correct?!?\n" );
			return;
		}
	}
	else
	{
		g_Log->Error( "%s at line %d\n", doc.ErrorDesc(), doc.ErrorRow() );
		return;
	}

	//remove( pbuf );
	fGotList = true;
	fListDirty = false;
}

/*
	updateCachedSheep().
	Run through the client cache to update it.
*/
void	SheepDownloader::updateCachedSheep()
{
 	boost::mutex::scoped_lock lockthis( s_DownloaderMutex );

	//	Get the client flock.
	if( Shepherd::getClientFlock( &fClientFlock ) )
	{
		//	Run through the client flock to find deleted sheep.
		for( uint32 i=0; i<fClientFlock.size(); i++ )
		{
			//	Get the current sheep.
			Sheep *currentSheep = fClientFlock[i];

			//	Check if it is deleted.
			if( currentSheep->deleted() && fGotList )
			{
				//	If it is than run through the server flock to see if it is still there.
				uint32 j;
				for( j=0; j<fServerFlock.size(); j++ )
				{
					if( fServerFlock[j]->id() == currentSheep->id() && fServerFlock[j]->generation() == currentSheep->generation() )
						break;
				}

				//	If it was not found on the server then it is time to delete the file.
				if( j == fServerFlock.size() )
				{
					g_Log->Info( "Deleting %s", currentSheep->fileName() );
					if (remove( currentSheep->fileName() ) != 0)
						g_Log->Warning( "Failed to remove %s", currentSheep->fileName());
					else
					{
						Shepherd::subClientFlockBytes(currentSheep->fileSize(), currentSheep->getGenerationType());
						Shepherd::subClientFlockCount(currentSheep->getGenerationType());
					}
					continue;
				}
				continue;
			}
			else if( currentSheep->isTemp() )
			{
				g_Log->Info( "Deleting %s", currentSheep->fileName() );
				if (remove( currentSheep->fileName() ) != 0)
					g_Log->Warning( "Failed to remove %s", currentSheep->fileName());
				continue;
			}

			if (fGotList)
			{
				//	Update the sheep rating from the server.
				for( uint32 j=0; j<fServerFlock.size(); j++ )
				{
					Sheep *shp = fServerFlock[j];
					if( shp->id() == currentSheep->id() && shp->generation() == currentSheep->generation() )
						if( shp->firstId() == currentSheep->firstId() )
							if( shp->lastId() == currentSheep->lastId() )
							{
								currentSheep->setRating( shp->rating() );
								break;
							}
				}
			}

			//	Should use win native call but i don't know what it is.
			struct stat stat_buf;
			if( -1 != stat( currentSheep->fileName(), &stat_buf ) )
				currentSheep->setFileWriteTime( stat_buf.st_mtime );
		}

//		fRenderer->updateClientFlock( fClientFlock );
	}
}

/*
	cacheOverflow().
	Checks if the cache will overflow if the given number of bytes are added.
*/
int	SheepDownloader::cacheOverflow( const double &bytes, const int getGenerationType ) const
{
	boost::uintmax_t availableBytes;

#ifdef WIN32
	ULARGE_INTEGER avBytes;

	//	Get the available number of bytes on the disk.
	if( !GetDiskFreeSpaceExA( Shepherd::xmlPath(), (PULARGE_INTEGER)&avBytes, NULL, NULL ) )
	{
		g_Log->Error( "Unable to get free disc space" );
		avBytes.QuadPart = 0;
		exit(1);
	}

	availableBytes = (boost::uintmax_t)avBytes.QuadPart;
#else
	struct statfs buf;
	if(statfs(Shepherd::xmlPath(), &buf) < 0)
	{
		g_Log->Error( "not enough space on disk" );
		exit(1);
	}

	availableBytes = (boost::uintmax_t)buf.f_bavail * (boost::uintmax_t)buf.f_bsize;
#endif

	//	Return the overflow status
    return (Shepherd::cacheSize(getGenerationType) && (bytes > (1024.0 * 1024.0 * Shepherd::cacheSize(getGenerationType))));
}

/*
	deleteCached().
	This function will make sure there is enough room in the cache for any newly downloaded files.
	If the cache is to large than the oldest and worst rated files will be deleted.
*/
void	SheepDownloader::deleteCached( const int &size, const int getGenerationType )
{
	double total;
    time_t oldest_time; // oldest time for sheep with highest playcount
    int highest_playcount;
    int best; // oldest sheep with highest playcount
	time_t oldest_sheep_time; // oldest time for sheep (from whole flock)
	int oldest; // oldest sheep index

//	updateCachedSheep();

	if( Shepherd::cacheSize(getGenerationType) != 0 )
	{
		while( fClientFlock.size() )
		{
			//	Initialize some data.
			total = size;
			oldest_time = 0;
			highest_playcount = 0;
			best = 0;
			oldest = 0;
			oldest_sheep_time = 0;

			//	Iterate the client flock to get the oldest and worst_rated file.
			for( uint32 i=0; i<fClientFlock.size(); i++ )
			{
				Sheep *curSheep = fClientFlock[i];
				//	If the file is allready deleted than skip.
				if( curSheep->deleted() || curSheep->isTemp() || curSheep->getGenerationType() != getGenerationType )
					continue;

				//	Store the file size.
				total += curSheep->fileSize();
				if ( oldest_sheep_time == 0 || oldest_sheep_time < curSheep->fileWriteTime() )
				{
					oldest = i;
					oldest_sheep_time = curSheep->fileWriteTime();
				}
				if( !oldest_time || 
					(g_PlayCounter().PlayCount(curSheep->generation(), curSheep->id()) > highest_playcount) ||
					( (g_PlayCounter().PlayCount(curSheep->generation(), curSheep->id()) == highest_playcount) && (curSheep->fileWriteTime() < oldest_time) )
					)
				{
					//	Update this as the file to delete if necessary.
					best = i;
					oldest_time = curSheep->fileWriteTime();
					highest_playcount = g_PlayCounter().PlayCount(curSheep->generation(), curSheep->id());
				}
			}

			//	If a file is found and the cache has overflowed then start deleting.
			if( oldest_time && cacheOverflow( total, getGenerationType ) )
			{
				if (rand() % 2 == 0)
				{
					best = oldest;
					oldest_time = oldest_sheep_time;
					g_Log->Info("Deleting oldest sheep");
				} else
					g_Log->Info("Deleting most played sheep");

				std::string filename(fClientFlock[ best ]->fileName());
				if (filename.find_last_of("/\\") != filename.npos)
					filename.erase( filename.begin(), filename.begin() + filename.find_last_of("/\\") + 1);

				uint16 playcount = g_PlayCounter().PlayCount( fClientFlock[ best ]->generation(), fClientFlock[ best ]->id() ) - 1;
				std::stringstream temp;
				std::string temptime = ctime( &oldest_time );
				temptime.erase(temptime.size() - 1);
				
				temp << "Deleted: " << filename << ", played:" << playcount << " time" <<  ((playcount == 1) ? "," : "s,") << temptime;
				Shepherd::AddOverflowMessage( temp.str() );
				g_Log->Info("%s", temp.str().c_str());
				deleteSheep( fClientFlock[ best ] );
			}
			else
			{
				//	Cache is ok so return.
				return;
			}
		}
    }
}

/*
	deleteSheep().

*/
void	SheepDownloader::deleteSheep( Sheep *sheep )
{
	if (remove( sheep->fileName() ) != 0)
		g_Log->Warning( "Failed to remove %s", sheep->fileName());
	else
	{
		Shepherd::subClientFlockBytes(sheep->fileSize(), sheep->getGenerationType());
		Shepherd::subClientFlockCount(sheep->getGenerationType());
	}
	sheep->setDeleted( true );

	//	Create the filename with an xxx extension.
	size_t len = strlen( sheep->fileName() );
	char *deletedFile = new char[ len + 1 ];
	strcpy( deletedFile, sheep->fileName() );

	deletedFile[len - 3] = 'x';
	deletedFile[len - 2] = 'x';
	deletedFile[len - 1] = 'x';

	//	Open the deleted file and save the zero length file.
	FILE *out = fopen( deletedFile, "wb" );
	if( out != NULL )
		fclose( out );

	SAFE_DELETE_ARRAY( deletedFile );

	/*if( sheep->firstId() == sheep->lastId() )
	{
		//	It's a loop so you might as well delete the edges as well.
		for( uint32 i=0; i<fClientFlock.size(); i++ )
		{
			Sheep *curSheep = fClientFlock[i];

			//	If the file is allready deleted than skip.
			if( curSheep->deleted() || curSheep->isTemp() )
				continue;

			if( curSheep->firstId() == sheep->id() || curSheep->lastId() == sheep->id() )
				deleteSheep( curSheep );
		}
	}*/
}

/*
*/
void	SheepDownloader::deleteSheepId( int sheepId )
{
	for( uint32 i=0; i<fClientFlock.size(); i++ )
	{
		Sheep *curSheep = fClientFlock[i];
		if( curSheep->deleted() || curSheep->isTemp() )
			continue;

		if( curSheep->id() == sheepId )
			deleteSheep( curSheep );
	}
}

bool SheepDownloader::isFolderAccessible( const char *folder )
{
	if (folder == NULL)
		return false;
#ifdef WIN32
	if (_access(folder, 6) != 0)
		return false;
#else
	if (access(folder, 6) != 0)
		return false;
#endif

	struct stat status;
	std::string tempstr(folder);
	if (tempstr.size() > 1 && (tempstr.at(tempstr.size()-1) == '\\' || tempstr.at(tempstr.size()-1) == '/'))
		tempstr.erase( tempstr.size() - 1 );
	if ( stat( tempstr.c_str(), &status ) == -1)
		return false;

	if ( !(status.st_mode & S_IFDIR) )
		return false; // it's a file

	return true;
}

/*
	findSheepToDownload().
	This method loads all of the sheep that are cached on disk and deletes any files in the cache that no longer exist on the server
*/
void	SheepDownloader::findSheepToDownload()
{
	int best_rating;
	time_t best_ctime;
    int best_anim;
	int best_rating_old;
	int best_anim_old;
	time_t best_ctime_old;

	try {
#ifndef	DEBUG

		//if there are at least three sheep to display in content folder, sleep, otherwise start to download immediately
		if ( fClientFlock.size() > 3 )
		{
			std::stringstream tmp;
			
			tmp << "Downloading starts in {" << (int32)ContentDownloader::INIT_DELAY << "}...";
		
			Shepherd::setDownloadState(tmp.str());
			
			//	Make sure we are really deeply settled asleep, avoids lots of timed out frames.
			g_Log->Info( "Chilling for %d seconds before trying to download sheeps...", ContentDownloader::INIT_DELAY );
			
			thread::sleep( get_system_time() + posix_time::seconds(ContentDownloader::INIT_DELAY) );
		}
#endif

	boost::uintmax_t lpFreeBytesAvailable = 0, lpTotalNumberOfBytes = 0;

	int32	failureSleepDuration = TIMEOUT;
	int32	badSheepSleepDuration = TIMEOUT;


		while( 1 )
		{

			this_thread::interruption_point();
			bool incorrect_folder = false;
#ifdef WIN32
			ULARGE_INTEGER winlpFreeBytesAvailable, winlpTotalNumberOfBytes, winlpRealBytesAvailable;

			if ( GetDiskFreeSpaceExA( Shepherd::xmlPath(),(PULARGE_INTEGER)&winlpFreeBytesAvailable,(PULARGE_INTEGER)&winlpTotalNumberOfBytes,(PULARGE_INTEGER)&winlpRealBytesAvailable ) )
			{
				lpFreeBytesAvailable = winlpFreeBytesAvailable.QuadPart;
				lpTotalNumberOfBytes = winlpTotalNumberOfBytes.QuadPart;
			} else
				incorrect_folder = true;
#else
			struct statfs buf;
			if( statfs( Shepherd::xmlPath(), &buf) >= 0 )
			{
				lpFreeBytesAvailable = (boost::uintmax_t)buf.f_bavail * (boost::uintmax_t)buf.f_bsize;
				lpTotalNumberOfBytes = (boost::uintmax_t)buf.f_blocks * (boost::uintmax_t)buf.f_bsize;
			} else
				incorrect_folder = true;
#endif

			incorrect_folder = !isFolderAccessible( Shepherd::xmlPath() ) || !isFolderAccessible( Shepherd::mpegPath() );
			if( lpFreeBytesAvailable < ((boost::uintmax_t)MIN_MEGABYTES * 1024 * 1024) || incorrect_folder)
			{
				if (incorrect_folder)
				{
					const char *err = "Folder is not working.  Downloading disabled.\n";
					Shepherd::addMessageText( err, strlen(err), 180 ); //3 minutes

					thread::sleep( get_system_time() + posix_time::seconds(TIMEOUT) );
				}
				else
				{
					const char *err = "Low disk space.  Downloading disabled.\n";
					Shepherd::addMessageText( err, strlen(err), 180 ); //3 minutes

					thread::sleep( get_system_time() + posix_time::seconds(TIMEOUT) );
				
					boost::mutex::scoped_lock lockthis( s_DownloaderMutex );

					deleteCached( 0, 0 );
					deleteCached( 0, 1 );
				}
			}
			else
			{
				best_anim_old = -1;
				std::string best_anim_old_url;
				best_ctime_old = 0;
				best_rating_old = INT_MAX;

				//	Reset the generation number.
				setCurrentGeneration( -1 );

				//	Get the sheep list from the server.
				if( getSheepList() )
				{
					Shepherd::setDownloadState("Searching for sheep to download...");
					
					if (fListDirty)
					{
						//clearFlocks();
						parseSheepList();
					}
					else 
					{
						fGotList = true;
					}

					do
					{
						best_rating = INT_MIN;
						best_ctime = 0;
						best_anim = -1;

						updateCachedSheep();

						boost::mutex::scoped_lock lockthis( s_DownloaderMutex );

						unsigned int i;
						unsigned int j;
						
						size_t downloadedcount = 0;
						for( i=0; i<fServerFlock.size(); i++ )
						{
							//	Iterate the client flock to see if it allready exists in the cache.
							for( j=0; j<fClientFlock.size(); j++ )
							{
								if( (fServerFlock[i]->id() == fClientFlock[j]->id()) && (fServerFlock[i]->generation() == fClientFlock[j]->generation()) )
								{
									downloadedcount++;
									break;
								}
							}
						}
						//	Iterate the server flock to find the next sheep to download.
						for( i=0; i<fServerFlock.size(); i++ )
						{
							//	Iterate the client flock to see if it allready exists in the cache.
							for( j=0; j<fClientFlock.size(); j++ )
							{
								if( (fServerFlock[i]->id() == fClientFlock[j]->id()) && (fServerFlock[i]->generation() == fClientFlock[j]->generation()) )
									break;
							}

							//	If it is not found and the cache is ok to store than check if the file should be downloaded based on rating and server file write time.
							if( (j == fClientFlock.size()) && !cacheOverflow((double)fServerFlock[i]->fileSize(), fServerFlock[i]->getGenerationType()) )
							{
								//	Check if it is the best file to download.
								if( (best_ctime == 0 && best_ctime_old == 0) ||
									(fServerFlock[i]->rating() > best_rating && fServerFlock[i]->rating() <= best_rating_old) ||
									(fServerFlock[i]->rating() == best_rating && fServerFlock[i]->fileWriteTime() < best_ctime ) )
									{
										if ( fServerFlock[i]->rating() != best_rating_old ||
											(fServerFlock[i]->rating() == best_rating_old  && fServerFlock[i]->fileWriteTime() > best_ctime_old) )
										{
											best_rating = fServerFlock[i]->rating();
											best_ctime = fServerFlock[i]->fileWriteTime();
											best_anim = i;
										}
									}
							}
						}

						//	Found a valid sheep so download it.
						if( best_anim != -1 )
						{
							best_ctime_old = best_ctime;
							best_rating_old = best_rating;
							//	Make enough room in the cache for it.
							deleteCached( fServerFlock[ best_anim ]->fileSize(), fServerFlock[ best_anim ]->getGenerationType() );

							std::stringstream downloadingsheepstr;
							downloadingsheepstr << "Downloading sheep " << downloadedcount+1 << "/" << fServerFlock.size() << "...\n";
							Shepherd::setDownloadState(downloadingsheepstr.str() + fServerFlock[ best_anim ]->URL());
						
							g_Log->Info( "Best sheep to download rating=%d, fServerFlock index=%d, write time=%s", best_rating, best_anim, ctime(&best_ctime) );
							if( downloadSheep( fServerFlock[ best_anim ] ) )
							{
								failureSleepDuration = 0;
								badSheepSleepDuration = TIMEOUT;
							} else
							{
								best_anim_old = best_anim;
								best_anim_old_url = fServerFlock[ best_anim_old ]->URL();
							}
						}
						this_thread::interruption_point();
					} while (best_anim != -1);

					if (best_anim_old == -1)
					{
						failureSleepDuration = badSheepSleepDuration;
							
						badSheepSleepDuration = Base::Math::Clamped( badSheepSleepDuration * 2, TIMEOUT, MAX_TIMEOUT );

						std::stringstream tmp;
			
						tmp << "All available sheep downloaded, will retry in {" << std::fixed << std::setprecision(0) << failureSleepDuration << "}...";

						Shepherd::setDownloadState(tmp.str());
					}
					else
					{
							//	Gradually increase duration betwee 10 seconds and TIMEOUT, if the sheep fail to download consecutively
							failureSleepDuration = badSheepSleepDuration;
						
							badSheepSleepDuration = Base::Math::Clamped( badSheepSleepDuration * 2, TIMEOUT, MAX_TIMEOUT );
						
							std::stringstream tmp;
			
							tmp << "Downloading failed, will retry in {" << std::fixed << std::setprecision(0) << failureSleepDuration << "}...\n" << best_anim_old_url;
							Shepherd::setDownloadState(tmp.str());
					}
				}
				else
				{
					//	Error connecting to server so timeout then try again.
				        const char *err = "error connecting to server";
					Shepherd::addMessageText( err, strlen(err), 180 );	//	3 minutes.
					failureSleepDuration = TIMEOUT;
					
					badSheepSleepDuration = 10;
				}

				thread::sleep( get_system_time() + posix_time::seconds(failureSleepDuration) );
				
				failureSleepDuration = TIMEOUT;

			}
		}
	}
	catch(thread_interrupted const&)
	{
	}
}

/*
	getSheepList().
	This method will download the sheep list and uncompress it.
*/
bool	SheepDownloader::getSheepList()
{
	const char *xmlPath = Shepherd::xmlPath();

	char filename[ MAX_PATH ];
    snprintf( filename, MAX_PATH, "%slist_%s.xml", xmlPath, Shepherd::role() );

	struct stat stat_buf;
	if( -1 != stat( filename, &stat_buf) )
	{
		if( time(0) - stat_buf.st_mtime < MIN_READ_INTERVAL )
			return true;
	}
	
	Shepherd::setDownloadState("Getting sheep list...");

    snprintf( filename, MAX_PATH, "%slist.gzip", xmlPath );

	//	Create the url for getting the cp file to create the frame
	char 	url[ MAXBUF*5 ];
    snprintf( url, MAXBUF*5, "%scgi/list?v=%s&u=%s",	ContentDownloader::Shepherd::serverName(),
																CLIENT_VERSION,
																Shepherd::uniqueID() );

	Network::spCFileDownloader spDownload = new Network::CFileDownloader( "Sheep list" );
	if( !spDownload->Perform( url ) )
	{
		if( spDownload->ResponseCode() == 304 )	//	"Not Modified"
			return true;

		if( spDownload->ResponseCode() == 401 )
			g_ContentDownloader().ServerFallback();

		g_Log->Error( "Failed to download %s.\n", url );
		return false;
	}

	//	Save the data to file.
    if( !spDownload->Save( filename ) )
    {
    	g_Log->Error( "Unable to save %s\n", filename );
    	return false;
    }

	//	Save file time.
	time( &fLastListTime );

	//	Open the file to write the uncompressed xml data.
	snprintf( filename, MAX_PATH, "%slist_%s.xml", xmlPath, Shepherd::role() );
	FILE *outXML = fopen( filename, "wb" );
	if( outXML == NULL )
		return false;

	//	Reset the current generation
	//setCurrentGeneration(-1);

	//	Gzopen the compressed file to uncompress.
	snprintf( filename, MAX_PATH, "%slist.gzip", xmlPath );
	gzFile gzinF = gzopen( filename, "rb" );
	if( gzinF == NULL )
	{
		g_Log->Error( "Unable to open %s", filename );
		fclose( outXML );
		return false;
	}

	//	Uncompress the data.
	char	buf[ MAXBUF ];
	int numBytes = 0;
	do
	{
		numBytes = gzread( gzinF, buf, 250 );
		if (numBytes <= 0)
            break;
		fwrite( buf, numBytes, 1, outXML );
	} while( !gzeof( gzinF ) );

	//	Close the input and output file.
	gzclose( gzinF );
	fclose( outXML );

	fListDirty = true;

	//	Delete the temp file with the compressed data.
	snprintf( filename, MAX_PATH, "%slist.gzip", xmlPath );
	remove( filename );

	return true;
}

/*
*/
bool	SheepDownloader::sheepOnServer( Sheep *sheep )
{
 	boost::mutex::scoped_lock lockthis( s_DownloaderMutex );

	if( fGotList == true )
	{
		SheepArray::iterator is = fServerFlock.begin();
		for( ; is != fServerFlock.end(); ++is )
		{
			if( !(*is)->id() == sheep->id() )
				return true;
		}
	}
	else
		return true;

	return false;
}

/*
	shepherdCallback().
	This method is also in charge of downling the sheep in their transitional order.
*/
void SheepDownloader::shepherdCallback( void *data )
{
	((SheepDownloader *)data)->findSheepToDownload();
}

};
