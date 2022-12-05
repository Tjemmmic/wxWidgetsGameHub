#pragma once
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

class App: public wxApp
{
public:
	App();
	bool OnInit();
	~App();

private:
	
};

DECLARE_APP(App); //wxGetApp()