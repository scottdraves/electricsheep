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
#ifndef __TORRENT_H_
#define __TORRENT_H_

#include "libtorrent/torrent_handle.hpp"

namespace ContentDownloader
{

//
// Description:
//		This class will represent a torrent as it exists
// on the server or on the client. It has methods to
// get and set different aspects of a torrent
//
class Torrent
{
public:

	enum torrentstate_t
	{
		none,
		queued_seed,
		queued_download,
		seed,
		download
	};


	// Default constructor
	Torrent();

	// Copy Constructor
	Torrent( Torrent &torrent );

	// Destructor
	~Torrent();

	// sets the URL that this torrent lives at
	void setURL(const char *url);

	// returns the URL that the torrent lives at
	const char *URL() const { return fURL; }

	// Sets the torrent file size on the server
	void setFileSize(const long &size) { fFileSize = size; }

	// gets the torrent file size
	long fileSize() const { return fFileSize; }

	// Sets the torrent file name
	void setFileName(const char *name);

	// gets the torrent file name
	const char *fileName() const { return fFileName; }

	// sets the torrent id
	void setId(const char *id);

	// gets the torrent id
	char *const id() const { return fTorrentId; }

	// sets whether or not the torrent has been downloaded
	void setDownloaded( const bool &state ) { fDownloaded = state; }

	// returns if the torrent has been downloaded
	bool downloaded() const { return fDownloaded; }

	// sets whether or not the torrent has been added to the session
	void setActive( const bool &state );

	// returns if the torrent has been added to the session
	bool active() const { return fActive; }

	// sets whether or not the torrent has been removed from the server
	void setDeleted( const bool &state ) { fDeleted = state; }

	// returns if the torrent has been removed from the server
	bool deleted() const { return fDeleted; }

	// set the torrent generation
	void setGeneration( const int &gen ) { fGeneration = gen; }

	// returns the torrent generation
	int generation() const { return fGeneration; }

	// returns true if this torrent is just seeding
	bool isSeeding();

	// move finished files to mpg dir
	void moveSeeding();

	void setInSeeding( const bool &state ) { fInSeeding = state; }
	bool inSeeding() { return fInSeeding; }

	void setPurged( const bool &state ) { fPurged = state; }
	bool purged() { return fPurged; }

	void stopSeeding() { fInSeeding = false; }

	// handle single sheep downloads
	void setSingleDownload( const int &index);
	int singleDownload() { return fSingleDownload; }
	void resetSingleDownload();

	void setSeedQueue( const bool &state ) { fSeedQueue = state; }
	bool seedQueue() { return fSeedQueue; }
	void setDownloadQueue( const bool &state ) { fDownloadQueue = state; }
	bool downloadQueue() { return fDownloadQueue; }
	float getRatio(int place);

	bool gotHandle() {return fGotHandle; }

	bool is_paused();

	libtorrent::torrent_status::state_t getStatus();

	libtorrent::torrent_info getInfo() {return fTorrentHandle.get_torrent_info(); }

	libtorrent::torrent_handle handle() { return fTorrentHandle; }

	libtorrent::torrent_handle fTorrentHandle;

	int fReannounceCounter;
	int fNextReannounce;

	void setTorrentState( const torrentstate_t &state ) { fTorrentState = state; }
	torrentstate_t torrentState() { return fTorrentState; }

private:
	// private memeber data
	//
	char		*fURL;
	char		*fFileName;
	char		*fTorrentId;
	bool		fDownloaded;
	int			fGeneration;
	bool		fActive;
	bool		fDeleted;
	bool		fGotHandle;
	bool		fInSeeding;
	int			fFileSize;
	int			fSingleDownload;
	bool		fPurged;
	bool		fSeedQueue;
	bool		fDownloadQueue;

	torrentstate_t	fTorrentState;
};

// This is a convienince for defining an array
// of torrents using stl::vector
//
typedef std::vector<Torrent *> TorrentArray;

};

#endif
