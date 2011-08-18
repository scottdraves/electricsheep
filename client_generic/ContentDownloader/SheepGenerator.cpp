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
#include <shellapi.h>
#endif
#include <time.h>
#include <zlib.h>
#include <string>
#include <vector>
#include <list>
#include <iomanip>

#include "base.h"
#include "MathBase.h"
#include "Log.h"
#include "Timer.h"
#include "clientversion.h"
#include "Networking.h"
#include "Timer.h"
#include "ContentDownloader.h"
#include "SheepGenerator.h"
#include "Shepherd.h"
#include "SheepUploader.h"
#include "ProcessForker.h"
#include "Settings.h"
#if defined(WIN32) && defined(_MSC_VER)
#include "../msvc/msvc_fix.h"
#endif

namespace ContentDownloader
{

using namespace boost;

static const uint32 MAXBUF = 1024;
static const uint32 TIMEOUT = 600;
static const uint32 MAX_TIMEOUT = 24*60*60U; // 1 day
static const uint32 INIT_DELAY = 600;

//	Initialize the static class data.
char *SheepGenerator::fNickName = NULL;
char *SheepGenerator::fURL = NULL;

//	Initialize any global data.
boost::mutex SheepGenerator::s_GeneratorMutex;

boost::mutex SheepGenerator::s_NickNameMutex;

//
static void encode( char *dst, const char *src )
{
    static const char *hex = "0123456789ABCDEF";
    char t;
    while( t = *src++ )
    {
        if( isalnum(t) )	*dst++ = t;
        else
        {
            *dst++ = '%';
            *dst++ = hex[(t >> 4) & 15];
            *dst++ = hex[t & 15];
        }
    }

	*dst = '\0';
}

/*
*/
SheepGenerator::SheepGenerator()
{
	fHasMessage = false;
	fGeneratorId = -1;
	fTempFile = NULL;
	m_DelayAfterRenderSec = g_Settings()->Get( "settings.generator.DelayAfterRenderSeconds", 1 );
}

/*
*/
SheepGenerator::~SheepGenerator()
{
	g_Log->Info( "~SheepGenerator()..." );

	SAFE_DELETE_ARRAY( fNickName );
	SAFE_DELETE_ARRAY( fURL );

	if( fTempFile )
		remove( fTempFile );

	//SAFE_DELETE(m_spFlam);

	//m_spFlam = NULL;
}

/*
*/
void	SheepGenerator::initializeGenerator()
{
}

/*
*/
void	SheepGenerator::closeGenerator()
{
	g_Log->Info( "Closing generator..." );
}

/*
*/
void	SheepGenerator::Abort()
{
	if ( !m_spFlam.IsNull() )
		m_spFlam->Terminate();
}


/*
*/
void	SheepGenerator::setNickName( const char *nick )
{
 	boost::mutex::scoped_lock lockthis( s_NickNameMutex );

	SAFE_DELETE_ARRAY( fNickName );
	fNickName = new char[ strlen(nick) + 1 ];
	strcpy( fNickName, nick );
}

/*
*/
const char	*SheepGenerator::nickName()
{
 	boost::mutex::scoped_lock lockthis( s_NickNameMutex );

	return fNickName;
}

/*
*/
void	SheepGenerator::setURL( const char *url )
{
 	boost::mutex::scoped_lock lockthis( s_GeneratorMutex );

	SAFE_DELETE_ARRAY( fURL );
	size_t len = strlen( url );
	fURL = new char[ len + 1 ];
	strncpy( fURL, url, len + 1 );
}

/*
*/
const char	*SheepGenerator::URL()
{
 	boost::mutex::scoped_lock lockthis( s_GeneratorMutex );

	return fURL;
}

#if 1//DASVO_TEST
//
void SheepGenerator::handleGetElement(TiXmlElement* getElement, SheepUploader *uploader)
{
	uploader->setHasMessage( false );

	if ( strcmp( "get", getElement->Value() ) != 0 )
		return;

	const char *a;
	if (a = getElement->Attribute("gen"))		uploader->setSheepGeneration( atoi(a) );
	if (a = getElement->Attribute("id"))		uploader->setSheepID( atoi(a) );
	if (a = getElement->Attribute("job"))		uploader->setSheepJob( atoi(a) );
	if (a = getElement->Attribute("time"))		uploader->setSheepTime( atoi(a) );
	if (a = getElement->Attribute("frame"))
	{
		g_Log->Info( "frame: %u", atoi(a) );
		uploader->setSheepTime(atoi(a));
		uploader->setSheepJob(atoi(a));
	}

	TiXmlElement* pChildNode;
	for ( pChildNode = getElement->FirstChildElement(); pChildNode; pChildNode = pChildNode->NextSiblingElement() )
	{
		const char* name = pChildNode->Value();

		if( !strcmp( "message", name ) )
		{
			uploader->setHasMessage( true );

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
				strncpy( server_error_type, a, MAXBUF );

				Shepherd::addMessageText( server_error_type, strlen( server_error_type ), 180 ); //	3 minutes
			}
		}
	}
}
#else
/*
	getStartElement().
	This method will parse the start element of the control point file to get information specific to the frame.
*/
void	SheepGenerator::getStartElement( void *userData, const char *name, const char **atts )
{
	SheepUploader *uploader = (SheepUploader *)userData;
    int i = 0;

    if( !strcmp( "get", name ) )
	{
		while( atts[i] != NULL)
		{
			const char *a = atts[i+1];

			if( !strcmp(atts[i], "gen") )		uploader->setSheepGeneration( atoi(a) );
			else if( !strcmp(atts[i], "id") )	uploader->setSheepID( atoi(a) );
			else if( !strcmp(atts[i], "job") )	uploader->setSheepJob( atoi(a) );
			else if( !strcmp(atts[i], "time") )	uploader->setSheepTime( atoi(a) );
			else if( !strcmp(atts[i], "frame") )
			{
				g_Log->Info( "frame: %u", atoi(a) );
				uploader->setSheepTime(atoi(a));
				uploader->setSheepJob(atoi(a));
			}

			i += 2;
		}
    }
	else if (!strcmp( "message", name ))
	{
		uploader->setHasMessage( true );
    }
	else if( !strcmp( "error", name ) )
	{
		char servererrortype[ ContentDownloader::MAXBUF ];
		while( atts[i] )
		{
			if( !strcmp(atts[i], "type") )
			{
				strncpy( servererrortype, atts[i+1], ContentDownloader::MAXBUF );
				ContentDownloader::Shepherd::addMessageText( servererrortype, strlen(servererrortype), 180 ); //3 minutes
			}
			i += 2;
		}
    }
}

/*
	getEndElement().
	This method gets the end element of the control point file.
*/
void	SheepGenerator::getEndElement( void *userData, const char *name )
{
	SheepUploader *uploader = (SheepUploader *)userData;
    if( !strcmp( "message", name ) )
		uploader->setHasMessage( false );
}

/*
	characterHandler().
	Character handler for the parser.
*/
void	SheepGenerator::characterHandler( void *userData, const XML_Char *s, int len )
{
	SheepUploader *uploader = (SheepUploader *)userData;
    if( uploader->hasMessage() )
		ContentDownloader::Shepherd::addMessageText( s, len, 30 ); //	5 minutes.
}
#endif

/*
	getControlPoints().
	This method downloads, uncompreses and parses the control point file need to create the frame for the sheep.
*/
bool	SheepGenerator::getControlPoints( SheepUploader *uploader )
{
	//	Encode the nickname & user-url since they're part of the request.
	std::string	nickEncoded = Network::CManager::Encode( SheepGenerator::nickName() );
	std::string	urlEncoded = Network::CManager::Encode( SheepGenerator::URL() );

	//	Create the url for getting the cp file to create the frame
	char 	url[ MAXBUF*5 ];
    snprintf( url, MAXBUF*5, "%scgi/get?n=%s&w=%s&v=%s&u=%s&r=%.3g&c=%.3g", ContentDownloader::Shepherd::serverName(),
																					nickEncoded.c_str(),
																					urlEncoded.c_str(),
																					CLIENT_VERSION,
																					Shepherd::uniqueID(),
																					1.0,
																					1.0 );

	char tmp[ 128 ];
	snprintf( tmp, 128, "Controlpoints for generator #%d", fGeneratorId );
	Network::spCFileDownloader spDownload = new Network::CFileDownloader( tmp );
	if( !spDownload->Perform( url ) )
	{
		g_Log->Error( "Failed to download %s.\n", url );

		if( spDownload->ResponseCode() == 401 )
			g_ContentDownloader().ServerFallback();

		return false;
	}

	const char *xmlPath = Shepherd::xmlPath();

	//	Save the data to file.
	char filename[ MAX_PATH ];
    snprintf( filename, MAX_PATH, "%scp_%u.gzip", xmlPath, fGeneratorId );
    if( !spDownload->Save( filename ) )
    {
    	g_Log->Error( "Unable to save %s\n", filename );
    	return false;
    }

	//	Open the input and output files for decompression.
	snprintf( filename, MAX_PATH, "%scp_%u.xml", xmlPath, fGeneratorId );
	FILE *outXML = fopen( filename, "w" );
	if( outXML == NULL )
	{
		g_Log->Error( "Unable to open %s", filename );
		return false;
	}


	snprintf( filename, MAX_PATH, "%scp_%u.gzip", xmlPath, fGeneratorId );
	gzFile gzinF = gzopen( filename, "rb" );
	if( gzinF == NULL )
	{
		g_Log->Error( "Unable to open %s", filename );
		fclose( outXML );
		return false;
	}

	//	Decompress the file into the xml file.
	char	buf[ MAXBUF ];
	int numBytes = 0;
	do
	{
		numBytes = gzread( gzinF, buf, 250 );
		if (numBytes <= 0)
            break;
		fwrite( buf, numBytes, 1, outXML );
	} while( !gzeof( gzinF ) );

	fclose( outXML );
	gzclose( gzinF );

	//	Delete the temp file.
	snprintf( filename, MAX_PATH, "%scp_%u.gzip", xmlPath, fGeneratorId );
	remove( filename );

	snprintf( filename, MAX_PATH, "%scp_%u.xml", xmlPath, fGeneratorId );
#if 1//DASVO_TEST
	TiXmlDocument doc(filename);
	if (doc.LoadFile())
	{
		TiXmlHandle hDoc(&doc);
		TiXmlElement* getElement;

		getElement=hDoc.FirstChild("get").Element();

		handleGetElement(getElement, uploader);
	}
	else
	{
		fprintf( stderr, "%s at line %d\n", doc.ErrorDesc(), doc.ErrorRow() );
	}
#else
	FILE *inXML = fopen( filename, "r" );
	if( inXML == NULL )
	{
		g_Log->Error( "Unable to open %s", filename );
		return false;
	}

	//	Parse the xml file.
	XML_Parser parser;
    parser = XML_ParserCreate( NULL );
    XML_SetElementHandler( parser, getStartElement, getEndElement );
    XML_SetCharacterDataHandler( parser, characterHandler );
	XML_SetUserData( parser, uploader );

	int done = 0;

	do
	{
		size_t len = fread( buf, 1, MAXBUF, inXML );
		done = len < MAXBUF;
		if( len == 0 )	//	Lost contact, no data to parse.
			break;

		if( !XML_Parse( parser, buf, len, done ) )
		{
			g_Log->Error( "%s at line %d\n", XML_ErrorString( XML_GetErrorCode( parser ) ), XML_GetCurrentLineNumber( parser ) );
			break;
		}
	} while( !done );

	XML_ParserFree( parser );

	fclose( inXML );
#endif


	return true;
}

/*
*/
bool	SheepGenerator::generateSheep()
{
	char	cpf[ MAX_PATH ];
	char	jpf[ MAX_PATH ];

	try
	{
#ifdef WIN32
		std::string forkee = g_Settings()->Get( "settings.app.InstallDir", std::string(".\\") ) + "flam3-animate.exe";
#else
#ifndef LINUX_GNU
		std::string forkee = g_Settings()->Get( "settings.app.InstallDir", std::string("./") ) + "flam3-animate";
#else
		/* linux should find it in the user's $PATH */
		char fullpath[MAX_PATH];
		FILE *fp = popen( "which flam3-animate", "r" );
		char *ret = fgets( fullpath, MAX_PATH, fp );
		pclose( fp );

		if ( strlen ( fullpath ) > 0 )
		  /* remove terminating newline */
		  memset( fullpath + strlen( fullpath ) - 1, 0 , 1 );

		std::string forkee( fullpath );
#endif
#endif

#ifndef	DEBUG
		std::stringstream tmp;
				
		tmp << "Rendering starts in {" << std::fixed << std::setprecision(0) << (int32)ContentDownloader::INIT_DELAY << "}...";

		Shepherd::setRenderState(tmp.str());
		//	Make sure we are really deeply settled asleep, avoids lots of timed out frames.
		g_Log->Info( "Chilling for %d seconds before trying to render frames...", ContentDownloader::INIT_DELAY );
		
		thread::sleep( get_system_time() + posix_time::seconds(ContentDownloader::INIT_DELAY) );
#endif

		uint32	failureSleepDuration = TIMEOUT;
		uint32	noWorkSleepDuration = 0;
		Base::CTimer	timer;

		while( true )
		{
			this_thread::interruption_point();

			SheepUploader *uploader = new SheepUploader();
			Shepherd::setRenderState( "Requesting control points from the server..." );
			//	Get the control points for the frame.
			bool allowRender = Shepherd::RenderingAllowed();
			if( getControlPoints( uploader ) && allowRender)
			{
				bool hasWork = true;
				Shepherd::setRenderState( "Processing received control points..." );
				//	We got data from server, reset failure delay.
				failureSleepDuration = TIMEOUT;

				//	Create the filenames to send to the flame generator.
				snprintf( cpf, MAX_PATH, "%scp_%u.xml", ContentDownloader::Shepherd::xmlPath(), fGeneratorId );

				//fp8 starttime = timer.Time();

				//	Parse the xml file to get env-vars.
				TiXmlDocument *pDoc = new TiXmlDocument;

				uint32	sleeptime = TIMEOUT;
				if( pDoc )
				{
					if( pDoc->LoadFile( cpf ) )
					{
						TiXmlNode *pNode = pDoc->FirstChild( "get" );
						if( pNode )
						{
							TiXmlNode *pArgs = pNode->FirstChild( "args" );
							if( pArgs )
							{
								//	Create process...
								std::string fmt = "tmp";
								m_spFlam = new Base::CProcessForker( forkee.c_str() );

								TiXmlElement *pElement = pArgs->ToElement();
								for( TiXmlAttribute *pAttribute = pElement->FirstAttribute(); pAttribute; pAttribute = pAttribute->Next() )
								{
									if( std::string( pAttribute->Name() ) == "format" )
										fmt = pAttribute->Value();

									m_spFlam->PushEnv( pAttribute->Name(), pAttribute->Value() );
								}

								snprintf( jpf, MAX_PATH, "%ssheep_%d_%d_%d.%s", ContentDownloader::Shepherd::jpegPath(), uploader->sheepGeneration(), uploader->sheepID(), uploader->sheepJob(), fmt.c_str() );
#ifdef WIN32
								
								fTempFile = _strdup(jpf);
#else
								fTempFile = strdup(jpf);
#endif

								//	Fork flam process, throws if failed.
								m_spFlam->PushEnv( "verbose", "0" );
								m_spFlam->PushEnv( "in", cpf );
								m_spFlam->PushEnv( "out", jpf );
								m_spFlam->PushEnv( "nthreads", "1" );
								m_spFlam->Execute();

								Shepherd::FrameStarted();
								m_spFlam->Wait();

								//if the sheep's file really exists, mark it as completed
								FILE *f = fopen( jpf, "rb" );
								if ( f )
								{
									Shepherd::FrameCompleted();
									fclose( f );
								}
								
								thread::sleep( get_system_time() + posix_time::seconds(m_DelayAfterRenderSec) );

								uploader->setSheepFile( jpf );
								if( uploader->uploadSheep() )
                                    sleeptime = 0;
								else
                                    sleeptime = TIMEOUT;

								SAFE_DELETE( uploader );

								if( !ContentDownloader::Shepherd::saveFrames() )
								{
									if (remove( jpf ) != 0)
										g_Log->Warning( "Failed to remove %s", jpf);
								}

								free( fTempFile );
								fTempFile = NULL;
							}
							else
							{
								hasWork = false;
								noWorkSleepDuration = Base::Math::Clamped( noWorkSleepDuration * 2, TIMEOUT, MAX_TIMEOUT );
								//	No args node, check for "retry" & message...
								TiXmlElement *pElement = pNode->ToElement();
								for( TiXmlAttribute *pAttribute = pElement->FirstAttribute(); pAttribute; pAttribute = pAttribute->Next() )
								{
									if( std::string( pAttribute->Name() ) == "retry" )
									{
										sleeptime = atoi( pAttribute->Value() );
										sleeptime = 0; // ignore value from server
										g_Log->Info( "Retry in %dsec", sleeptime );
									}
								}

								pNode = pNode->FirstChild( "message" );
								if( pNode )
								{
									pNode = pNode->FirstChild();
									if( pNode )
									{
										TiXmlText	*pText = pNode->ToText();
										if( pText )
											ContentDownloader::Shepherd::addMessageText( pText->Value(), strlen(pText->Value()), 180); //3 minutes
									}
								}
							}
						}
					}
					SAFE_DELETE( pDoc );
				}

				if( sleeptime > 0 || (noWorkSleepDuration > 0 && hasWork == false))
				{
					g_Log->Info( "Chilling for %d+%d seconds...", sleeptime, noWorkSleepDuration );

					std::stringstream tmp;
					if (hasWork == false)
						tmp << "No work received from server. ";
					else
						noWorkSleepDuration = 0;
					tmp << "Rendering restarting in {" << std::fixed << std::setprecision(0) << (sleeptime + noWorkSleepDuration) << "}...";
					Shepherd::setRenderState(tmp.str());

					thread::sleep( get_system_time() + posix_time::seconds(sleeptime + noWorkSleepDuration) );
				}
				else
				{
					noWorkSleepDuration = 0;
					sleeptime = 0;
					thread::yield();
				}
			}
			else
			{
				if (allowRender == false)
				{
					g_Log->Warning( "Rendering not allowed... chilling for %d seconds...", failureSleepDuration );

					thread::sleep( get_system_time() + posix_time::seconds(failureSleepDuration) );

					std::stringstream tmp;
					tmp << "Rendering restarting in {" << std::fixed << std::setprecision(0) << failureSleepDuration << "}...";
					Shepherd::setRenderState(tmp.str());

					failureSleepDuration = TIMEOUT;//Base::Math::Clamped( failureSleepDuration * 2, TIMEOUT, failureSleepDuration*10 );
				}
				else
				{
					g_Log->Warning( "Unable to get control points from server... chilling for %d seconds...", failureSleepDuration );

					thread::sleep( get_system_time() + posix_time::seconds(failureSleepDuration) );

					std::stringstream tmp;
					tmp << "Rendering restarting in {" << std::fixed << std::setprecision(0) << failureSleepDuration << "}...";
					Shepherd::setRenderState(tmp.str());

					failureSleepDuration = TIMEOUT;//Base::Math::Clamped( failureSleepDuration * 2, TIMEOUT, failureSleepDuration*10 );
				}
			}
			SAFE_DELETE( uploader );
		}
	}
	catch(thread_interrupted const&)
	{
	}

	return true;
}

/*
	shepherdCallback().
	This method is the entry point for the shepherd. The method will create new frames and get than indicate to the uploader to begin uploading them
*/
void SheepGenerator::shepherdCallback( void* data )
{
	((SheepGenerator *)data)->generateSheep();
}

}
