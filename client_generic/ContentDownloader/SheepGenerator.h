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
#ifndef _SHEEPGENERATOR_H_
#define _SHEEPGENERATOR_H_

#if 0//DASVO_TEST
#include "expat.h"
#endif
#include "base.h"
#include "ProcessForker.h"
#include "tinyxml.h"


namespace ContentDownloader
{

class SheepUploader;

/*
	SheepGenerator.
	This class is responsible for creating sheep frames to be uploaded to the server.
*/
class SheepGenerator
{
	public:

	static char		*fNickName;
	static char		*fURL;

	Base::spCProcessForker	m_spFlam;
	
	static boost::mutex	s_GeneratorMutex;
	static boost::mutex	s_NickNameMutex;
	

	char            *fTempFile;
	bool			fHasMessage;
	uint32				fGeneratorId;
	int32			m_DelayAfterRenderSec;

	protected:
		//	Gets the control point file from the server.
		bool getControlPoints( SheepUploader *uploader );

		//	Generates a sheep from the control point file that was last downloaded.
		bool generateSheep();

	public:
			SheepGenerator();
			virtual ~SheepGenerator();

			static void shepherdCallback( void* data );

			//	Sets/gets the nickname to be used with the sheep server.
			static void setNickName(const char *nick);
			static const char *nickName();

			//	Sets/gets the url to be displayed with the nickname on the server
			static void setURL( const char *url );
			static const char *URL();

#if 1//DASVO_TEST
			void handleGetElement(TiXmlElement* getElement, SheepUploader *uploader);
#else
			//	Functions to parse the XML structures.
			static void getStartElement( void *userData, const char *name, const char **atts );
			static void characterHandler( void *userData, const XML_Char *s, int len );
			static void getEndElement( void *userData, const char *name );
#endif

			// Sets/gets whether or not to play a beep after a sheep frame has been generated.
			static void setPlayBeep( const int &playBeep );
			static int playBeep();

			//	Sets/gets generator thread ID.
			void setGeneratorId(const uint32 id) { fGeneratorId = id; }
			uint32 generatorId() const { return fGeneratorId; }
			
			void Abort( void );

			friend class Shepherd;

			// Initialize global data used by the generators.
			static void initializeGenerator();

			// Cleans up any global data used by the generators.
			static void closeGenerator();
};

};
#endif
