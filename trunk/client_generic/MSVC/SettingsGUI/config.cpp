///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "config.h"

#ifdef LINUX_GNU
#include "../../Runtime/sheep_logo2.xpm"
#endif

///////////////////////////////////////////////////////////////////////////

MyDialog2::MyDialog2( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	wxImage::AddHandler(new wxPNGHandler);
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
#ifdef LINUX_GNU
	m_bitmap12 = new wxStaticBitmap( this, wxID_ANY, wxBitmap(eslogo), wxDefaultPosition, wxDefaultSize, 0 );
#else	
	m_bitmap12 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("logo.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
#endif
	bSizer2->Add( m_bitmap12, 0, wxALL, 5 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("Electric Sheep"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	m_staticText4->SetFont( wxFont( 20, 70, 93, 92, false, wxT("Nice") ) );
	
	bSizer2->Add( m_staticText4, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticVersion = new wxStaticText( this, wxID_ANY, wxT("version"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticVersion->Wrap( -1 );
	m_staticVersion->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 93, 90, false, wxEmptyString ) );
	
	bSizer2->Add( m_staticVersion, 0, wxALL, 5 );
	
	bSizer1->Add( bSizer2, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	m_RunButton = new wxButton( this, wxID_ANY, wxT("Run"), wxDefaultPosition, wxDefaultSize, 0 );
	m_RunButton->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	m_RunButton->SetToolTip( wxT("Start the client in standalone mode") );
	
	bSizer3->Add( m_RunButton, 0, wxALL, 1 );
	
	m_HelpButton = new wxButton( this, wxID_ANY, wxT("Help"), wxDefaultPosition, wxDefaultSize, 0 );
	m_HelpButton->SetToolTip( wxT("Launch the online help") );
	
	bSizer3->Add( m_HelpButton, 0, wxALL, 1 );
	
	bSizer1->Add( bSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	m_notebook2 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_Basic = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText22 = new wxStaticText( m_Basic, wxID_ANY, wxT("While sheep are playing, press F1 for onscreen help with interaction"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText22->Wrap( -1 );
	bSizer5->Add( m_staticText22, 0, wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( m_Basic, wxID_ANY, wxT("Sign In") ), wxVERTICAL );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText3 = new wxStaticText( m_Basic, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bSizer6->Add( m_staticText3, 0, wxALL, 5 );
	
	m_textDrupalName = new wxTextCtrl( m_Basic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 256,-1 ), wxTE_CENTRE );
	m_textDrupalName->SetMaxLength( 256 ); 
	m_textDrupalName->SetToolTip( wxT("your username from\nthe web site") );
	
	bSizer6->Add( m_textDrupalName, 0, wxALL|wxFIXED_MINSIZE, 5 );
	
	sbSizer1->Add( bSizer6, 0, wxALIGN_RIGHT, 5 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText41 = new wxStaticText( m_Basic, wxID_ANY, wxT("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	bSizer7->Add( m_staticText41, 0, wxALL, 5 );
	
	m_textDrupalPassword = new wxTextCtrl( m_Basic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 256,-1 ), wxTE_CENTRE|wxTE_PASSWORD );
	m_textDrupalPassword->SetMaxLength( 256 ); 
	m_textDrupalPassword->SetToolTip( wxT("the password from\nyour account") );
	
	bSizer7->Add( m_textDrupalPassword, 0, wxALL|wxFIXED_MINSIZE, 5 );
	
	sbSizer1->Add( bSizer7, 0, wxALIGN_RIGHT, 5 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer47;
	bSizer47 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText6 = new wxStaticText( m_Basic, wxID_ANY, wxT("...logging in..."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_staticText6->Wrap( -1 );
	m_staticText6->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 93, 90, false, wxEmptyString ) );
	m_staticText6->SetToolTip( wxT("Authentication status") );
	
	bSizer47->Add( m_staticText6, 0, wxALIGN_LEFT|wxALL, 5 );
	
	bSizer8->Add( bSizer47, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer481;
	bSizer481 = new wxBoxSizer( wxVERTICAL );
	
	m_CreateAccountButton = new wxButton( m_Basic, wxID_ANY, wxT("Create Account"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer481->Add( m_CreateAccountButton, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer8->Add( bSizer481, 1, wxEXPAND, 5 );
	
	sbSizer1->Add( bSizer8, 1, wxEXPAND, 5 );
	
	bSizer5->Add( sbSizer1, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( m_Basic, wxID_ANY, wxT("Local Flock") ), wxVERTICAL );
	
	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxVERTICAL );
	
	m_staticTextFlockSize = new wxStaticText( m_Basic, wxID_ANY, wxT("New sheep are created everyday.  When the screensaver runs,\nit tries to download them, and saves them on your hard disk,\ndeleting old ones to make room.  Login to get more sheep.\nIt is currently using 0 MBs"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextFlockSize->Wrap( -1 );
	bSizer35->Add( m_staticTextFlockSize, 0, 0, 5 );
	
	sbSizer2->Add( bSizer35, 0, wxALIGN_RIGHT, 5 );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText7 = new wxStaticText( m_Basic, wxID_ANY, wxT("Max disk space:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	bSizer9->Add( m_staticText7, 0, wxALL, 5 );
	
	m_spinCache = new wxTextCtrl( m_Basic, wxID_ANY, wxT("2000"), wxDefaultPosition, wxSize( 64,-1 ), wxTE_CENTRE );
	m_spinCache->SetMaxLength( 5 ); 
	m_spinCache->SetToolTip( wxT("After the maximum is reached,\nold sheep are deleted to make\nroom for new ones") );
	
	bSizer9->Add( m_spinCache, 0, wxALL|wxSHAPED, 3 );
	
	m_staticText10 = new wxStaticText( m_Basic, wxID_ANY, wxT("MB"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	bSizer9->Add( m_staticText10, 0, wxALL, 5 );
	
	sbSizer2->Add( bSizer9, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxVERTICAL );
	
	m_checkUnlimitedCache = new wxCheckBox( m_Basic, wxID_ANY, wxT("No Maximum"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_checkUnlimitedCache->SetToolTip( wxT("Do not delete old sheep") );
	
	bSizer28->Add( m_checkUnlimitedCache, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	sbSizer2->Add( bSizer28, 10, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer5->Add( sbSizer2, 0, wxEXPAND, 5 );
	
	m_Basic->SetSizer( bSizer5 );
	m_Basic->Layout();
	bSizer5->Fit( m_Basic );
	m_notebook2->AddPage( m_Basic, wxT("Basic"), true );
	m_Advanced = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_Advanced->SetToolTip( wxT("Advanced client settings") );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxHORIZONTAL );
	
	m_checkHttp = new wxCheckBox( m_Advanced, wxID_ANY, wxT("HTTP download"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_checkHttp->SetValue(true); 
	m_checkHttp->SetToolTip( wxT("Enables/Disables the downloading of\nnew sheep from the server via the\nHTTP protocol") );
	
	bSizer17->Add( m_checkHttp, 0, wxALL, 5 );
	
	bSizer31->Add( bSizer17, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxHORIZONTAL );
	
	m_checkRenderFrames = new wxCheckBox( m_Advanced, wxID_ANY, wxT("Render frames"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_checkRenderFrames->SetValue(true); 
	m_checkRenderFrames->SetToolTip( wxT("Enables/Disables your computer\nto contribute to the flock by\nrendering new frames") );
	
	bSizer18->Add( m_checkRenderFrames, 0, wxALL, 5 );
	
	bSizer31->Add( bSizer18, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxHORIZONTAL );
	
	m_checkMulticore = new wxCheckBox( m_Advanced, wxID_ANY, wxT("Multicore rendering"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_checkMulticore->Hide();
	m_checkMulticore->SetToolTip( wxT("Use all the processor cores available in your computer to render sheep frames.\nNote that this will take quite a bit of cpu, and the display may not be able to update at full framerate!") );
	
	bSizer19->Add( m_checkMulticore, 0, wxALL, 5 );
	
	bSizer31->Add( bSizer19, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );
	
	m_checkKeepFrames = new wxCheckBox( m_Advanced, wxID_ANY, wxT("Keep rendered frames"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_checkKeepFrames->SetValue(true); 
	m_checkKeepFrames->SetToolTip( wxT("Stores the rendered\nframes on your harddrive") );
	
	bSizer20->Add( m_checkKeepFrames, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer31->Add( bSizer20, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	m_checkNegVoteDeletes = new wxCheckBox( m_Advanced, wxID_ANY, wxT("Negative vote deletes sheep"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_checkNegVoteDeletes->SetValue(true); 
	m_checkNegVoteDeletes->SetToolTip( wxT("If you press the down arrow key,\nthen delete the currently displayed\nsheep in addition to notifying the\nserver not to make more sheep like it") );
	
	bSizer21->Add( m_checkNegVoteDeletes, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer31->Add( bSizer21, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer33->Add( bSizer31, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer271;
	bSizer271 = new wxBoxSizer( wxHORIZONTAL );
	
	m_QuietMode = new wxCheckBox( m_Advanced, wxID_ANY, wxT("Quiet mode"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_QuietMode->SetValue(true); 
	m_QuietMode->SetToolTip( wxT("Do not show connection\nerror messages") );
	
	bSizer271->Add( m_QuietMode, 0, wxALL, 5 );
	
	bSizer34->Add( bSizer271, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );
	
	m_DebugLog = new wxCheckBox( m_Advanced, wxID_ANY, wxT("Debugging log"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_DebugLog->SetToolTip( wxT("Log errors, warnings\nand info messages") );
	
	bSizer24->Add( m_DebugLog, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer34->Add( bSizer24, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxVERTICAL );
	
	m_checkAttributionPNG = new wxCheckBox( m_Advanced, wxID_ANY, wxT("Show Attribution"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_checkAttributionPNG->SetToolTip( wxT("Every 10 minutes show\na watermark for attribution") );
	
	bSizer32->Add( m_checkAttributionPNG, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer34->Add( bSizer32, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer33->Add( bSizer34, 1, wxEXPAND, 5 );
	
	bSizer13->Add( bSizer33, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( m_Advanced, wxID_ANY, wxT("Content directory") ), wxHORIZONTAL );
	
	m_dirContent = new wxDirPickerCtrl( m_Advanced, wxID_ANY, wxT(",90,90,-1,70,0"), wxT("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE|wxDIRP_DIR_MUST_EXIST );
	sbSizer3->Add( m_dirContent, 0, wxALL, 5 );
	
	m_buttonOpenContent = new wxButton( m_Advanced, wxID_ANY, wxT("Open this folder"), wxDefaultPosition, wxDefaultSize, 0 );
	m_buttonOpenContent->SetToolTip( wxT("Opens the content directory\nin an external exporer window") );
	
	sbSizer3->Add( m_buttonOpenContent, 0, wxALL, 5 );
	
	bSizer13->Add( sbSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5 );
	
	bSizer12->Add( bSizer13, 0, wxALL|wxEXPAND, 5 );
	
	m_Advanced->SetSizer( bSizer12 );
	m_Advanced->Layout();
	bSizer12->Fit( m_Advanced );
	m_notebook2->AddPage( m_Advanced, wxT("Advanced"), false );
	m_Playback = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText8 = new wxStaticText( m_Playback, wxID_ANY, wxT("Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	bSizer10->Add( m_staticText8, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_spinDecodeFps = new wxTextCtrl( m_Playback, wxID_ANY, wxT("15"), wxDefaultPosition, wxSize( 32,-1 ), wxTE_CENTRE );
	m_spinDecodeFps->SetMaxLength( 2 ); 
	m_spinDecodeFps->SetToolTip( wxT("Frames per second (FPS)") );
	
	bSizer10->Add( m_spinDecodeFps, 0, wxALIGN_RIGHT|wxALL|wxSHAPED, 3 );
	
	m_staticText11 = new wxStaticText( m_Playback, wxID_ANY, wxT("FPS"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	bSizer10->Add( m_staticText11, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer40->Add( bSizer10, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText9 = new wxStaticText( m_Playback, wxID_ANY, wxT("Repeat loops"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	bSizer11->Add( m_staticText9, 0, wxALL, 5 );
	
	m_spinRepeatLoops = new wxSpinCtrl( m_Playback, wxID_ANY, wxT("2"), wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 100, 15 );
	m_spinRepeatLoops->SetToolTip( wxT("Play the sheep that are\nloops this many times") );
	
	bSizer11->Add( m_spinRepeatLoops, 0, wxALL|wxSHAPED, 3 );
	
	m_staticText12 = new wxStaticText( m_Playback, wxID_ANY, wxT("times"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	bSizer11->Add( m_staticText12, 0, wxALL, 5 );
	
	bSizer40->Add( bSizer11, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxHORIZONTAL );
	
	m_SeamlessPlayback = new wxCheckBox( m_Playback, wxID_ANY, wxT("Seamless playback"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_SeamlessPlayback->SetToolTip( wxT("avoid sheep that lead\nto hard cuts") );
	
	bSizer23->Add( m_SeamlessPlayback, 0, wxALL, 5 );
	
	bSizer40->Add( bSizer23, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_Playback->SetSizer( bSizer40 );
	m_Playback->Layout();
	bSizer40->Fit( m_Playback );
	m_notebook2->AddPage( m_Playback, wxT("Playback"), false );
	m_Display = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer77;
	bSizer77 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString m_radioMultiDisplayModeChoices[] = { wxT("Cloned"), wxT("Independent"), wxT("Single") };
	int m_radioMultiDisplayModeNChoices = sizeof( m_radioMultiDisplayModeChoices ) / sizeof( wxString );
	m_radioMultiDisplayMode = new wxRadioBox( m_Display, wxID_ANY, wxT("Multi Monitor Mode"), wxDefaultPosition, wxDefaultSize, m_radioMultiDisplayModeNChoices, m_radioMultiDisplayModeChoices, 1, wxRA_SPECIFY_ROWS );
	m_radioMultiDisplayMode->SetSelection( 0 );
	m_radioMultiDisplayMode->SetToolTip( wxT("How to handle\nmultiple screens") );
	
	bSizer25->Add( m_radioMultiDisplayMode, 0, wxALL|wxEXPAND, 5 );
	
	bSizer77->Add( bSizer25, 0, wxALIGN_RIGHT, 5 );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText13 = new wxStaticText( m_Display, wxID_ANY, wxT("Display Monitor"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	bSizer14->Add( m_staticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_spinMonitor = new wxSpinCtrl( m_Display, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 0, 3, 0 );
	m_spinMonitor->SetToolTip( wxT("If you have multiple monitors and\nare in single mode, then display on\nthis monitor") );
	
	bSizer14->Add( m_spinMonitor, 0, wxALL, 5 );
	
	m_staticText111 = new wxStaticText( m_Display, wxID_ANY, wxT("Display speed"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText111->Wrap( -1 );
	bSizer14->Add( m_staticText111, 3, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_spinDisplayFps = new wxTextCtrl( m_Display, wxID_ANY, wxT("60"), wxDefaultPosition, wxSize( 32,-1 ), wxTE_CENTRE );
	m_spinDisplayFps->SetMaxLength( 3 ); 
	m_spinDisplayFps->SetToolTip( wxT("When Interpolation is Linear or\nCubic then the video is upsampled\nto this rate") );
	
	bSizer14->Add( m_spinDisplayFps, 0, wxALL, 5 );
	
	m_staticText19 = new wxStaticText( m_Display, wxID_ANY, wxT("FPS"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19->Wrap( -1 );
	bSizer14->Add( m_staticText19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer77->Add( bSizer14, 0, wxALIGN_RIGHT, 5 );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxHORIZONTAL );
	
	wxString m_radioDisplayModeChoices[] = { wxT("Off"), wxT("Linear"), wxT("Cubic") };
	int m_radioDisplayModeNChoices = sizeof( m_radioDisplayModeChoices ) / sizeof( wxString );
	m_radioDisplayMode = new wxRadioBox( m_Display, wxID_ANY, wxT("Interpolation"), wxDefaultPosition, wxDefaultSize, m_radioDisplayModeNChoices, m_radioDisplayModeChoices, 1, wxRA_SPECIFY_ROWS );
	m_radioDisplayMode->SetSelection( 1 );
	m_radioDisplayMode->SetToolTip( wxT("Linear and Cubic interpolation\nallow display speed (above) to\nbe higher than decode speed\n(under the Playback tab)") );
	
	bSizer16->Add( m_radioDisplayMode, 0, wxALIGN_RIGHT|wxALL|wxEXPAND, 5 );
	
	bSizer77->Add( bSizer16, 0, wxALIGN_RIGHT, 5 );
	
	wxBoxSizer* bSizer331;
	bSizer331 = new wxBoxSizer( wxVERTICAL );
	
	m_VerticalSync = new wxCheckBox( m_Display, wxID_ANY, wxT("V-Sync"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_VerticalSync->SetToolTip( wxT("Off to improve multiple monitor\ndisplay performance, On to\nprevent image tearing") );
	
	bSizer331->Add( m_VerticalSync, 0, wxALL, 5 );
	
	bSizer77->Add( bSizer331, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer341;
	bSizer341 = new wxBoxSizer( wxVERTICAL );
	
	m_ReverseDisplays = new wxCheckBox( m_Display, wxID_ANY, wxT("Reverse monitor order"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_ReverseDisplays->SetToolTip( wxT("Experimental switch to\nincrease multimon\nperformance") );
	
	bSizer341->Add( m_ReverseDisplays, 0, wxALL, 5 );
	
	bSizer77->Add( bSizer341, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxHORIZONTAL );
	
	m_DirectDraw = new wxCheckBox( m_Display, wxID_ANY, wxT("DirectDraw mode"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_DirectDraw->SetToolTip( wxT("only for compatibility with\nvery old graphics chips, do\nNOT enable this except to\nfix instant-crashing") );
	
	bSizer27->Add( m_DirectDraw, 0, wxALL, 5 );
	
	bSizer77->Add( bSizer27, 1, wxALIGN_RIGHT|wxALL, 5 );
	
	m_Display->SetSizer( bSizer77 );
	m_Display->Layout();
	bSizer77->Fit( m_Display );
	m_notebook2->AddPage( m_Display, wxT("Display"), false );
	m_Proxy = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer61;
	bSizer61 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText31 = new wxStaticText( m_Proxy, wxID_ANY, wxT("Hostname:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText31->Wrap( -1 );
	bSizer61->Add( m_staticText31, 0, wxALL, 5 );
	
	m_textProxyHost = new wxTextCtrl( m_Proxy, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 256,-1 ), wxTE_CENTRE );
	m_textProxyHost->SetMaxLength( 256 ); 
	bSizer61->Add( m_textProxyHost, 0, wxALL|wxFIXED_MINSIZE, 5 );
	
	bSizer36->Add( bSizer61, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText411 = new wxStaticText( m_Proxy, wxID_ANY, wxT("User:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText411->Wrap( -1 );
	bSizer71->Add( m_staticText411, 0, wxALL, 5 );
	
	m_textProxyUser = new wxTextCtrl( m_Proxy, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 256,-1 ), wxTE_CENTRE|wxTE_PASSWORD );
	m_textProxyUser->SetMaxLength( 256 ); 
	bSizer71->Add( m_textProxyUser, 0, wxALL|wxFIXED_MINSIZE, 5 );
	
	bSizer36->Add( bSizer71, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer72;
	bSizer72 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText412 = new wxStaticText( m_Proxy, wxID_ANY, wxT("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText412->Wrap( -1 );
	bSizer72->Add( m_staticText412, 0, wxALL, 5 );
	
	m_textProxyPassword = new wxTextCtrl( m_Proxy, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 256,-1 ), wxTE_CENTRE|wxTE_PASSWORD );
	m_textProxyPassword->SetMaxLength( 256 ); 
	bSizer72->Add( m_textProxyPassword, 0, wxALL|wxFIXED_MINSIZE, 5 );
	
	bSizer36->Add( bSizer72, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_Proxy->SetSizer( bSizer36 );
	m_Proxy->Layout();
	bSizer36->Fit( m_Proxy );
	m_notebook2->AddPage( m_Proxy, wxT("Proxy"), false );
	m_Gold = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxVERTICAL );
	
	m_PromoText = new wxRichTextCtrl( m_Gold, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 40), wxTE_AUTO_URL|wxTE_READONLY|wxVSCROLL|wxHSCROLL|wxNO_BORDER|wxWANTS_CHARS );
	bSizer42->Add( m_PromoText, 1, wxEXPAND | wxALL, 5 );
	
	bSizer41->Add( bSizer42, 1, wxEXPAND, 5 );
	
	wxString m_radioPlaybackMixingModeChoices[] = { wxT("if there are any gold sheep then play only gold sheep"), wxT("free sheep only"), wxT("play all sheep") };
	int m_radioPlaybackMixingModeNChoices = sizeof( m_radioPlaybackMixingModeChoices ) / sizeof( wxString );
	m_radioPlaybackMixingMode = new wxRadioBox( m_Gold, wxID_ANY, wxT("Playback mixing mode"), wxDefaultPosition, wxDefaultSize, m_radioPlaybackMixingModeNChoices, m_radioPlaybackMixingModeChoices, 1, wxRA_SPECIFY_COLS );
	m_radioPlaybackMixingMode->SetSelection( 0 );
	bSizer41->Add( m_radioPlaybackMixingMode, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( m_Gold, wxID_ANY, wxT("Gold Flock") ), wxVERTICAL );
	
	wxBoxSizer* bSizer44;
	bSizer44 = new wxBoxSizer( wxVERTICAL );
	
	m_staticTextGoldFlockSize1 = new wxStaticText( m_Gold, wxID_ANY, wxT("It is currently using 0 MBs"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextGoldFlockSize1->Wrap( -1 );
	bSizer44->Add( m_staticTextGoldFlockSize1, 0, 0, 5 );
	
	sbSizer4->Add( bSizer44, 0, wxALIGN_RIGHT, 5 );
	
	wxBoxSizer* bSizer43;
	bSizer43 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText20 = new wxStaticText( m_Gold, wxID_ANY, wxT("Max disk space:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText20->Wrap( -1 );
	bSizer43->Add( m_staticText20, 0, wxALL, 5 );
	
	m_spinGoldCache = new wxTextCtrl( m_Gold, wxID_ANY, wxT("2000"), wxDefaultPosition, wxSize( 64,-1 ), wxTE_CENTRE );
	m_spinGoldCache->SetMaxLength( 5 ); 
	m_spinGoldCache->SetToolTip( wxT("After the maximum is reached,\nold sheep are deleted to make\nroom for new ones") );
	
	bSizer43->Add( m_spinGoldCache, 0, wxALL|wxSHAPED, 3 );
	
	m_staticText21 = new wxStaticText( m_Gold, wxID_ANY, wxT("MB"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText21->Wrap( -1 );
	bSizer43->Add( m_staticText21, 0, wxALL, 5 );
	
	sbSizer4->Add( bSizer43, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	wxBoxSizer* bSizer281;
	bSizer281 = new wxBoxSizer( wxVERTICAL );
	
	m_checkGoldUnlimitedCache = new wxCheckBox( m_Gold, wxID_ANY, wxT("No Maximum"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_checkGoldUnlimitedCache->SetToolTip( wxT("Do not delete old sheep") );
	
	bSizer281->Add( m_checkGoldUnlimitedCache, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	sbSizer4->Add( bSizer281, 1, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer41->Add( sbSizer4, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer55;
	bSizer55 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText81 = new wxStaticText( m_Gold, wxID_ANY, wxT("Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	bSizer101->Add( m_staticText81, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_spinDecodeFpsGold = new wxTextCtrl( m_Gold, wxID_ANY, wxT("15"), wxDefaultPosition, wxSize( 32,-1 ), wxTE_CENTRE );
	m_spinDecodeFpsGold->SetMaxLength( 2 ); 
	m_spinDecodeFpsGold->SetToolTip( wxT("Frames per second (FPS)") );
	
	bSizer101->Add( m_spinDecodeFpsGold, 0, wxALIGN_RIGHT|wxALL|wxSHAPED, 3 );
	
	m_staticText112 = new wxStaticText( m_Gold, wxID_ANY, wxT("FPS"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText112->Wrap( -1 );
	bSizer101->Add( m_staticText112, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer55->Add( bSizer101, 1, wxALIGN_RIGHT, 5 );
	
	bSizer41->Add( bSizer55, 1, wxEXPAND, 5 );
	
	m_Gold->SetSizer( bSizer41 );
	m_Gold->Layout();
	bSizer41->Fit( m_Gold );
	m_notebook2->AddPage( m_Gold, wxT("Gold"), false );
	m_About = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxVERTICAL );
	
	m_AboutText = new wxRichTextCtrl( m_About, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_AUTO_URL|wxTE_READONLY|wxVSCROLL|wxHSCROLL|wxNO_BORDER|wxWANTS_CHARS );
	bSizer48->Add( m_AboutText, 1, wxEXPAND | wxALL, 5 );
	
	m_About->SetSizer( bSizer48 );
	m_About->Layout();
	bSizer48->Fit( m_About );
	m_notebook2->AddPage( m_About, wxT("About"), false );
	
	bSizer4->Add( m_notebook2, 1, wxALL, 5 );
	
	bSizer1->Add( bSizer4, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALIGN_TOP|wxALL, 5 );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );
	
	m_Ok = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_Ok->SetDefault(); 
	m_Ok->SetToolTip( wxT("Confirm all changes and exit") );
	
	bSizer22->Add( m_Ok, 0, wxALL, 5 );
	
	m_Cancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_Cancel->SetToolTip( wxT("Discard all changes and exit") );
	
	bSizer22->Add( m_Cancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	bSizer1->Add( bSizer22, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	this->SetSizerAndFit( bSizer1 );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MyDialog2::OnDialogClose ) );
	this->Connect( wxEVT_CHAR_HOOK, wxKeyEventHandler( MyDialog2::OnDialogCharHook ), NULL, this );
	this->Connect( wxEVT_IDLE, wxIdleEventHandler( MyDialog2::OnIdle ), NULL, this );
	m_RunButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnRunClick ), NULL, this );
	m_HelpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnHelpClick ), NULL, this );
	m_textDrupalName->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MyDialog2::OnDrupalNameTextEnter ), NULL, this );
	m_textDrupalPassword->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MyDialog2::OnDrupalPasswordTextEnter ), NULL, this );
	m_CreateAccountButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnCreateClick ), NULL, this );
	m_checkUnlimitedCache->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( MyDialog2::OnUnlimitedCacheCheck ), NULL, this );
	m_dirContent->Connect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( MyDialog2::OnContentDirChanged ), NULL, this );
	m_buttonOpenContent->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnOpenClick ), NULL, this );
	m_textProxyHost->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MyDialog2::OnProxyTextEnter ), NULL, this );
	m_textProxyUser->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MyDialog2::OnProxyUserNameEnter ), NULL, this );
	m_textProxyPassword->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MyDialog2::OnProxyPasswordEnter ), NULL, this );
	m_PromoText->Connect( wxEVT_COMMAND_TEXT_URL, wxTextUrlEventHandler( MyDialog2::OnPromoTextURL ), NULL, this );
	m_checkGoldUnlimitedCache->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( MyDialog2::OnGoldUnlimitedCacheCheck ), NULL, this );
	m_AboutText->Connect( wxEVT_COMMAND_TEXT_URL, wxTextUrlEventHandler( MyDialog2::OnAboutUrl ), NULL, this );
	m_Ok->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnClickOk ), NULL, this );
	m_Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnCancelClick ), NULL, this );
}

MyDialog2::~MyDialog2()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MyDialog2::OnDialogClose ) );
	this->Disconnect( wxEVT_CHAR_HOOK, wxKeyEventHandler( MyDialog2::OnDialogCharHook ), NULL, this );
	this->Disconnect( wxEVT_IDLE, wxIdleEventHandler( MyDialog2::OnIdle ), NULL, this );
	m_RunButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnRunClick ), NULL, this );
	m_HelpButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnHelpClick ), NULL, this );
	m_textDrupalName->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MyDialog2::OnDrupalNameTextEnter ), NULL, this );
	m_textDrupalPassword->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MyDialog2::OnDrupalPasswordTextEnter ), NULL, this );
	m_CreateAccountButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnCreateClick ), NULL, this );
	m_checkUnlimitedCache->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( MyDialog2::OnUnlimitedCacheCheck ), NULL, this );
	m_dirContent->Disconnect( wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler( MyDialog2::OnContentDirChanged ), NULL, this );
	m_buttonOpenContent->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnOpenClick ), NULL, this );
	m_textProxyHost->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MyDialog2::OnProxyTextEnter ), NULL, this );
	m_textProxyUser->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MyDialog2::OnProxyUserNameEnter ), NULL, this );
	m_textProxyPassword->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MyDialog2::OnProxyPasswordEnter ), NULL, this );
	m_PromoText->Disconnect( wxEVT_COMMAND_TEXT_URL, wxTextUrlEventHandler( MyDialog2::OnPromoTextURL ), NULL, this );
	m_checkGoldUnlimitedCache->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( MyDialog2::OnGoldUnlimitedCacheCheck ), NULL, this );
	m_AboutText->Disconnect( wxEVT_COMMAND_TEXT_URL, wxTextUrlEventHandler( MyDialog2::OnAboutUrl ), NULL, this );
	m_Ok->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnClickOk ), NULL, this );
	m_Cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MyDialog2::OnCancelClick ), NULL, this );
	
}
