
#include "stdinc.h"
#include "dupfgui.h"

#define DUPFINDER_GUI
#include "dbl.cpp"

class DupFinderApp :public wxApp {
public:
	virtual bool OnInit();
	virtual int OnExit();

};
#ifdef _______
class DupFinderDlg : public wxDialog {
public:
	DupFinderDlg(wxWindow *, const wxString &, const wxPoint &, const wxSize &);

	void OnClose(wxCloseEvent &);
	void OnSize(wxSizeEvent &);

	void OnEscape(wxCommandEvent &);
	void OnOk(wxCommandEvent &);

	void OnDlgChange(wxCommandEvent &WXUNUSED(event)) { UpdateView(); }
	void OnListSelChange(wxListEvent &WXUNUSED(event)) { UpdateView(); }
	
	void InitControls();
	void UpdateView();
	void CreateControls();

	void UpdateList();

	void OnDirAdd(wxCommandEvent &);
	void OnDirRemove(wxCommandEvent &);

	void AddDir(const pathinfo  &);

	void OnGetDir(wxCommandEvent &);

private:
	wxListView *wDirList;
	wxTextCtrl *wDirName;
	wxCheckBox *wRecursive;
	wxCheckBox *wHidden;
	wxTextCtrl *wMinSize;
	wxCheckBox *wMaskEnable;
	wxTextCtrl *wMask;

	findfileinfo ffi;
	

	DECLARE_EVENT_TABLE()
};

enum {
	ID_DIRLIST = 1, 
	ID_GETDIR, 
	ID_ADDDIR, 
	ID_RMDIR, 
	ID_DIRNAME, 
	// options
	ID_RECURSIVE, 
	ID_HIDDEN, 
	ID_MASKENABLE, 
	ID_MASK, 
	ID_MINSIZE, 
};

BEGIN_EVENT_TABLE(DupFinderDlg, wxDialog)
	EVT_CLOSE(			DupFinderDlg::OnClose)
	EVT_SIZE(			DupFinderDlg::OnSize)
	EVT_BUTTON(wxID_CANCEL,		DupFinderDlg::OnEscape)
	EVT_BUTTON(wxID_OK, 		DupFinderDlg::OnOk)
	// the following is for UpdateView()
	EVT_TEXT(ID_DIRNAME, 		DupFinderDlg::OnDlgChange)
	EVT_CHECKBOX(ID_MASKENABLE,  	DupFinderDlg::OnDlgChange)
	EVT_LIST_ITEM_SELECTED(ID_DIRLIST, DupFinderDlg::OnListSelChange)
	EVT_LIST_ITEM_DESELECTED(ID_DIRLIST, DupFinderDlg::OnListSelChange)
	EVT_TEXT(ID_MINSIZE, 		DupFinderDlg::OnDlgChange)

	EVT_BUTTON(ID_ADDDIR, 		DupFinderDlg::OnDirAdd)
	EVT_BUTTON(ID_RMDIR, 		DupFinderDlg::OnDirRemove)
	EVT_BUTTON(ID_GETDIR, 		DupFinderDlg::OnGetDir)
END_EVENT_TABLE()


IMPLEMENT_APP(DupFinderApp)
#endif

bool DupFinderApp::OnInit() {

	#ifdef ____
	DupFinderDlg *maindlg = new DupFinderDlg (NULL, _T("DupFinder"), wxDefaultPosition, wxDefaultSize);

	maindlg->Show(true);
	SetTopWindow(maindlg);

	#endif

	return true;
}

int DupFinderApp::OnExit() {
	return 0;
}

#ifdef ___________

DupFinderDlg::DupFinderDlg(wxWindow * parent, const wxString &title, const wxPoint &pos, const wxSize &size) 
	: wxDialog(parent, -1, title, pos, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

	CreateControls();

	InitControls();
	UpdateView();

}

void DupFinderDlg::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *dirsizer = new wxStaticBoxSizer(wxVERTICAL, this, _T("Directories"));

	wxBoxSizer *dirrow1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *dirrow2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *dirrow3 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer *controlssizer = new wxBoxSizer(wxHORIZONTAL);

	const int wxTOPLEFT = wxLEFT | wxTOP;
	const int wxTOPLEFTRIGHT = wxLEFT | wxTOP | wxRIGHT;
	const int wxTOPRIGHT = wxRIGHT | wxTOP;

	/***************************** Dialog creation **********************************/

	topsizer->Add(
		new wxStaticText(this, wxID_STATIC, 
			wxString(_T("Step 1: \nEnter the directories for search and parameters for each diretory: ")) ), 
			0, 
			wxTOPLEFTRIGHT | wxEXPAND, 
			10 );

	dirsizer->Add(
		wDirList = new wxListView(this, ID_DIRLIST, wxDefaultPosition, 
			wxSize(wxDefaultSize.GetWidth(), 200)), 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	dirrow1->Add(
		new wxStaticText(this, wxID_STATIC, _T("&Directory: ")),
		0,
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow1->Add(
		wDirName = new wxTextCtrl(this, ID_DIRNAME, _T(""), wxDefaultPosition), 
		1, 
		wxTOPLEFT, 
		10);

	dirrow1->Add(
		new wxButton(this, ID_GETDIR, _T("&...")), 
		0, 
		wxTOPLEFT | wxRIGHT, 
		10);

	dirrow2->Add(
		wRecursive = new wxCheckBox(this, ID_RECURSIVE, _T("Include &subdirs")), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow2->Add( 
		wHidden = new wxCheckBox(this, ID_HIDDEN, _T("Include &hidden files")), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow2->Add(
		new wxStaticText(this, wxID_STATIC, _T("M&inimal file size: ")), 
		0,
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow2->Add(
		wMinSize = new wxTextCtrl(this, ID_MINSIZE, _T(""), wxDefaultPosition, 
			wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC) ), 
		1, 
		wxTOPLEFT | wxRIGHT, 
		10);

	dirrow3->Add(
		wMaskEnable = new wxCheckBox(this, ID_MASKENABLE, _T("Search &mask: ")), 
		0, 
		wxTOPLEFT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow3->Add(
		wMask = new wxTextCtrl(this, ID_MASK, _T("")), 
		1, 
		wxTOPLEFT | wxBOTTOM, 
		10);

	dirrow3->Add(
		new wxButton(this, ID_ADDDIR, _T("&Add")), 
		0, 
		wxTOPLEFT | wxBOTTOM, 
		10);

	dirrow3->Add(
		new wxButton(this, ID_RMDIR, _T("Re&move")), 	
		0, 
		wxTOPLEFTRIGHT | wxBOTTOM, 
		10);

	controlssizer->Add(
		new wxButton(this, wxID_OK, _T("&Start! ")), 
		0, 
		wxALIGN_RIGHT, 
		10);

	controlssizer->Add(
		new wxButton(this, wxID_CANCEL, _T("&Close")), 
		0, 
		wxALIGN_RIGHT | wxLEFT, 
		10);

	dirsizer->Add(dirrow1, 
		0, 
		wxTOPLEFT | wxEXPAND, 
		0);

	dirsizer->Add(dirrow2, 
		0, 
		wxTOPLEFT | wxEXPAND, 
		0);

	dirsizer->Add(dirrow3, 
		0, 
		wxTOPLEFT | wxEXPAND, 
		0);

	topsizer->Add(dirsizer,
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	topsizer->Add(controlssizer, 
		0, 
		wxALL | wxALIGN_RIGHT, 
		10); 
	

	SetSizer(topsizer);
	topsizer->SetSizeHints(this);

	/*************************************** dialog created ****************************/

}

void DupFinderDlg::InitControls() {

	// enable/disable/check/uncheck all checkboxes,...
	wRecursive->SetValue(true);
	wHidden->SetValue(false);
	wMaskEnable->SetValue(false);

	// add coloumns to dir list control
	// which order?...
	wDirList->InsertColumn(1, _T("Subdirs"), wxLIST_FORMAT_LEFT, 30);
	wDirList->InsertColumn(2, _T("Hidden"), wxLIST_FORMAT_LEFT, 30);
	wDirList->InsertColumn(3, _T("Path"), wxLIST_FORMAT_LEFT, 250);
	wDirList->InsertColumn(4, _T("Min size"), wxLIST_FORMAT_LEFT, 60);
	wDirList->InsertColumn(5, _T("Mask"), wxLIST_FORMAT_LEFT, 60);
	
}

void DupFinderDlg::UpdateView() {

	// disable/enable controls

	// mask text field enabled only if mask checkbox is checked
	wMask->Enable(wMaskEnable->GetValue());

	// add button only enabled if there's a (valid) dir in 
	// dir text control and there is a valid number/no number
	// in minsize field

	wxString dir = wDirName->GetValue();	

	FindWindow(ID_ADDDIR)->Enable(
		wxFileName::DirExists(dir)  && wMinSize->GetValue().IsNumber()
	);

	// remove button only if there is a selection 
	// in the dir list

	FindWindow(ID_RMDIR)->Enable(
		wDirList->GetFirstSelected() != -1
	);

}

void DupFinderDlg::OnClose(wxCloseEvent &WXUNUSED(event) ) {
	Destroy();
}

void DupFinderDlg::OnEscape(wxCommandEvent &WXUNUSED(event)) {
	Close();
}

void DupFinderDlg::OnSize(wxSizeEvent &WXUNUSED(event)) {
	wxSizer *pSizer = GetSizer();
	if(pSizer) {
		pSizer->SetDimension(0, 0, 
			GetClientSize().GetWidth(), GetClientSize().GetHeight());
	}
}

void DupFinderDlg::OnOk(wxCommandEvent &WXUNUSED(event)) {
	
	list<pathinfo>::iterator it;

	for(it = ffi.paths.begin(); it != ffi.paths.end(); it++) {
		wxMessageBox(it->path);
	}
}

void DupFinderDlg::OnDirAdd(wxCommandEvent &WXUNUSED(event)) {
	pathinfo pi;
	wxULongLong_t minsize;
	bool bResult;
	
	pi.path = wDirName->GetValue();
	bResult = wMinSize->GetValue().ToULongLong(&minsize);
	/* does mingw also support 64-bit parsing ?
	 * if not, i have to change the upper to ....ToULong(..)
	 */
	if(!bResult) {
		minsize = 0;
	}
		/* wxMessageBox(_T("You have to enter a number in the minimum size text field! \n"), 
			_T("Error"), wxOK | wxICON_ERROR, this);
		SetFocus(wMinSize);
		return; */
	pi.nMaxFileSizeIgnore = minsize;
	pi.bGoIntoSubDirs = wRecursive->GetValue();
	pi.bSearchHidden = wHidden->GetValue();
	pi.Mask = wMaskEnable->GetValue() ? wMask->GetValue() : _T("");

	AddDir(pi);	

}

void DupFinderDlg::AddDir(const pathinfo &pi)
{
	wxListItem c1, c2, c3, c4, c5;
	// c1.SetColumn(0); // don't set column, else you will get strange assert messages
	c2.SetColumn(1);
	c3.SetColumn(2);
	c4.SetColumn(3);
	c5.SetColumn(4);
	c1.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
	c2.SetMask(wxLIST_MASK_TEXT);
	c3.SetMask(wxLIST_MASK_TEXT);
	c4.SetMask(wxLIST_MASK_TEXT);
	c5.SetMask(wxLIST_MASK_TEXT);
	
	ffi.paths.push_back(pi);

	c1.SetData(new list<pathinfo>::iterator( --ffi.paths.end()) );

	c1.SetText(pi.bGoIntoSubDirs ? _T("x") : _T(""));
	c2.SetText(pi.bSearchHidden ? _T("x") : _T(""));
	c3.SetText(pi.path);
	c4.SetText(pi.nMaxFileSizeIgnore.ToString());
	c5.SetText(pi.Mask);

	// wDirList->InsertItem(0, _T("tmp"));	

	wDirList->InsertItem(c1);
	wDirList->SetItem(c2);
	wDirList->SetItem(c3);
	wDirList->SetItem(c4);
	wDirList->SetItem(c5);

}

void DupFinderDlg::OnDirRemove(wxCommandEvent &WXUNUSED(event)) {
	long item;

	while(true) {
		item = wDirList->GetFirstSelected();

		if(item == -1) {
			break;
		}

		// delete item <item>
		list<pathinfo>::iterator *pit = 
			(list<pathinfo>::iterator *)wDirList->GetItemData(item);
		
		ffi.paths.erase(*pit);
		delete pit;

		// ? delete all items ? 
		wDirList->DeleteItem(item);

		// item = wDirList->GetNextSelected();
	}
}

void DupFinderDlg::OnGetDir(wxCommandEvent &WXUNUSED(event)) {
	wxDirDialog dirch(this);

	dirch.SetPath(wDirName->GetValue());
	// dirch.SetMessage(
	int ret = dirch.ShowModal();

	if(ret == wxID_OK) {
		wDirName->SetValue(dirch.GetPath());
	}
}
#endif






















