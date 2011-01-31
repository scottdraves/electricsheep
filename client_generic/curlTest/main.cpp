#include	<sstream>
#include	"Networking.h"

//
static std::string generateID()
{
    unsigned char	*salt;
    unsigned int	u;
	char id[17];
	id[16] = 0;

    SYSTEMTIME syst;
    GetSystemTime(&syst);
    salt = ((unsigned char *)&syst) + sizeof(SYSTEMTIME) - 8;

	for( u=0; u<16; u++ )
	{
		unsigned int r = rand();
		r = r ^ (salt[u>>1] >> ((u&1)<<2));
		r &= 15;
		if( r < 10 )
			r += '0';
		else
			r += 'A' - 10;

		id[u] = r;
	}

	return 	std::string( id );
}

//
void	TestDownload()
{
	printf( "**** Authenticated download ****\n{\n" );

	//	Download test.
	Network::spCFileDownloader spDownload = new Network::CFileDownloader( "Download test" );
	spDownload->Login( "keffo", "discodans" );

	if( spDownload->Perform( "http://r2d7c.sheepserver.net/gen/243/7854/sheep.avi" ) )
	{
		printf( "%s\n", spDownload->Status().c_str() );
		spDownload->Save( "test.avi" );
	}
	else
		printf( "Download failed...\n" );

	spDownload = NULL;

	printf( "}\n**** Authenticated download complete ****\n\n" );
}

//
void	TestUpload()
{
	printf( "**** Authenticated upload ****\n{\n" );

	//	Upload test.
	Network::spCFileUploader spUpload = new Network::CFileUploader( "Upload test" );
	spUpload->Login( "keffo", "discodans" );

	std::string serverName = "r2d7c.sheepserver.net";
	std::string uniqueID = generateID();

	//	Get filesize.
	FILE *pFile = fopen( "test.png", "rb" );
	fseek( pFile, 0, SEEK_END );
	long fileSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET);
	fclose( pFile );

	//	Make a fake upload url.
	/*std::stringstream url;
	url << "http://" << serverName
		<< "/cgi/put?j=" << 123			//	Job.
		<< "&id=" << 321				//	AnimID.
		<< "&s=" << fileSize			//	Filesize.
		<< "&g=" << 666					//	Generation.
		<< "&v=" << "WIN_2.7b11"		//	Client version.
		<< "&u=" << uniqueID;			//	Client ID.*/

	char pbuf[256];
    snprintf( pbuf, 255, "http://%s/cgi/put?j=%d&id=%d&s=%ld&g=%d&v=%s&u=%s", serverName.c_str(), 123, 321, fileSize, 666, "WIN_2.7b11", uniqueID.c_str() );

/*http://%s/cgi/put?j=%d
				 &id=%d
				 &s=%ld
				 &g=%d
				 &v=%s
				 &u=%s*/

	if( spUpload->Perform( /*url.str()*/pbuf, "test.png", fileSize ) )
		printf( "%s\n", spUpload->().c_str() );
	else
		printf( "Upload failed...\n" );



	spUpload = NULL;

	printf( "}\n**** Authenticated upload complete ****\n\n" );
}

//
int main( int argc, char *argv[] )
{
	g_NetworkManager->Startup();

	TestDownload();
	TestUpload();

	g_NetworkManager->Shutdown();
}
