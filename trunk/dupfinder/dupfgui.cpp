
#include "stdinc.h"
#include "dupfgui.h"

#define DUPFINDER_GUI
#include "dbl.cpp"

class DupFinderApp :public wxApp {
public:
	virtual bool OnInit();
	virtual int OnExit();

};

class DupFinderDlg : public wxDialog {
public:
	DupFinderDlg(wxWindow *, const wxString &, const wxPoint &, const wxSize &);

	void OnClose(wxCloseEvent &);
	void OnSize(wxSizeEvent &);

private:
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(DupFinderDlg, wxDialog)
	EVT_CLOSE(DupFinderDlg::OnClose)
	EVT_SIZE(DupFinderDlg::OnSize)
END_EVENT_TABLE()

IMPLEMENT_APP(DupFinderApp)

enum {
	ID_DIRLIST = 0, 
	ID_GETDIR, 
	ID_ADDDIR, 
	ID_RMDIR, 
	ID_RECURSIVE, 
	ID_DIRNAME
};


bool DupFinderApp::OnInit() {
	DupFinderDlg *maindlg = new DupFinderDlg (NULL, _T("DupFinder"), wxDefaultPosition, wxDefaultSize);

	maindlg->Show(true);
	SetTopWindow(maindlg);

	return true;
}

int DupFinderApp::OnExit() {
	return 0;
}

DupFinderDlg::DupFinderDlg(wxWindow * parent, const wxString &title, const wxPoint &pos, const wxSize &size) 
	: wxDialog(parent, -1, title, pos, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer *dirsizer = new wxStaticBoxSizer(wxVERTICAL, this, _T("Directories"));
	wxStaticBoxSizer *optsizer = new wxStaticBoxSizer(wxVERTICAL, this, _T("Options"));
	wxBoxSizer *diraddsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *controlssizer = new wxBoxSizer(wxHORIZONTAL);

	const int wxTOPLEFT = wxLEFT | wxTOP;
	const int wxTOPLEFTRIGHT = wxLEFT | wxTOP | wxRIGHT;
	const int wxTOPRIGHT = wxRIGHT | wxTOP;

	topsizer->Add(
		new wxStaticText(this, -1, 
			wxString(_T("Step 1: \nParameters for search: ")) ), 
			0, 
			wxTOPLEFTRIGHT | wxEXPAND, 
			10 );

	dirsizer->Add(
		new wxListBox(this, ID_DIRLIST, wxDefaultPosition), 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	diraddsizer->Add(
		new wxTextCtrl(this, ID_DIRNAME, _T(""), wxDefaultPosition /*, wxSize(150, wxDefaultSize.GetHeight()) */), 
		90, 
		wxTOPLEFT | wxBOTTOM, 
		10);

	diraddsizer->Add(
		new wxButton(this, ID_GETDIR, _T("&...")), 
		0, 
		wxTOPLEFT | wxBOTTOM, 
		10);

	diraddsizer->Add(
		new wxCheckBox(this, ID_RECURSIVE, _T("&Recursive")), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL | wxBOTTOM, 
		10);

	diraddsizer->Add(
		new wxButton(this, ID_ADDDIR, _T("&Add")), 
		0, 
		wxTOP | wxBOTTOM, 
		10);

	diraddsizer->Add(
		new wxButton(this, ID_RMDIR, _T("Re&move")), 	
		0, 
		wxTOPRIGHT | wxBOTTOM, 
		10);

	controlssizer->Add(
		new wxButton(this, -1, _T("&Start! ")), 
		0, 
		wxALIGN_RIGHT, 
		10);

	dirsizer->Add(diraddsizer, 
		0, 
		wxTOPLEFT | wxEXPAND, 
		0);

	topsizer->Add(dirsizer,
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	topsizer->Add(optsizer, 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	topsizer->Add(controlssizer, 
		0, 
		wxALL | wxALIGN_RIGHT, 
		10); 
	

	SetSizer(topsizer);
	topsizer->SetSizeHints(this);

}

void DupFinderDlg::OnClose(wxCloseEvent &WXUNUSED(event) ) {
	Destroy();
}

void DupFinderDlg::OnSize(wxSizeEvent &WXUNUSED(event)) {
	wxSizer *pSizer = GetSizer();
	if(pSizer) {
		pSizer->SetDimension(0, 0, 
			GetClientSize().GetWidth(), GetClientSize().GetHeight());
	}
}







