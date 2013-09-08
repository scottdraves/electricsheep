#ifndef CLIENTVERSION_H_INCLUDED
#define CLIENTVERSION_H_INCLUDED

#ifdef	WIN32
	#define CLIENT_VERSIONW				L"WIN_2.7b34c"
	#define	CLIENT_VERSION_PRETTYW		L"Windows 2.7 (Beta 34c)"
	#define CLIENT_SETTINGSW			L"ElectricSheep"

	#define CLIENT_VERSION				"WIN_2.7b34c"
	#define	CLIENT_VERSION_PRETTY		"Windows 2.7 (Beta 34c)"
	#define CLIENT_SETTINGS				"ElectricSheep"

	#define	CLIENT_VERSION_PRETTYW2		L"version 2.7 (Beta 34c)"
#else
	#ifdef MAC
		#define CLIENT_VERSION				"OSX_2.7b35"
		#define	CLIENT_VERSION_PRETTY		"Mac OS X 2.7 (Beta 35)"
		#define CLIENT_SETTINGS				"ElectricSheep"
	#else
		#define CLIENT_VERSION				"LNX_2.7b33"
		#define	CLIENT_VERSION_PRETTY		"version 2.7b33"
		#define CLIENT_SETTINGS				"ElectricSheep"
	#endif
#endif

#ifdef WIN32
	#define	CLIENT_SERVERW				L"v2d7c.sheepserver.net"
	#define REDIRECT_SERVERW			L"community.sheepserver.net"
#endif

#define	CLIENT_SERVER				"v2d7c.sheepserver.net"
//#define	CLIENT_SERVER_REGISTERED	"r2d7c.sheepserver.net"
#define REDIRECT_SERVER				"community.sheepserver.net"

#define	BETA_RELEASE


#endif // CLIENTVERSION_H_INCLUDED
