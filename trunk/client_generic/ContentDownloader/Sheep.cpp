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
#include <vector>
#include <time.h>
#include <string.h>

#include "Sheep.h"

namespace ContentDownloader
{

Sheep::Sheep()
//
// Description:
//		Default constrictor. Initialize all class data.
//
{
	fURL = NULL;
	fFileSize = 0;
	fFileName = NULL;
	fWriteTime = 0;
	fRating = 0;
	fDownloaded = false;
	fSheepId = 0;
	fFirst = 0;
	fLast = 0;
	fType = 0;
	fDeleted = false;
	fGeneration = -1;
	fIsTemp = false;
}

Sheep::Sheep( const Sheep &sheep )
//
// Description:
//		Copy constrictor. copies all class data.
//
{
	fURL = NULL;
	fFileName = NULL;
	setURL(sheep.fURL);
	setFileName(sheep.fFileName);
	fFileSize = sheep.fFileSize;
	fWriteTime = sheep.fWriteTime;
	fRating = sheep.fRating;
	fDownloaded = sheep.fDownloaded;
	fSheepId = sheep.fSheepId;
	fFirst = sheep.fFirst;
	fLast = sheep.fLast;
	fType = sheep.fType;
	fDeleted = sheep.fDeleted;
	fGeneration = sheep.fGeneration;
	fIsTemp = sheep.fIsTemp;
}

Sheep::~Sheep()
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

void
Sheep::setURL(const char *url)
//
// Description:
//		Sets the URL for this sheep.
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
Sheep::setFileName(const char *filename)
//
// Description:
//		Sets the filename of the sheep.
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

};
