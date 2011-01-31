#pragma once

#include <wx/wx.h>
#include "electricsheepguiMyDialog2.h"

class wxWidgetsApp : public wxApp
{
public:
    wxWidgetsApp();
    virtual ~wxWidgetsApp();
    virtual bool OnInit();
private:
	electricsheepguiMyDialog2 *m_dialog;
};

DECLARE_APP(wxWidgetsApp)