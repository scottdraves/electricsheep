///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __config__
#define __config__

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/radiobox.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/notebook.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MyDialog2
///////////////////////////////////////////////////////////////////////////////
class MyDialog2 : public wxDialog 
{
	private:
	protected:
		wxStaticBitmap* m_bitmap12;
		wxStaticText* m_staticText4;
		wxStaticText* m_staticVersion;
		wxButton* m_RunButton;
		wxButton* m_HelpButton;
		wxNotebook* m_notebook2;
		wxPanel* m_Basic;
		wxStaticText* m_staticText22;
		wxStaticText* m_staticText3;
		wxTextCtrl* m_textDrupalName;
		wxStaticText* m_staticText41;
		wxTextCtrl* m_textDrupalPassword;
		wxStaticText* m_staticText25;
		wxStaticText* m_staticText6;
		wxButton* m_TestAccountButton;
		wxButton* m_CreateAccountButton;
		wxPanel* m_Flock;
		wxStaticText* m_staticTextFlockSize;
		wxStaticText* m_staticText7;
		wxTextCtrl* m_spinCache;
		wxStaticText* m_staticText10;
		wxCheckBox* m_checkUnlimitedCache;
		wxStaticText* m_staticTextGoldFlockSize;
		wxStaticText* m_staticText20;
		wxTextCtrl* m_spinGoldCache;
		wxStaticText* m_staticText21;
		wxCheckBox* m_checkGoldUnlimitedCache;
		wxPanel* m_Advanced;
		wxCheckBox* m_checkHttp;
		wxCheckBox* m_checkRenderFrames;
		wxCheckBox* m_checkMulticore;
		wxCheckBox* m_checkKeepFrames;
		wxCheckBox* m_checkNegVoteDeletes;
		wxCheckBox* m_QuietMode;
		wxCheckBox* m_DebugLog;
		wxCheckBox* m_checkAttributionPNG;
		wxDirPickerCtrl* m_dirContent;
		wxButton* m_buttonOpenContent;
		wxChoice* m_choicePlaybackMixingMode;
		wxPanel* m_Playback;
		wxStaticText* m_staticText8;
		wxTextCtrl* m_spinDecodeFps;
		wxStaticText* m_staticText11;
		wxStaticText* m_staticText9;
		wxSpinCtrl* m_spinRepeatLoops;
		wxStaticText* m_staticText12;
		wxCheckBox* m_SeamlessPlayback;
		wxPanel* m_Display;
		wxRadioBox* m_radioMultiDisplayMode;
		wxStaticText* m_staticText13;
		wxSpinCtrl* m_spinMonitor;
		wxStaticText* m_staticText111;
		wxTextCtrl* m_spinDisplayFps;
		wxStaticText* m_staticText19;
		wxRadioBox* m_radioDisplayMode;
		wxCheckBox* m_VerticalSync;
		wxCheckBox* m_ReverseDisplays;
		wxCheckBox* m_DirectDraw;
		wxPanel* m_Proxy;
		wxStaticText* m_staticText31;
		wxTextCtrl* m_textProxyHost;
		wxStaticText* m_staticText411;
		wxTextCtrl* m_textProxyUser;
		wxStaticText* m_staticText412;
		wxTextCtrl* m_textProxyPassword;
		wxPanel* m_About;
		wxRichTextCtrl* m_AboutText;
		wxButton* m_Ok;
		wxButton* m_Cancel;
		wxStaticBoxSizer* m_GoldFlockStaticSizer;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnDialogClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnIdle( wxIdleEvent& event ) { event.Skip(); }
		virtual void OnRunClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnHelpClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDrupalNameTextEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDrupalPasswordTextEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTestAccountButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCreateClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnUnlimitedCacheCheck( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnGoldUnlimitedCacheCheck( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnContentDirChanged( wxFileDirPickerEvent& event ) { event.Skip(); }
		virtual void OnOpenClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnProxyTextEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnProxyUserNameEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnProxyPasswordEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAboutUrl( wxTextUrlEvent& event ) { event.Skip(); }
		virtual void OnClickOk( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancelClick( wxCommandEvent& event ) { event.Skip(); }
		
		virtual void OnDecodeFpsTextUpdated( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDecodeFpsKillFocus( wxFocusEvent& event ) { event.Skip(); }
		virtual void OnPlayerFpsTextUpdated( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPlayerFpsKillFocus( wxFocusEvent& event ) { event.Skip(); }
		
		virtual void OnDialogCharHook( wxKeyEvent& e )
		{ 
			e.Skip(); 
			if (e.GetKeyCode() == WXK_ESCAPE)
			{
				this->Show( false );
				this->Destroy();
			}
		}
		virtual void LoginTest( wxIdleEvent& event ) { event.Skip(); }
		
	
	public:
		
		MyDialog2( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Electric Sheep Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,500 ), long style = wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER );
		~MyDialog2();
	
};

#endif //__config__
