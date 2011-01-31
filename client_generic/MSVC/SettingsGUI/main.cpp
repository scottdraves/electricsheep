#include "main.h"
#include <wx/msgdlg.h>
#include "electricsheepguiMyDialog2.h"

#ifdef LINUX_GNU
#include "../../Runtime/sheep_logo.xpm"
#endif

IMPLEMENT_APP(wxWidgetsApp)

wxWidgetsApp::wxWidgetsApp()
{
}

wxWidgetsApp::~wxWidgetsApp()
{
}

bool wxWidgetsApp::OnInit()
{
    m_dialog = new electricsheepguiMyDialog2((wxWindow*)NULL);
#ifdef WIN32
	m_dialog->SetIcon(wxString(L"0"));
#endif
#ifdef LINUX_GNU
	wxIcon myicon = wxIcon(eslogo);
	m_dialog->SetIcon(myicon);
#endif
	m_dialog->Centre();
	m_dialog->Show(true);
	SetTopWindow(m_dialog);
	return true;
}
