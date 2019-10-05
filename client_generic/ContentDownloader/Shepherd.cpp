///////////////////////////////////////////////////////////////////////////////
//
//    electricsheep for windows - collaborative screensaver
//    Copyright 2003 Nicholas Long <nlong@cox.net>
//    electricsheep for windows is based of software
//    written by Scott Draves <source@electricsheep.org>
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
#include <sys/stat.h>
#include <zlib.h>
#include <time.h>
#include <vector>
#include <string>
#if defined(WIN32) && defined(_MSC_VER)
#include "../msvc/msvc_fix.h"
#else
#include <dirent.h>
#endif
#include <boost/filesystem.hpp>

#include "base.h"
#include "clientversion.h"
#include "Log.h"
#include "Settings.h"
#include "Shepherd.h"
#include "SheepDownloader.h"
#include "SheepGenerator.h"
#include "md5.h"

namespace ContentDownloader
{

// declare namespace we are using
//
using namespace std;

#define MAXBUF 1024
#define MIN_READ_INTERVAL 600

// Initialize static class data
//
uint64 Shepherd::s_ClientFlockBytes = 0;
uint64 Shepherd::s_ClientFlockGoldBytes = 0;
uint64 Shepherd::s_ClientFlockCount = 0;
uint64 Shepherd::s_ClientFlockGoldCount = 0;
atomic_char_ptr Shepherd::fRootPath(NULL);
atomic_char_ptr Shepherd::fMpegPath(NULL);
atomic_char_ptr Shepherd::fXmlPath(NULL);
atomic_char_ptr Shepherd::fJpegPath(NULL);
atomic_char_ptr Shepherd::fRedirectServerName(NULL);
atomic_char_ptr Shepherd::fServerName(NULL);
atomic_char_ptr Shepherd::fVoteServerName(NULL);
atomic_char_ptr Shepherd::fRenderServerName(NULL);
atomic_char_ptr Shepherd::fProxy(NULL);
atomic_char_ptr Shepherd::fProxyUser(NULL);
atomic_char_ptr Shepherd::fProxyPass(NULL);
int Shepherd::fUseProxy = 0;
int Shepherd::fSaveFrames = 0;
int Shepherd::fCacheSize = 100;
int Shepherd::fCacheSizeGold = 100;
int Shepherd::fRegistered = 0;
atomic_char_ptr Shepherd::fPassword(NULL);
atomic_char_ptr Shepherd::fUniqueID(NULL);
atomic_char_ptr Shepherd::fRole(NULL);
boost::detail::atomic_count *Shepherd::renderingFrames = NULL;
boost::detail::atomic_count *Shepherd::totalRenderedFrames = NULL;
bool Shepherd::m_RenderingAllowed = true;

std::queue<spCMessageBody>	Shepherd::m_MessageQueue;
boost::mutex	Shepherd::m_MessageQueueMutex;

std::vector<spCTimedMessageBody>	Shepherd::m_OverflowMessageQueue;
boost::mutex	Shepherd::s_OverflowMessageQueueMutex;

boost::mutex	Shepherd::s_ShepherdMutex;

boost::shared_mutex	Shepherd::s_DownloadStateMutex;
boost::shared_mutex	Shepherd::s_RenderStateMutex;

boost::shared_mutex	Shepherd::s_GetServerNameMutex;

boost::mutex	Shepherd::s_ComputeServerNameMutex;

bool Shepherd::fShutdown = false;
int Shepherd::fChangeRes = 0;
int Shepherd::fChangingRes = 0;

time_t Shepherd::s_LastRequestTime = 0;

Base::CBlockingQueue<char *> Shepherd::fStringsToDelete;

std::string Shepherd::s_DownloadState;
std::string Shepherd::s_RenderState;

bool Shepherd::s_IsDownloadStateNew = false;
bool Shepherd::s_IsRenderStateNew = false;

#define _24_HOURS (60 * 60 * 24)


Shepherd::~Shepherd()
{
	g_Log->Info( "~Shepherd()..." );

	while( m_MessageQueue.size() != 0 )
		m_MessageQueue.pop();
}

void Shepherd::initializeShepherd(/*HINSTANCE hInst, HWND hWnd*/)
//
// Description:
//		Initialize global data for the shepherd and his heard.
//
{
	SheepDownloader::initializeDownloader();
	SheepGenerator::initializeGenerator();

	totalRenderedFrames = new boost::detail::atomic_count((int)(int32)g_Settings()->Get( "settings.generator.totalFramesRendered", 0 ));
	renderingFrames = new boost::detail::atomic_count(0);
}

/*
	notifyShepherdOfHisUntimleyDeath().
	This method gets called by the main window procedure before the app is about to close. This allows the shepherd to clean any data that it allocated.
*/
void	Shepherd::notifyShepherdOfHisUntimleyDeath()
{
	g_Log->Info( "notifyShepherdOfHisUntimleyDeath..." );

	SheepDownloader::closeDownloader();
	SheepGenerator::closeGenerator();

	//boost::mutex::scoped_lock lockthis( s_ShepherdMutex );

	SAFE_DELETE_ARRAY( fRootPath );
	SAFE_DELETE_ARRAY( fMpegPath );
	SAFE_DELETE_ARRAY( fXmlPath );
	SAFE_DELETE_ARRAY( fJpegPath );
	SAFE_DELETE_ARRAY( fRedirectServerName );
	SAFE_DELETE_ARRAY( fServerName );
	SAFE_DELETE_ARRAY( fVoteServerName );
	SAFE_DELETE_ARRAY( fRenderServerName );
	SAFE_DELETE_ARRAY( fPassword );
	SAFE_DELETE_ARRAY( fProxy );
	SAFE_DELETE_ARRAY( fProxyUser );
	SAFE_DELETE_ARRAY( fProxyPass );
	SAFE_DELETE_ARRAY( fUniqueID );
	SAFE_DELETE_ARRAY( fRole );

	SAFE_DELETE( totalRenderedFrames );
	SAFE_DELETE( renderingFrames );

	std::vector<char *>::const_iterator it;
	
	char *str;
	
	while(fStringsToDelete.pop(str))
	{
		SAFE_DELETE_ARRAY( str );
	}
	
	fStringsToDelete.clear(0);
}
    
void Shepherd::setNewAndDeleteOldString(atomic_char_ptr &str, char *newval, boost::memory_order mem_ord)
{
    char *toDelete = str.exchange(newval, mem_ord);
    
    if (toDelete != NULL)
        fStringsToDelete.push(toDelete);
}

void Shepherd::setRootPath(const char *path)
//
// Description:
//		This method is used to set the root path
// where all of the files will be created and downloaded
// to. The method also initializes all of the relative paths
// for the subdirectories.
//
{
	// strip off trailing white space
	//
	size_t len = strlen(path);
	char *runner = const_cast<char *>(path) + len - 1;
	while(*runner == ' ')
	{
		len--; runner--;
	}
    
    //we need space for trailing \ character
    char *newRootPath = new char[len + 2];

	// copy the data
	memcpy(newRootPath, path, len);

	// check for the trailing \ character
	if(*runner != PATH_SEPARATOR_C)
	{
		newRootPath[len] = PATH_SEPARATOR_C;
		newRootPath[len + 1] = '\0';
	}
	else
		newRootPath[len] = '\0';

	// create the directory for the path
#ifdef WIN32
	CreateDirectoryA(newRootPath, NULL);
#else
	mkdir(newRootPath, 0755);
#endif
    
    setNewAndDeleteOldString(fRootPath, newRootPath);
    
	char *newMpegPath = new char[(len + 12)];
    
	snprintf(newMpegPath, len + 12, "%smpeg%c", newRootPath,PATH_SEPARATOR_C);
#ifdef WIN32
	CreateDirectoryA(newMpegPath, NULL);
#else
	mkdir(newMpegPath, 0755);
#endif
    
    setNewAndDeleteOldString(fMpegPath, newMpegPath);

    char *newXmlPath = new char[(len + 12)];
	snprintf(newXmlPath, len + 12,"%sxml%c", newRootPath,PATH_SEPARATOR_C);
#ifdef WIN32
	CreateDirectoryA(newXmlPath, NULL);
#else
	mkdir(newXmlPath, 0755);
#endif

    setNewAndDeleteOldString(fXmlPath, newXmlPath);

    char *newJpegPath = new char[(len + 12)];
	snprintf(newJpegPath, len + 12,"%sjpeg%c", newRootPath,PATH_SEPARATOR_C);
#ifdef WIN32
	CreateDirectoryA(newJpegPath, NULL);
#else
	mkdir(newJpegPath, 0755);
#endif
    
    setNewAndDeleteOldString(fJpegPath, newJpegPath);
}

void Shepherd::setRole( const char *role )
{
	size_t len = strlen(role);

    char *newRole = new char[len + 1];
	strcpy(newRole, role);
    
    setNewAndDeleteOldString(fRole, newRole);
}

const char *Shepherd::role()
{
	return fRole.load(boost::memory_order_relaxed);
}

void
Shepherd::setRedirectServerName(const char *server)
//
// Description:
//		Sets the server name for the sheep server.
//
{
	size_t len = strlen(server);

	char *newRedirectServerName = new char[len + 1];
	strcpy(newRedirectServerName, server);
    
    setNewAndDeleteOldString(fRedirectServerName, newRedirectServerName);
}


void
Shepherd::setProxy(const char *proxy)
//
// Description:
//		Sets the proxy server address for tcp|ip
//	transactions.
//
{
	size_t len = strlen(proxy);

	char *newProxy = new char[len + 1];
	strcpy(newProxy, proxy);
    
    setNewAndDeleteOldString(fProxy, newProxy);
}

void
Shepherd::setProxyUserName(const char *userName)
//
// Description:
//		Sets the proxy username.
//
{
	size_t len = strlen(userName);

	char *newProxyUser = new char[len + 1];
	strcpy(newProxyUser, userName);

    setNewAndDeleteOldString(fProxyUser, newProxyUser);
}

void
Shepherd::setProxyPassword(const char *password)
//
// Description:
//		Sets the proxy password.
//
{
	size_t len = strlen(password);

    char *newProxyPass = new char[len + 1];
	strcpy(newProxyPass, password);

    setNewAndDeleteOldString(fProxyPass, newProxyPass);
}

int
Shepherd::filenameIsMpg(const char *name)
//
// Description:
//		Returns if the file is a valid mpeg name.
//
{
	size_t n = strlen(name);
	return !((n <= 4 || 0 != strcmp(&name[n-4], ".avi")) );
}

int
Shepherd::filenameIsXxx(const char *name)
//
// Description:
//		Returns if the file is a deleted mpeg file.
//
{
	size_t n = strlen(name);
	return !(n <= 4 || 0 != strcmp(&name[n-4], ".xxx"));
}

int
Shepherd::filenameIsTmp(const char *name)
//
// Description:
//		Returns if the file is a deleted mpeg file.
//
{
	size_t n = strlen(name);
	return !(n <= 4 || 0 != strcmp(&name[n-4], ".tmp"));
}

void Shepherd::addMessageText(const char *s, size_t len, time_t timeout)
{
	QueueMessage( std::string( s, len ), (fp8)timeout );
}

const char *Shepherd::rootPath()
{
    return fRootPath.load(boost::memory_order_relaxed);
}

const char *Shepherd::mpegPath()
{
	return fMpegPath.load(boost::memory_order_relaxed);
}

const char *Shepherd::xmlPath()
{
	return fXmlPath.load(boost::memory_order_relaxed);
}

const char *Shepherd::jpegPath()
{
	return fJpegPath.load(boost::memory_order_relaxed);
}

std::string Shepherd::computeMD5( const std::string& str )
{
	unsigned char digest[16]; //md5 digest size is 16

	md5_buffer( str.c_str(), str.size(), digest );

	std::string md5Str;

	for (uint32 i = 0; i < sizeof(digest); i++)
	{
		const char *hex_digits = "0123456789ABCDEF";

		md5Str += hex_digits[ digest[i] >> 4 ];
		md5Str += hex_digits[ digest[i] & 0x0F ];
	}

	return md5Str;
}


const char *Shepherd::serverName( bool allowServerQuery, eServerTargetType serverType)
{
	//just want to get current server name? We should not block!!!
	if (!allowServerQuery)
	{
        return fServerName.load(boost::memory_order_relaxed);
	}

	{
		boost::mutex::scoped_lock lockName( s_ComputeServerNameMutex );
        
        if ( ( fServerName.load(boost::memory_order_relaxed) == NULL || (time(0) - s_LastRequestTime) > _24_HOURS ) )
		{
            const char *redirectServerName = REDIRECT_SERVER_FULL; //fRedirectServerName.load(boost::memory_order_relaxed);
            
            if ( redirectServerName != NULL )
			{
				std::string	nickEncoded = Network::CManager::Encode( SheepGenerator::nickName() );
				std::string	passEncoded = Network::CManager::Encode( Shepherd::password() );
                
                bool addHTTP = false;
                
                if (redirectServerName[0] != 'h' || redirectServerName[1] != 't' || redirectServerName[2] != 't' || redirectServerName[3] != 'p')
                {
                    addHTTP = true;
                }

				//	Create the url for getting the cp file to create the frame
				char 	url[ MAXBUF*5 ];
				snprintf( url, MAXBUF*5, "%s%s/query.php?q=redir&u=%s&p=%s&v=%s&i=%s",
                    addHTTP ? "http://" : "",
                    redirectServerName,
					nickEncoded.c_str(),
					passEncoded.c_str(),
					CLIENT_VERSION,
					Shepherd::uniqueID()
					);

				Network::spCFileDownloader spDownload = new Network::CFileDownloader( "Sheep Server Request" );

				if( spDownload->Perform( url ) )
				{
					std::string response( spDownload->Data() );

					//TinyXML has problems with string not terminated by \n
					response += "\n";

					TiXmlDocument doc;
					if ( doc.Parse(response.c_str(), NULL, TIXML_ENCODING_UTF8 ) )
					{
						TiXmlHandle hDoc(&doc);
						TiXmlElement* listElement;
						const char *host = NULL;
						const char *vote = NULL;
						const char *render = NULL;
						const char *role = NULL;

						listElement=hDoc.FirstChild( "query" ).FirstChild( "redir" ).Element();

						if ( listElement != NULL )
						{
							host = listElement->Attribute("host");
							vote = listElement->Attribute("vote");
							render = listElement->Attribute("render");
							role = listElement->Attribute("role");
						}

						if (vote != NULL && *vote != 0)
						{
							const char *oldVoteServerName = fVoteServerName.load(boost::memory_order_relaxed);

							if (oldVoteServerName == NULL || (strcmp(oldVoteServerName, vote) != 0))
							{
								char *newVoteServerName = new char[strlen(vote) + 1];

								strcpy(newVoteServerName, vote);

								setNewAndDeleteOldString(fVoteServerName, newVoteServerName);
							}
						}


						if (render != NULL && *render != 0)
						{
							const char *oldRenderServerName = fRenderServerName.load(boost::memory_order_relaxed);

							if (oldRenderServerName == NULL || (strcmp(oldRenderServerName, render) != 0))
							{
								char *newRenderServerName = new char[strlen(render) + 1];

								strcpy(newRenderServerName, render);

								setNewAndDeleteOldString(fRenderServerName, newRenderServerName);
							}
						}


						if ( host != NULL && *host != 0 )
						{
                            const char *oldServerName = fServerName.load(boost::memory_order_relaxed);
							
                            if ( oldServerName == NULL || (strcmp( oldServerName, host ) != 0) )
							{
								char *newServerName = new char[ strlen(host) + 1 ];

								strcpy( newServerName, host );
                                
                                setNewAndDeleteOldString(fServerName, newServerName);
							}

						}

						if ( role != NULL && *role != 0 )
						{
							setRole(role);
						}
					}
					else
					{
						fprintf( stderr, "Server Request Failed: %s at line %d\n", doc.ErrorDesc(), doc.ErrorRow() );
					}

					time(&s_LastRequestTime);
				}
			}
		}
	}

	switch (serverType)
	{
	case eVoteServer:
		if (fVoteServerName.load(boost::memory_order_relaxed) != NULL) return fVoteServerName.load(boost::memory_order_relaxed);
		else return fServerName.load(boost::memory_order_relaxed);
	case eRenderServer:
		if (fRenderServerName.load(boost::memory_order_relaxed) != NULL) return fRenderServerName.load(boost::memory_order_relaxed);
		else return fServerName.load(boost::memory_order_relaxed);
	case eHostServer:
	default:
    return fServerName.load(boost::memory_order_relaxed);
}
}

const char *Shepherd::proxy()
{
    return fProxy.load(boost::memory_order_relaxed);
}

const char	*Shepherd::proxyUserName()
{
	return fProxyUser.load(boost::memory_order_relaxed);
}

/*
*/
const char	*Shepherd::proxyPassword()
{
	return fProxyPass.load(boost::memory_order_relaxed);
}

/*
*/
void	Shepherd::setPassword( const char *password )
{
	size_t len = strlen(password);

	char *newPassword = new char[ len + 1 ];
	strcpy( newPassword, password );
    
    setNewAndDeleteOldString(fPassword, newPassword);
}

/*
*/
const char *Shepherd::password()
{
	return fPassword.load(boost::memory_order_relaxed);
}

/*
	Get flock size in MBs after recounting
*/

uint64 Shepherd::GetFlockSizeMBsRecount(const int generationtype)
{
	if (generationtype == 0)
	{
		ContentDownloader::SheepArray tempSheepArray;
		ContentDownloader::Shepherd::getClientFlock(&tempSheepArray);
		for(unsigned i = 0; i < tempSheepArray.size(); ++i)
			delete tempSheepArray[i];
	}
	if (generationtype == 1)
		return ContentDownloader::Shepherd::getClientFlockMBs(1);

	return ContentDownloader::Shepherd::getClientFlockMBs(0);
}

/*
	getClientFlock().
	This method fills the given sheeparray with all of the sheep on the client.
*/
bool	Shepherd::getClientFlock(SheepArray *sheep)
{
	uint64 clientFlockBytes = 0;
	uint64 clientFlockCount = 0;
	
	uint64 clientFlockGoldBytes = 0;
	uint64 clientFlockGoldCount = 0;

	SheepArray::iterator iter;
	for (iter = sheep->begin(); iter != sheep->end(); ++iter )
		delete *iter;

	sheep->clear();

	//	Get the sheep in fMpegPath.
	getSheep( mpegPath(), sheep );
	for (iter = sheep->begin(); iter != sheep->end(); ++iter )
	{
		if ((*iter)->getGenerationType() == 0)
		{
			clientFlockBytes += (*iter)->fileSize();
			++clientFlockCount;
		}
		else if ((*iter)->getGenerationType() == 1)
		{
			clientFlockGoldBytes += (*iter)->fileSize();
			++clientFlockGoldCount;
		}
	}
	
	
	s_ClientFlockBytes = clientFlockBytes;
	s_ClientFlockCount = clientFlockCount;
	
	s_ClientFlockGoldBytes = clientFlockGoldBytes;
	s_ClientFlockGoldCount = clientFlockGoldCount;

	return true;
}

using namespace boost::filesystem;

/*
	getSheep().
	Recursively loops through all files in the path looking for sheep.
*/
bool Shepherd::getSheep( const char *path, SheepArray *sheep )
{
	bool gotSheep = false;
	char fbuf[ MAXBUF ];

	try {
	boost::filesystem::path p(path);

	directory_iterator end_itr; // default construction yields past-the-end
	for ( directory_iterator itr( p );
			itr != end_itr;
			++itr )
	{
		uint32 id = 0;
		uint32 first = 0;
		uint32 last = 0;
		uint32 generation = 0;
		bool isTemp = false;
		bool isDeleted = false;

		if (is_directory(itr->status()))
		{
			bool gotSheepSubfolder = getSheep( (char*)(itr->path().string() + std::string("/")).c_str(), sheep );
			gotSheep |= gotSheepSubfolder;

			if (!gotSheepSubfolder)
				remove_all(itr->path());
		}
		else
		{
			std::string fname(itr->path().filename().string());

			if( Shepherd::filenameIsXxx( fname.c_str() ) )
			{
				//	We have found an mpeg so check the filename to see if it is valid than add it
				if( 4 != sscanf( fname.c_str(), "%d=%d=%d=%d.xxx", &generation, &id, &first, &last ) )
				{
					//	This file is from the older format so delete it from the cache.
					remove(itr->path());
					continue;
				}

				isDeleted = true;
			}
			else if( Shepherd::filenameIsTmp( fname.c_str() ) )
			{
				//	This file is an unfinished downloaded file so mark it for deletion.
				if( 4 != sscanf( fname.c_str(), "%d=%d=%d=%d.avi.tmp", &generation, &id, &first, &last ) )
				{
					//	This file is from the older format so delete it from the cache.
					remove(itr->path());
					continue;
				}
				isTemp = true;
			}
			else if( Shepherd::filenameIsMpg( fname.c_str() ) )
			{
				//	We have found an mpeg so check the filename to see if it is valid than add it
				if( 4 != sscanf( fname.c_str(), "%d=%d=%d=%d.avi", &generation, &id, &first, &last ) )
				{
					//	This file is from the older format so delete it from the cache.
					remove(itr->path());
					continue;
				}
				
				std::string xxxname(fname);
				xxxname.replace(fname.size() - 3, 3, "xxx");
				
				if ( exists( p/xxxname ) )
				{
					isDeleted = true;
				}
			}
			else
			{
				//	Not a recognizable file skip it.
				continue;
			}

			//	Allocate the sheep and set the attributes.
			Sheep *newSheep = new Sheep();
			newSheep->setGeneration( generation );
			newSheep->setId( id );
			newSheep->setFirstId( first );
			newSheep->setLastId( last );
			newSheep->setIsTemp( isTemp );
			newSheep->setDeleted( isDeleted );

			//	Set the filename.
			snprintf( fbuf, MAXBUF, "%s%s", path,  fname.c_str() );
			newSheep->setFileName( fbuf );
			struct stat sbuf;

			stat( fbuf, &sbuf );
			newSheep->setFileWriteTime( sbuf.st_ctime );
			newSheep->setFileSize( static_cast<uint64>(sbuf.st_size) );

			//	Add it to the return array.
			sheep->push_back( newSheep );
			gotSheep = true;
		}
	}

	}
	catch(boost::filesystem::filesystem_error& err)
	{
		g_Log->Error( "Path enumeration threw error: %s",  err.what() );
	}

	return gotSheep;
}

//	Sets the unique id for this Shepherd.
void	Shepherd::setUniqueID( const char *uniqueID )
{
	size_t len;
	if( strlen( uniqueID ) > 16 )
		len = strlen( uniqueID );
	else
		len = 16;

	char *newUniqueID = new char[ len + 1 ];
	strcpy( newUniqueID, uniqueID );
    
    setNewAndDeleteOldString(fUniqueID, newUniqueID);
}

//	This method returns the unique ID to use.
const char *Shepherd::uniqueID()
{
    return fUniqueID.load(boost::memory_order_relaxed);
}

void Shepherd::setDownloadState( const std::string& state )
{
    boost::upgrade_lock<boost::shared_mutex> lockthis( s_DownloadStateMutex );

	s_DownloadState.assign( state );
	
	s_IsDownloadStateNew = true;
}

std::string Shepherd::downloadState( bool &isnew )
{
	boost::shared_lock<boost::shared_mutex> lockthis( s_DownloadStateMutex );
	
	isnew = s_IsDownloadStateNew;
	
	s_IsDownloadStateNew = false;
	
	return s_DownloadState;
}

void Shepherd::setRenderState( const std::string& state )
{
	boost::upgrade_lock<boost::shared_mutex> lockthis( s_RenderStateMutex );

	s_RenderState.assign( state );
	
	s_IsRenderStateNew = true;
}

std::string Shepherd::renderState( bool &isnew )
{
	boost::shared_lock<boost::shared_mutex> lockthis( s_RenderStateMutex );
	
	isnew = s_IsRenderStateNew;
	
	s_IsRenderStateNew = false;
	
	return s_RenderState;
}

//
void Shepherd::FrameStarted()		{	if (renderingFrames) ++(*renderingFrames);	}
int	Shepherd::FramesRendering()		{	return renderingFrames ? static_cast<int>(*renderingFrames) : 0;	}
int	Shepherd::TotalFramesRendered()	{	return totalRenderedFrames ? static_cast<int>(*totalRenderedFrames) : 0;	}
bool Shepherd::RenderingAllowed()	{	return m_RenderingAllowed;	}
void Shepherd::SetRenderingAllowed(bool _yesno)	{	m_RenderingAllowed = _yesno;	}

//
void	Shepherd::FrameCompleted()
{
	if (renderingFrames) --(*renderingFrames);
	if (totalRenderedFrames) ++(*totalRenderedFrames);
	g_Settings()->Set( "settings.generator.totalFramesRendered", totalRenderedFrames ? (int32)*totalRenderedFrames : 0 );
}

};
