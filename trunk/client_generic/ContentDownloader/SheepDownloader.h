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
#ifndef _SHEEPDOWNLOADER_H_
#define _SHEEPDOWNLOADER_H_

#if 1//DASVO_TEST
#include "tinyxml.h"
#else
#include "expat.h"
#endif
#include "Sheep.h"
#include "Networking.h"

namespace ContentDownloader
{
class SheepRenderer;

/*

	This class is responsible for downloading and queueing new sheep to the renderer..
*/
class SheepDownloader
{
	// number of sheep that the downloader has downloaded
	static int fDownloadedSheep;

	// sheep flocks
	SheepArray fServerFlock;
	SheepArray fClientFlock;
	SheepRenderer *fRenderer;

	// boolean for message checks
	bool fHasMessage;
	static int fCurrentGeneration;

	static time_t fLastListTime;
	
	static boost::mutex	s_DownloaderMutex;
	
	bool m_bAborted;
	
	Network::spCFileDownloader m_spSheepDownloader;
	
	boost::mutex m_AbortMutex;
	
	protected:

		//	Downloads the given sheep and queues it up for rendering.
		bool downloadSheep( Sheep *sheep );

		//	Function to parse the cache and find a sheep to download.
		void findSheepToDownload();

		//	Ipdate the cached sheep and make sure there is enough room for a second sheep.
		void updateCachedSheep();

		//	Clears the flock list being maintained
		void clearFlocks();

		//	Delete enough sheep to clear enough room for the given amount of bytes.
		void deleteCached( const int &bytes, const int getGenerationType );

		bool isFolderAccessible( const char *folder );

		//	This methods parses the sheep list and intializes the array of server sheep.
		void parseSheepList();

#if 1//DASVO_TEST
		void handleListElement(TiXmlElement* listElement);
#else
		//	Functions to parse the XML structures.
		static void listStartElement( void *userData, const char *name, const char **atts );
		static void characterHandler( void *userData, const char *s, int len );
		static void getEndElement( void *userData, const char *name );
#endif

		//	Message retrival from server.
		void setHasMessage(const bool &hasMessage) { fHasMessage = hasMessage; }
		bool hasMessage() const { return fHasMessage; }

		void setCurrentGeneration(const int &generation) { fCurrentGeneration = generation; }

		//	Checks the disk space to make sure the cache is not being overflowed.
		int cacheOverflow(const double &bytes, const int getGenerationType) const;

		// Clean global and static data for the downloader threads.
		static void closeDownloader();

		//	Function to initialize the downloader threads
		static void initializeDownloader();

		//	Returns true if the sheep is still on the server.
		bool sheepOnServer(Sheep *sheep);
		void deleteSheep(Sheep *sheep);

		static bool fGotList;
		
		static bool fListDirty;

	public:
			SheepDownloader();
			virtual ~SheepDownloader();

			static void shepherdCallback(void* data);

			static int numberOfDownloadedSheep();

			static int currentGeneration() { return fCurrentGeneration; }

			static bool getSheepList();


			// add to the number of downloaded sheep (called by torrent)
			static void addDownloadedSheep(int sheep) { fDownloadedSheep += sheep; }

			void deleteSheepId(int sheepId);

			// Aborts the working thread
			void Abort( void );

			//	Declare friend classes for protected data accesss.
			friend class Shepherd;
};

};

#endif
