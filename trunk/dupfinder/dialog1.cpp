/*
 *  dupfinder - search for duplicate files
 *  Copyright (C) 2007-2008 Matthias Boehm
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include "stdinc.h"
#include "dialog1.h"
#include "dialog2.h"
#include "os_cc_specific.h"


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
	ID_RMALL
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
	EVT_TEXT_ENTER(ID_DIRNAME, 	DupFinderDlg::OnDirNameEnter)
	EVT_BUTTON(ID_RMALL, 		DupFinderDlg::OnRemoveAll)
	EVT_INIT_DIALOG(		DupFinderDlg::OnInitDialog)
	EVT_BUTTON(wxID_ABOUT, 		DupFinderDlg::OnAbout)
END_EVENT_TABLE()



DupFinderDlg::DupFinderDlg(wxWindow * parent) 
	: wxDialog(parent, -1, _T("DupFinder"), 
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

	CreateControls();

	InitControls();

	CenterOnScreen();

	ffi.pFilesBySize = new multiset_fileinfosize;
}

DupFinderDlg::~DupFinderDlg() {
	int i;
	int count = wDirList->GetItemCount();

	// delete item related memory
	for(i = 0; i < count; i++) {
		// delete item <item>
		list<pathinfo>::iterator *pit = 
			(list<pathinfo>::iterator *)wDirList->GetItemData(i);
		
		delete pit;
	}
}

void DupFinderDlg::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *dirsizer = new wxStaticBoxSizer(wxVERTICAL, this, _T("Direc&tories"));

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
			wxSize(wxDefaultSize.GetWidth(), 200), wxBORDER_SUNKEN | wxLC_REPORT), 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	dirrow1->Add(
		new wxStaticText(this, wxID_STATIC, _T("&Directory: ")),
		0,
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow1->Add(
		wDirName = new wxTextCtrl(this, ID_DIRNAME, _T(""), wxDefaultPosition, 
			wxDefaultSize, wxTE_PROCESS_ENTER), 
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
		new wxButton(this, ID_RMDIR, _T("&Remove")), 	
		0, 
		wxTOPLEFT | wxBOTTOM, 
		10);

	dirrow3->Add(
		new wxButton(this, ID_RMALL, _T("Rem&ove all")), 
		0, 
		wxTOPLEFTRIGHT | wxBOTTOM, 
		10);

	controlssizer->Add(
		new wxButton(this, wxID_ABOUT, _T("A&bout...")), 
		0, 
		wxALIGN_LEFT, 
		10);

	controlssizer->AddStretchSpacer(1);

	controlssizer->Add(
		new wxButton(this, wxID_OK, _T("&Go! ")), 
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
		wxALL | wxEXPAND, 
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
	
	// set focus
	wDirName->SetFocus();
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

	// Go button only activated if there is at least
	// one path in the list

	FindWindow(wxID_OK)->Enable(
		ffi.paths.size() > 0
	);

	// RemoveAll enabled if there's something in the list
	FindWindow(ID_RMALL)->Enable(
		ffi.paths.size() > 0
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
	
	/*
	list<pathinfo>::iterator it;

	for(it = ffi.paths.begin(); it != ffi.paths.end(); it++) {
		wxMessageBox(it->path);
	}
	*/

	this->Hide();
	DupFinderDlg2 *progress = new DupFinderDlg2(ffi, this);	

	progress->Show();
	wxTheApp->SetTopWindow(progress);

}

static wxULongLong_t StrToULongLong(const wxString & str) {
	
	// no test whether it's a valid string
	// not necessary because of Validator
	wxULongLong_t value, lastvalue;
	int size = str.Length();
	int i;
	
	value = lastvalue = 0;
	for(i = 0; i < size; i++) {
		value *= 10;
		if(str[i] < '0' || str[i] > '9') {
			break;
		}
		value += str[i]-'0';
		// test for overflow
		if(lastvalue > value) {
			return 0;
		}
		lastvalue = value;
	}

	return value;
}

void DupFinderDlg::OnDirAdd(wxCommandEvent &WXUNUSED(event)) {
	pathinfo pi;
	wxULongLong_t minsize;
	bool bResult;
	
	pi.path = wDirName->GetValue();

	if(!wxFileName::DirExists(pi.path)) {
		return;
	}


	bResult = wMinSize->GetValue().ToULongLong(&minsize);
	/* does mingw also support 64-bit parsing ?
	 * if not, i have to change the upper to ....ToULong(..)
	 */
	if(!bResult) {
		// this is ONLY the case, if the number is a) too big
		// or b) we are using mingw
		minsize = StrToULongLong(wMinSize->GetValue());
	}
	
	pi.nMaxFileSizeIgnore = minsize;
	pi.bGoIntoSubDirs = wRecursive->GetValue();
	pi.bSearchHidden = wHidden->GetValue();
	pi.Mask = wMaskEnable->GetValue() ? wMask->GetValue() : wxString(_T(""));

	AddDir(pi);

	UpdateView();

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

	UpdateView();
}

void DupFinderDlg::OnGetDir(wxCommandEvent &WXUNUSED(event)) {
	wxDirDialog dirch(this);

	dirch.SetWindowStyle(wxDD_DIR_MUST_EXIST);

	dirch.SetPath(wDirName->GetValue());
	// dirch.SetMessage(
	int ret = dirch.ShowModal();

	if(ret == wxID_OK) {
		wDirName->SetValue(dirch.GetPath());
	}
}

void DupFinderDlg::OnRemoveAll(wxCommandEvent &WXUNUSED(event)) {
	int i;
	int count = wDirList->GetItemCount();

	for(i = 0; i < count; i++) {
		delete (list<pathinfo>::iterator *)wDirList->GetItemData(i);
	}

	ffi.paths.clear();

	wDirList->DeleteAllItems();

	UpdateView();

}

void DupFinderDlg::OnInitDialog(wxInitDialogEvent &event) {
	wxDialog::OnInitDialog(event);

	UpdateView();
}

void DupFinderDlg::OnAbout(wxCommandEvent &WXUNUSED(event)) {
	wxAboutDialogInfo info;

	info.AddDeveloper(_T("Matthias Boehm"));
	info.SetCopyright(_T("(c) Matthias Boehm 2008"));
	info.SetDescription(_T("Find duplicate files"));
	info.SetName(_T("DupFinder"));
	info.SetVersion(_T("gui 0.20"));
	
#ifdef __MINGW32_VERSION
	wxMessageBox(_T("DupFinder gui 0.20\nFind duplicate files\n\nCopyright Matthias Boehm 2008"));
#else
	// mingw has problems with this
	wxAboutBox(info);
#endif


}

void DupFinderDlg::ReturnToMe()
{
	CleanUp();
	Show();
}

void DupFinderDlg::CleanUp()
{
	multiset_fileinfosize_it it;
	list<fileinfo>::iterator it2, it4;
	list<fileinfoequal>::iterator it3;
	multiset_fileinfosize &sortedbysize = *ffi.pFilesBySize;

	// set<filedata *> deleted;

	// delete sortedbysize
	// are there memory leaks if i don't delete 
	// the equalfiles-list?
	for(it = sortedbysize.begin(); it != sortedbysize.end(); it++) {
		for(it2 = unconst(*it).files.begin(); 
			it2 != it->files.end();
			it2++) {
			/* if(it2->data) {
				deleted.insert(it2->data);
			} */
			erase(*it2);
		}
		// memory leaks, memory leaks, memory leaks.....
		/* for(it3 = (*it)->equalfiles.begin(); 
			it3 != (*it)->equalfiles.end();
			it3++) {
			for(it4 = it3->files.begin(); 
				it4 != it3->files.end();
				it4++) {
				if(deleted.find(it4->data) != deleted.end()) {
					erase (*it4);
				}
			}
		} */

	}

	sortedbysize.clear();
}


