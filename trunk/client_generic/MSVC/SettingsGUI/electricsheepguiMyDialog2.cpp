#include "electricsheepguiMyDialog2.h"
#include <wx/valtext.h>
#include <wx/msgdlg.h>

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "../../msvc/msvc_fix.h"
#endif

#include "../../TupleStorage/Settings.h"
#include "../../Common/md5.h"
#include "../../Common/clientversion.h"
#include "../../Common/ProcessForker.h"

#include	<curl/curl.h>
#include	<curl/types.h>
#include	<curl/easy.h>

#ifdef WIN32
#define CLIENT_HELP_LINKW  			L"http://electricsheep.org/client/WIN_2.7b30"
#define CLIENT_HELP_LINK  			"http://electricsheep.org/client/WIN_2.7b30"
#endif

#ifdef LINUX_GNU
#define CLIENT_HELP_LINK  			"http://electricsheep.org/client/LNX_2.7b28"
#endif

static electricsheepguiMyDialog2 *sMainDialog = NULL;

std::string computeMD5( const std::string& str )
{
	unsigned char digest[16]; //md5 digest size is 16

	md5_buffer( str.c_str(), str.size(), digest );

	std::string md5Str;

	for (int i = 0; i < sizeof(digest); i++)
	{
	  const char *hex_digits = "0123456789ABCDEF";

	        md5Str += hex_digits[ digest[i] >> 4 ];
		md5Str += hex_digits[ digest[i] & 0x0F ];
	}

	return md5Str;
}

std::string Encode( const std::string &_src )
{
	const int8	dec2hex[ 16 + 1 ] = "0123456789ABCDEF";
	const uint8	*pSrc = (const uint8 *)_src.c_str();
	const int32 srcLen= _src.length();
	uint8 *const pStart = new uint8[ srcLen * 3 ];
	uint8 *pEnd = pStart;
	const uint8 * const srcEnd = pSrc + srcLen;

	for( ; pSrc<srcEnd; ++pSrc )
	{
		if( isalnum( *pSrc ) )
			*pEnd++ = *pSrc;
		else
		{
			//	Escape this char.
			*pEnd++ = '%';
			*pEnd++ = dec2hex[ *pSrc >> 4 ];
			*pEnd++ = dec2hex[ *pSrc & 0x0F ];
		}
	}

   std::string sResult( (char *)pStart, (char *)pEnd );
   delete [] pStart;
   return sResult;
}

void electricsheepguiMyDialog2::SaveSettings()
{
	g_Settings()->Set("settings.app.log", m_DebugLog->GetValue());
	g_Settings()->Set("settings.app.attributionpng", m_checkAttributionPNG->GetValue());

	long temp = 0;

	m_spinCache->GetValue().ToLong(&temp);
	g_Settings()->Set("settings.content.cache_size", (int32)temp);
	
	m_spinGoldCache->GetValue().ToLong(&temp);
	g_Settings()->Set("settings.content.cache_size_gold", (int32)temp);

	m_spinDecodeFps->GetValue().ToLong(&temp);
	g_Settings()->Set("settings.player.player_fps", (int32)temp);

	m_spinDecodeFpsGold->GetValue().ToLong(&temp);
	g_Settings()->Set("settings.player.player_fps_gold", (int32)temp);

	g_Settings()->Set("settings.player.screen", m_spinMonitor->GetValue());
	g_Settings()->Set("settings.player.LoopIterations", m_spinRepeatLoops->GetValue());

	m_spinDisplayFps->GetValue().ToLong(&temp);
	g_Settings()->Set("settings.player.display_fps", (int32)temp);

	g_Settings()->Set("settings.player.PlaybackMixingMode", m_radioPlaybackMixingMode->GetSelection());
	g_Settings()->Set("settings.player.SeamlessPlayback", m_SeamlessPlayback->GetValue());
	g_Settings()->Set("settings.player.quiet_mode", m_QuietMode->GetValue());
	g_Settings()->Set("settings.player.directdraw", m_DirectDraw->GetValue());

	g_Settings()->Set("settings.content.unlimited_cache", m_checkUnlimitedCache->IsChecked());
	
	g_Settings()->Set("settings.content.unlimited_cache_gold", m_checkGoldUnlimitedCache->IsChecked());

	g_Settings()->Set("settings.content.download_mode", m_checkHttp->GetValue());
	g_Settings()->Set("settings.generator.enabled", m_checkRenderFrames->GetValue());
	g_Settings()->Set("settings.generator.all_cores", m_checkMulticore->GetValue());
	g_Settings()->Set("settings.generator.save_frames", m_checkKeepFrames->GetValue());

	g_Settings()->Set("settings.content.negvotedeletes", m_checkNegVoteDeletes->GetValue());
	g_Settings()->Set("settings.player.vbl_sync", m_VerticalSync->GetValue());
	g_Settings()->Set("settings.player.reversedisplays", m_ReverseDisplays->GetValue());

	g_Settings()->Set("settings.generator.nickname", std::string(m_textDrupalName->GetValue()));

	if (g_Settings()->Get("settings.content.password_md5", std::string("")) != "")
		g_Settings()->Set("settings.content.password", g_Settings()->Get("settings.content.password_md5", std::string("")));

	g_Settings()->Set("settings.player.DisplayMode", m_radioDisplayMode->GetSelection());
	g_Settings()->Set("settings.player.MultiDisplayMode", m_radioMultiDisplayMode->GetSelection());

	g_Settings()->Set( "settings.content.sheepdir", std::string( m_dirContent->GetPath() ) );

	g_Settings()->Set( "settings.content.proxy", std::string( m_textProxyHost->GetValue() ) );
	g_Settings()->Set( "settings.content.proxy_username", std::string( m_textProxyUser->GetValue() ) );
	g_Settings()->Set( "settings.content.proxy_password", std::string( m_textProxyPassword->GetValue() ) );

	if (m_textProxyHost->GetValue() != wxT(""))
		g_Settings()->Set( "settings.content.use_proxy", true);
	else
		g_Settings()->Set( "settings.content.use_proxy", false);

	g_Settings()->Storage()->Commit();
}

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

uint64 GetFlockSizeBytes(wxString mpegpath, int sheeptype)
{
	if (mpegpath.substr(mpegpath.size() - 1, 1) != wxT("/"))
		mpegpath += wxT("/");
	uint64 retval = 0;

	try {
	boost::filesystem::path p(mpegpath.c_str());

	directory_iterator end_itr; // default construction yields past-the-end
	for ( directory_iterator itr( p );
			itr != end_itr;
			++itr )
	{
		if (!is_directory(itr->status()))
		{
			std::string fname(itr->path().filename());
			if (itr->path().extension() == std::string(".avi"))
			{
				int generation;
				int id;
				int first;
				int last;

				if( 4 == sscanf( fname.c_str(), "%d=%d=%d=%d.avi", &generation, &id, &first, &last ) )
				{
					if ( (generation >= 10000 && sheeptype == 1) || (generation < 10000 && sheeptype == 0) )
					{
						struct stat sbuf;

						if (stat( (mpegpath + wxT("/") +fname).c_str(), &sbuf ) == 0)
							retval += sbuf.st_size;
					}
				}
			}
		}
		else
			retval += GetFlockSizeBytes(itr->string(), sheeptype);
	}

	}
	catch(boost::filesystem::filesystem_error& err)
	{
		g_Log->Error( "Path enumeration threw error: %s",  err.what() );
		return 0;
	}
	return retval;
}

size_t GetFlockSizeMBs(wxString mpegpath, int sheeptype)
{
	return GetFlockSizeBytes(mpegpath, sheeptype)/1024/1024;

}

void electricsheepguiMyDialog2::LoadSettings()
{
	m_DebugLog->SetValue(g_Settings()->Get("settings.app.log", false));
	m_checkAttributionPNG->SetValue(g_Settings()->Get("settings.app.attributionpng", true));
	m_spinCache->SetValue(wxString::Format(wxT("%d"), g_Settings()->Get("settings.content.cache_size", 2000)));
	m_spinGoldCache->SetValue(wxString::Format(wxT("%d"), g_Settings()->Get("settings.content.cache_size_gold", 2000)));
	m_spinDecodeFps->SetValue(wxString::Format(wxT("%d"), g_Settings()->Get("settings.player.player_fps", 20)));
	m_spinDecodeFpsGold->SetValue(wxString::Format(wxT("%d"), g_Settings()->Get("settings.player.player_fps_gold", 30)));
	m_spinMonitor->SetValue(wxString::Format(wxT("%d"), g_Settings()->Get("settings.player.screen", 0)));
	m_spinRepeatLoops->SetValue(wxString::Format(wxT("%d"), g_Settings()->Get("settings.player.LoopIterations", 2)));
	m_spinDisplayFps->SetValue(wxString::Format(wxT("%d"), g_Settings()->Get("settings.player.display_fps", 60)));

	if (g_Settings()->Get( "settings.content.unlimited_cache", false) == true)
	{
		m_checkUnlimitedCache->SetValue(true);
		m_spinCache->Enable(false);
	}
	long temp = 0;
	m_spinCache->GetValue().ToLong(&temp);
	if (temp == 0)
	{
		m_checkUnlimitedCache->SetValue(true);
		m_spinCache->Enable(false);
	}

	if (g_Settings()->Get( "settings.content.unlimited_cache_gold", false) == true)
	{
		m_checkGoldUnlimitedCache->SetValue(true);
		m_spinGoldCache->Enable(false);
	}

	m_spinGoldCache->GetValue().ToLong(&temp);
	if (temp == 0)
	{
		m_checkGoldUnlimitedCache->SetValue(true);
		m_spinGoldCache->Enable(false);
	}

	m_checkHttp->SetValue( g_Settings()->Get( "settings.content.download_mode", true )  );
	m_checkRenderFrames->SetValue( g_Settings()->Get( "settings.generator.enabled", true )  );
	m_checkMulticore->SetValue( g_Settings()->Get( "settings.generator.all_cores", false  )  );
	m_radioPlaybackMixingMode->SetSelection( g_Settings()->Get( "settings.player.PlaybackMixingMode", 0 ) );
	m_SeamlessPlayback->SetValue( g_Settings()->Get( "settings.player.SeamlessPlayback", false ) );
	m_QuietMode->SetValue( g_Settings()->Get( "settings.player.quiet_mode", true) );
	m_DirectDraw->SetValue( g_Settings()->Get( "settings.player.directdraw", false) );
	m_checkKeepFrames->SetValue( g_Settings()->Get( "settings.generator.save_frames", false  )  );
	m_checkNegVoteDeletes->SetValue( g_Settings()->Get( "settings.content.negvotedeletes", true )  );
	m_VerticalSync->SetValue( g_Settings()->Get( "settings.player.vbl_sync", false ) );
	m_ReverseDisplays->SetValue( g_Settings()->Get( "settings.player.reversedisplays", false ) );

	m_textDrupalName->ChangeValue( g_Settings()->Get( "settings.generator.nickname", std::string("") ) );
	m_textDrupalPassword->ChangeValue( g_Settings()->Get( "settings.content.password_md5", std::string("") ) );

	/*if (g_Settings()->Get( "settings.content.registered", false ) == true )
	{
		m_staticText6->SetLabel( "...logged in!..." );
		m_CreateAccountButton->Enable( false );
	}
	else
	{
		m_staticText6->SetLabel( "...not logged in!..." );
		m_CreateAccountButton->Enable( true );
	}*/

	m_radioDisplayMode->SetSelection( g_Settings()->Get( "settings.player.DisplayMode", 2 ) );
	m_radioMultiDisplayMode->SetSelection( g_Settings()->Get( "settings.player.MultiDisplayMode", 0 ) );

	m_textProxyHost->SetValue( g_Settings()->Get( "settings.content.proxy", std::string("")) );
	m_textProxyUser->SetValue( g_Settings()->Get( "settings.content.proxy_username", std::string("")) );
	m_textProxyPassword->SetValue( g_Settings()->Get( "settings.content.proxy_password", std::string("")) );

	if (m_textProxyHost->GetValue() != wxT(""))
		g_Settings()->Set( "settings.content.use_proxy", true);
	else
		g_Settings()->Set( "settings.content.use_proxy", false);

#ifndef LINUX_GNU
	m_dirContent->SetPath( g_Settings()->Get( "settings.content.sheepdir", std::string(szPath) + "Content" ) );
#else
	m_dirContent->SetPath( g_Settings()->Get( "settings.content.sheepdir", std::string(szPath) + "content" ) );
#endif
	wxString newlabel = wxT("New sheep are created everyday.  When the screensaver runs,\nit tries to download them, and saves them on your hard disk,\ndeleting old ones to make room.  Login to get more sheep.\nIt is currently using ");
	wxString newlabelgold = wxT("It is currently using ");
#ifndef LINUX_GNU
	newlabel += wxString::Format(wxT("%d"), (int)GetFlockSizeMBs(m_dirContent->GetPath()+"\\mpeg", 0)) + wxT("MB.");
	newlabelgold += wxString::Format(wxT("%d"), (int)GetFlockSizeMBs(m_dirContent->GetPath()+"\\mpeg", 1)) + wxT("MB.");
#else
	newlabel += wxString::Format(wxT("%d"), (int)GetFlockSizeMBs(m_dirContent->GetPath()+"/mpeg", 0)) + wxT("MB.");
	newlabelgold += wxString::Format(wxT("%d"), (int)GetFlockSizeMBs(m_dirContent->GetPath()+"/mpeg", 1)) + wxT("MB.");
#endif
	m_staticTextFlockSize->SetLabel(newlabel);
	m_staticTextGoldFlockSize1->SetLabel(newlabelgold);
}

int32 customWrite( void *_pBuffer, size_t _size, size_t _nmemb, void *_pUserData )
{
	return _size*_nmemb; // dummy
}


void electricsheepguiMyDialog2::LoginTest()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL *pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_USERPWD, std::string(Encode( std::string(m_textDrupalName->GetValue()) ) + std::string(":") + Encode( std::string(m_textDrupalPassword->GetValue()) ) ).c_str());

	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &customWrite);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, true);

	curl_slist *slist=NULL;
	slist = curl_slist_append(slist, "Connection: Keep-Alive");
	slist = curl_slist_append(slist, "Accept-Language: en-us");

	curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, slist);
	curl_easy_setopt(pCurl, CURLOPT_URL, (std::string("http://") + CLIENT_SERVER ).c_str());
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 15);
	
	wxMutexGuiEnter();
	m_staticText6->SetLabel("...talking");
	wxMutexGuiLeave();

	long code = 0;
	if (code = curl_easy_perform(pCurl) == CURLE_OK)
	{
		if (curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &code) == CURLE_OK)
		{
			if (code == 200)
			{
				wxMutexGuiEnter();
				m_staticText6->SetLabel("...logged in!...");
				wxMutexGuiLeave();
				m_CreateAccountButton->Enable(false);
				g_Settings()->Set("settings.content.registered", true);

				curl_slist_free_all(slist);

				curl_easy_cleanup( pCurl );

				curl_global_cleanup();
				return;
			}
		}
	}
	g_Settings()->Set("settings.content.registered", false);
	wxMutexGuiEnter();
	m_staticText6->SetLabel("...Failed!...");
	m_CreateAccountButton->Enable(true);
	wxMutexGuiLeave();

	curl_slist_free_all(slist);

	curl_easy_cleanup( pCurl );

	curl_global_cleanup();
	return;
}

electricsheepguiMyDialog2::~electricsheepguiMyDialog2()
{
	if (m_LoginThread != NULL)
	{
		m_TestLogin = false;
		m_LoginThread->Delete();
		delete m_LoginThread;
	}
	g_Settings()->Shutdown();
}

electricsheepguiMyDialog2::electricsheepguiMyDialog2( wxWindow* parent )
:
MyDialog2( parent )
{
	sMainDialog = this;
	m_TestLogin = true;
	m_LoginThread = NULL;
#ifndef LINUX_GNU
if( SUCCEEDED( SHGetFolderPathA( NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath ) ) )
	PathAppendA( szPath, "\\ElectricSheep\\" );

g_Settings()->Init(szPath,".\\");
#else
 sprintf( szPath, "%s/.electricsheep/", getenv("HOME") );
 g_Settings()->Init(szPath, SHAREDIR);
#endif

TupleStorage::IStorageInterface::CreateFullDirectory( std::string(szPath) + "Logs/" );

#ifdef LINUX_GNU
m_staticVersion->SetLabel(CLIENT_VERSION_PRETTY);
#endif
#ifdef WIN32
m_staticVersion->SetLabel(CLIENT_VERSION_PRETTYW);
#endif

m_spinCache->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
m_spinDecodeFps->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
m_spinDecodeFpsGold->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
m_spinDisplayFps->SetValidator(wxTextValidator(wxFILTER_NUMERIC));


wxRichTextAttr urlStyle;
urlStyle.SetTextColour(*wxBLUE);
urlStyle.SetFontUnderlined(true);

// Fill rich text control on About tab
m_AboutText->BeginSuppressUndo();
m_AboutText->BeginStyle(urlStyle);
m_AboutText->BeginURL(wxT("http://electricsheep.org/"));
m_AboutText->WriteText(wxT("Electric Sheep"));
m_AboutText->EndURL();
m_AboutText->EndStyle();

m_AboutText->WriteText(wxT(" is a collaborative abstract artwork founded by Scott Draves. The thousands of computers it runs on work together as a supercomputer to render the animations (each frame takes about an hour). Everyone watching can influence what everyone sees by voting, and the more popular sheep mate with each other and reproduce according to a genetic algorithm, hence the flock evolves to satisfy human desire.  You can also design your own sheep and upload them into the gene pool."));
m_AboutText->LineBreak();
m_AboutText->LineBreak();

m_AboutText->AppendText(wxT("You can buy high-quality limited edition artworks made with the Electric Sheep, as well as Blu-Rays, DVDs and T-shirts, from "));

m_AboutText->BeginStyle(urlStyle);
m_AboutText->BeginURL(wxT("http://scottdraves.com/for-sale.html"));
m_AboutText->WriteText(wxT("ScottDraves.com"));
m_AboutText->EndURL();
m_AboutText->EndStyle();

m_AboutText->WriteText(wxT(". Proceeds from these sales keep the network free to the public."));
m_AboutText->LineBreak();
m_AboutText->LineBreak();
m_AboutText->WriteText(wxT("Programmed by an "));

m_AboutText->BeginStyle(urlStyle);
m_AboutText->BeginURL(wxT("http://community.electricsheep.org/credits"));
m_AboutText->WriteText(wxT("Open Source team"));
m_AboutText->EndURL();
m_AboutText->EndStyle();

m_AboutText->WriteText(wxT(" from all over the world."));

m_AboutText->EndSuppressUndo();

// Fill rich text control on Gold tab (promo text)
m_PromoText->BeginSuppressUndo();
m_PromoText->Clear();
m_PromoText->WriteText(wxT("Upgrade your account to gold membership to get higher quality sheep.  Learn more "));

m_PromoText->BeginStyle(urlStyle);
m_PromoText->BeginURL(wxT("http://electricsheep.org/gold"));
m_PromoText->WriteText(wxT("online"));
m_PromoText->EndURL();
m_PromoText->EndStyle();
m_PromoText->WriteText(wxT("."));

m_PromoText->EndSuppressUndo();

LoadSettings();
}

void electricsheepguiMyDialog2::OnDialogClose( wxCloseEvent& event )
{
	this->Destroy();
}

void* LoginThread::Entry()
{
	wxMilliSleep(1000);
	if (sMainDialog->m_TestLogin)
		return 0;
	sMainDialog->LoginTest();
	return 0;
}

void electricsheepguiMyDialog2::OnIdle( wxIdleEvent& event )
{
	if (m_TestLogin)
	{
		if (m_LoginThread == NULL)
		{
			m_LoginThread = new LoginThread();
			m_LoginThread->Create();
			m_LoginThread->Run();
		}
		else
		{
			if (!m_LoginThread->IsRunning())
			{
				m_TestLogin = false;
				m_LoginThread->Delete();
				delete m_LoginThread;
				m_LoginThread = new LoginThread();

				m_LoginThread->Create();
				m_LoginThread->Run();
			}
		}
	}
	event.Skip();
}

void electricsheepguiMyDialog2::OnRunClick( wxCommandEvent& event )
{
	SaveSettings();
	//::Base::RecreateProcess(std::string("-x"));
#ifndef LINUX_GNU
	wxString windir;
	wxGetEnv(L"WINDIR", &windir);
	wxExecute(windir + L"\\es.scr -x");
#else
	wxExecute("electricsheep");
#endif
}

void electricsheepguiMyDialog2::OnHelpClick( wxCommandEvent& event )
{
	wxLaunchDefaultBrowser(CLIENT_HELP_LINK);
}

void electricsheepguiMyDialog2::OnDrupalNameTextEnter( wxCommandEvent& event )
{
//m_staticText6->SetLabel("...password changed, press 'Login' to verify...");
m_CreateAccountButton->Enable( true );

g_Settings()->Set("settings.content.registered", false);

g_Settings()->Set("settings.content.password", std::string(m_textDrupalPassword->GetValue()));
g_Settings()->Set("settings.generator.nickname", std::string(m_textDrupalName->GetValue()));

g_Settings()->Set("settings.content.password_md5",
				  computeMD5(
				  g_Settings()->Get("settings.content.password", std::string("")) +
				  "sh33p" +
				  g_Settings()->Get("settings.generator.nickname", std::string(""))
				  )
				  );
m_TestLogin = true;
}

void electricsheepguiMyDialog2::OnDrupalPasswordTextEnter( wxCommandEvent& event )
{
//m_staticText6->SetLabel("...password changed, press 'Login' to verify...");
m_CreateAccountButton->Enable( true );
g_Settings()->Set("settings.content.registered", false);
g_Settings()->Set("settings.content.password", std::string(m_textDrupalPassword->GetValue()));
g_Settings()->Set("settings.generator.nickname", std::string(m_textDrupalName->GetValue()));
g_Settings()->Set("settings.content.password_md5",
				  computeMD5(
				  g_Settings()->Get("settings.content.password", std::string("")) +
				  "sh33p" +
				  g_Settings()->Get("settings.generator.nickname", std::string(""))
				  )
				  );
m_TestLogin = true;
}

void electricsheepguiMyDialog2::OnCreateClick( wxCommandEvent& event )
{
::wxLaunchDefaultBrowser(wxT("http://community.electricsheep.org/user/register"));
}

void electricsheepguiMyDialog2::OnUnlimitedCacheCheck( wxCommandEvent& event )
{
	m_spinCache->Enable(true);
	g_Settings()->Set("settings.content.unlimited_cache", false);
	if (m_checkUnlimitedCache->IsChecked())
	{
		g_Settings()->Set("settings.content.unlimited_cache", true);
		m_spinCache->Enable(false);
	}
}

void electricsheepguiMyDialog2::OnContentDirChanged( wxFileDirPickerEvent& event )
{
	g_Settings()->Set("settings.content.sheepdir", std::string(event.GetPath()) );
#ifndef LINUX_GNU
	wxString contentdir = event.GetPath();
	if (IsUserAnAdmin())
	{
		wxExecute(wxString(L"SETACL.EXE -on \"") + contentdir + wxString(L"\" -ot file -actn ace -ace \"n:S-1-1-0;p:full,write_dacl;s:y;\""));
	} else
	{
		wxMessageBox(wxT("This action requires administrator rights"), wxT("Program is not currently running as admin"), wxICON_INFORMATION);
		wxExecute(wxString(L"SETACL.EXE -on \"") + contentdir + wxString(L"\" -ot file -actn ace -ace \"n:S-1-1-0;p:full,write_dacl;s:y;\""));
	}
#endif
}

void electricsheepguiMyDialog2::OnOpenClick( wxCommandEvent& event )
{
#ifndef LINUX_GNU
	wxExecute(L"explorer " + m_dirContent->GetPath());
#else
	int iReturn = wxExecute(L"nautilus " + m_dirContent->GetPath());
	if ( iReturn == -1 ) wxExecute(L"dolphin " + m_dirContent->GetPath());
#endif
}

void electricsheepguiMyDialog2::OnProxyTextEnter( wxCommandEvent& event )
{
	g_Settings()->Set( "settings.content.proxy", std::string(m_textProxyHost->GetValue()));
	if (m_textProxyHost->GetValue() != wxT(""))
		g_Settings()->Set( "settings.content.use_proxy", true);
	else
		g_Settings()->Set( "settings.content.use_proxy", false);
}

void electricsheepguiMyDialog2::OnProxyUserNameEnter( wxCommandEvent& event )
{
	g_Settings()->Set( "settings.content.proxy_username", std::string(m_textProxyUser->GetValue()));
}

void electricsheepguiMyDialog2::OnProxyPasswordEnter( wxCommandEvent& event )
{
	g_Settings()->Set( "settings.content.proxy_password", std::string(m_textProxyPassword->GetValue()));
}

void electricsheepguiMyDialog2::OnPromoTextURL( wxTextUrlEvent& event )
{
	wxLaunchDefaultBrowser(event.GetString());
}

void electricsheepguiMyDialog2::OnGoldUnlimitedCacheCheck( wxCommandEvent& event )
{
	m_spinGoldCache->Enable(true);
	g_Settings()->Set("settings.content.unlimited_cache_gold", false);
	if (m_checkGoldUnlimitedCache->IsChecked())
	{
		g_Settings()->Set("settings.content.unlimited_cache_gold", true);
		m_spinGoldCache->Enable(false);
	}
}

void electricsheepguiMyDialog2::OnAboutUrl( wxTextUrlEvent& event )
{
	wxLaunchDefaultBrowser(event.GetString());
}

void electricsheepguiMyDialog2::OnClickOk( wxCommandEvent& event )
{
	SaveSettings();
	this->Destroy();
}

void electricsheepguiMyDialog2::OnCancelClick( wxCommandEvent& event )
{
	this->Destroy();
}
