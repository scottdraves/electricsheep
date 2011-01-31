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
#include "libtorrent/torrent_handle.hpp"

#include "Torrent.h"
#include "Shepherd.h"
#include "SheepDownloader.h"

namespace ContentDownloader
{

// declare namespace we are using
//
using namespace std;
using namespace libtorrent;

Torrent::Torrent()
//
// Description:
//		Default constrictor. Initialize all class data.
//
{
	fFileSize = 0;
	fURL = NULL;
	fFileName = NULL;
	fGeneration = -1;
	fDownloaded = false;
	fDeleted = false;
	fActive = false;
	fGotHandle = false;
	fInSeeding = false;
	fSingleDownload = -1;
	fPurged = false;
	fSeedQueue = false;
	fDownloadQueue = false;
	fReannounceCounter = 0;
	fNextReannounce = 1;
	fTorrentState = none;
}

Torrent::Torrent( Torrent &torrent )
//
// Description:
//		Copy constrictor. copies all class data.
//
{
	setId(torrent.fTorrentId);
	setURL(torrent.fURL);
	setFileName(torrent.fFileName);
	fGeneration = torrent.fGeneration;
	fFileSize = torrent.fFileSize;
	fDownloaded = torrent.fDownloaded;
	fDeleted = torrent.fDeleted;
	fActive = torrent.fActive;
	fGotHandle = torrent.fGotHandle;
	fInSeeding = torrent.fInSeeding;
	fSingleDownload = torrent.fSingleDownload;
	fPurged = torrent.fPurged;
	fSeedQueue = torrent.fSeedQueue;
	fDownloadQueue = torrent.fDownloadQueue;
	fTorrentHandle = torrent.fTorrentHandle;
	fReannounceCounter = torrent.fReannounceCounter;
	fNextReannounce = torrent.fNextReannounce;
	fTorrentState = torrent.fTorrentState;
}

Torrent::~Torrent()
//
// Description:
//		Destructor. Cleans up any alocated data.
//
{
	if(fURL != NULL)
	{
		delete [] fURL;
		fURL = NULL;
	}

	if(fFileName != NULL)
	{
		delete [] fFileName;
		fFileName = NULL;
	}
}

bool
Torrent::isSeeding()
//
// Description:
//		Returns true if torrent is seeding
//
{
	if (fTorrentHandle.is_valid())	{
		libtorrent::torrent_status status=fTorrentHandle.status();
		if(status.state == libtorrent::torrent_status::seeding)
			return true;
		else
			return false;
	}
	else
		return false;
}

void
Torrent::moveSeeding()
// Description:
//		Moves the finished torrent from /torrent to /mpeg/<generation>
//
{
	if (fTorrentHandle.is_valid())	{
		char file[255];
		if (generation() == -1)
			setGeneration(SheepDownloader::currentGeneration());
#ifndef LINUX_GNU
		sprintf(file,"%s%i\\",Shepherd::mpegPath(),generation());
		CreateDirectory(file, NULL);
#else
		sprintf(file,"%s%i",Shepherd::mpegPath(),generation());
		mkdir(file, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#endif
		fTorrentHandle.pause();
		fTorrentHandle.move_storage(file);
		fTorrentHandle.resume();
		fInSeeding = true;
	}
}

void
Torrent::setURL(const char *url)
//
// Description:
//		Sets the URL for this torrent.
//
{
	if(fURL == NULL)
	{
		delete [] fURL;
		fURL = NULL;
	}

	if(url)
	{
		fURL = new char[strlen(url) + 1];
		strcpy(fURL, url);
	}
}

void
Torrent::setFileName(const char *filename)
//
// Description:
//		Sets the filename of the torrent.
//
{
	if(fFileName == NULL)
	{
		delete [] fFileName;
		fFileName = NULL;
	}

	if(filename)
	{
		fFileName = new char[strlen(filename) + 1];
		strcpy(fFileName, filename);
	}
}

void
Torrent::setId(const char *id)
//
// Description:
//		Sets the filename of the sheep.
//
{
	if(fTorrentId == NULL)
	{
		delete [] fTorrentId;
		fTorrentId = NULL;
	}

	if(id)
	{
		fTorrentId = new char[strlen(id) + 1];
		strcpy(fTorrentId, id);
	}
}

void
Torrent::setActive( const bool &state )
{
	fActive = state;
	if (fActive == true && fTorrentHandle.is_valid())
	{
		fGotHandle = true;
	} else
	{
		fGotHandle = false;
	}
}

void
Torrent::resetSingleDownload()
{
	if (fTorrentHandle.is_valid())	{
		fSingleDownload = -1;
		const int num_files = fTorrentHandle.get_torrent_info().num_files();
		std::vector<bool> bitmask(num_files, false);
		fTorrentHandle.pause();
		fTorrentHandle.filter_files(bitmask);
		fTorrentHandle.resume();
	}
}

void
Torrent::setSingleDownload( const int &index )
{
	if (fTorrentHandle.is_valid())	{
		fSingleDownload = index;
		const int num_files = fTorrentHandle.get_torrent_info().num_files();
		std::vector<bool> bitmask(num_files, true);
		assert(index > 0 && index <= num_files);
		bitmask[index - 1] = false;
		fTorrentHandle.pause();
		fTorrentHandle.filter_files(bitmask);
		fTorrentHandle.resume();
	}
}

float
Torrent::getRatio(int place)
{
	if (fTorrentHandle.is_valid())	{
		libtorrent::torrent_status status=fTorrentHandle.status();
		float complete = (float)(status.num_complete+((30-place)/3));
		float incomplete = (float)status.num_incomplete;
		if (incomplete < 1)
			return 1000;
		else if (complete < 1)
			return -100;
		return complete/incomplete;
	}
	return 0;
}

libtorrent::torrent_status::state_t
Torrent::getStatus()
{
	if (fTorrentHandle.is_valid())
		return fTorrentHandle.status().state;
	else
		return libtorrent::torrent_status::queued_for_checking;
}

bool
Torrent::is_paused()
{
	if (fTorrentHandle.is_valid())
		if(fTorrentHandle.is_paused())
			return true;
		else
			return false;
	else
		return false;
}

};
