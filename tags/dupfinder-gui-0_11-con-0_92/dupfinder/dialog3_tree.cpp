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

#include "dialog3.h"

DupFinderDlg3::DupFinderDlg3(wxWindow *parent, findfileinfo &_ffi) 
	: wxDialog(parent, -1, _T("DupFinder"), wxDefaultPosition, wxDefaultSize, 
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) , ffi(_ffi)
{
	oldlogtarget = wxLog::GetActiveTarget();
	wxLog::SetActiveTarget(NULL);
}

DupFinderDlg3::~DupFinderDlg3() {
	wxLog::SetActiveTarget(oldlogtarget);

	int count = wResultList->GetItemCount();
	int i;

	for(i = 0; i < count; i++) {
		if(wResultList->GetItemData(i)) {
			delete (wxString *)wResultList->GetItemData(i);
		}
	}
}

BEGIN_EVENT_TABLE(DupFinderDlg3, wxDialog)
	EVT_CLOSE(			DupFinderDlg3::OnClose)
	EVT_SIZE(			DupFinderDlg3::OnSize)
	EVT_BUTTON(ID_STORE, 		DupFinderDlg3::OnStore)
	EVT_INIT_DIALOG(		DupFinderDlg3::OnInitDialog)
	EVT_LIST_ITEM_ACTIVATED(ID_RESULTLIST, DupFinderDlg3::OnListItemActivated)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_RESULTLIST, DupFinderDlg3::OnListItemRightClick)
	// Menu
	EVT_MENU(ID_OPENFILE, 		DupFinderDlg3::OnOpenFile)
	EVT_MENU(ID_OPENDIR, 		DupFinderDlg3::OnOpenDir)
	EVT_MENU(ID_COPYFILENAME, 	DupFinderDlg3::OnCopyFileName)
	EVT_MENU(ID_DELETE, 		DupFinderDlg3::OnDelete)
	/* not yet implemeted
	EVT_MENU(ID_HARDLINK, 		DupFinderDlg3::OnHardlink)
	EVT_MENU(ID_SOFTLINK, 		DupFinderDlg3::OnSoftLink)
	*/
END_EVENT_TABLE()

void DupFinderDlg3::OnInitDialog(wxInitDialogEvent  &event) 
{
	wxDialog::OnInitDialog(event);

	CreateControls();
	DisplayResults();

	CenterOnScreen();
}

void DupFinderDlg3::CreateControls() {
	
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *savesizer = new wxBoxSizer(wxHORIZONTAL);

	const int wxTOPLEFT = wxTOP | wxLEFT;
	const int wxTOPLEFTRIGHT = wxTOP | wxLEFT | wxRIGHT;

	topsizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Step 3: \nThe results")), 
		0, 
		wxTOPLEFT, 
		10);

	topsizer->Add(
		wResultList = new wxListView(this, ID_RESULTLIST, 
			wxDefaultPosition, wxSize(500, 300), 
			wxBORDER_SUNKEN | wxLC_REPORT), 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	savesizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Store results in file: ")), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	savesizer->Add(
		wReverse = new wxCheckBox(this, ID_REVERSE, _T("&Reverse order")),
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	savesizer->Add(
		new wxButton(this, ID_STORE, _T("&Store")), 
		0, 
		wxTOPLEFT | wxRIGHT, 
		10);

	topsizer->Add(
		savesizer, 
		0, 
		wxTOP | wxBOTTOM, 
		10);
	

	wResultList->InsertColumn(0, _T(""), wxLIST_FORMAT_LEFT, 500);

	SetSizer(topsizer);
	topsizer->SetSizeHints(this);
}

void DupFinderDlg3::OnClose(wxCloseEvent &WXUNUSED(event)) {

	int result;

	result = ::wxMessageBox(_T("Do you really want to close the results page? "), 
		_T("Confirm close"), wxYES_NO | wxICON_QUESTION, this);

	if(result == wxYES) {
		EndModal(0);
	}
}

void DupFinderDlg3::OnSize(wxSizeEvent &WXUNUSED(event)) {
	wxSizer *pSizer = GetSizer();
	if(pSizer) {
		pSizer->SetDimension(0, 0, 
			GetClientSize().GetWidth(), 
			GetClientSize().GetHeight());
	}
}

void DupFinderDlg3::DisplayResults() {
	multiset_fileinfosize_it it;
	list<fileinfoequal>::iterator it2;
	list<fileinfo>::iterator it3;
	wxFont font, boldfont;
	int item;

	for(it = ffi.pFilesBySize->begin(); it != ffi.pFilesBySize->end(); it++) {
		for(it2 = (*it)->equalfiles.begin(); it2 != (*it)->equalfiles.end(); it2++) {
			wxString tmp;
			tmp.Printf(_T("%u equal files of size %") wxLongLongFmtSpec _T("u"), 
				it2->files.size(), (*it)->size.GetValue());

			item = wResultList->InsertItem(wResultList->GetItemCount()+1, tmp);

			boldfont = font = wResultList->GetItemFont(item);
			boldfont.SetWeight(wxFONTWEIGHT_BOLD);
			boldfont.SetUnderlined(true);
			
			wResultList->SetItemFont(item, boldfont);

			wResultList->SetItemData(item, 0);

			for(it3 = it2->files.begin(); it3 != it2->files.end(); it3++) {
				item = wResultList->InsertItem(wResultList->GetItemCount()+1, it3->name);
				wxString *itemdata = new wxString(it3->name);
				wResultList->SetItemData(item, (long)itemdata);
			}
		}
	}

	// this causes problems with EndModal
	// i get a debug message that EndModal is 
	// called twice, but why?
	/*if(ffi.pFilesBySize->size() == 0) {
		wxMessageBox(_T("There are no double files! "), _T("DupFinder"), 
			 wxOK | wxICON_INFORMATION);

		EndModal(0);
	}*/
	if(ffi.pFilesBySize->size() == 0) {
		wResultList->InsertItem(1, _T("No duplicate files found. "));
	}

}

void DupFinderDlg3::OnStore(wxCommandEvent &WXUNUSED(event))
{
	bool bReverse = wReverse->GetValue();

	wxFileDialog * fdlg = new wxFileDialog(this, _T("Save as..."), _T(""), 
		_T("results.txt"), _T("Textfiles (*.txt)|*.txt|All files (*.*)|*.*"), 
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if(fdlg->ShowModal() == wxID_OK) {
		wxFile outfile;
		if(outfile.Create(fdlg->GetPath(), true)) {
			// use the old (primarily thought for console output) function
			PrintResults(*ffi.pFilesBySize, outfile, bReverse);
		}
	}

	delete fdlg;
}


void DupFinderDlg3::OnListItemActivated(wxListEvent &event) 
{
	
	if(event.GetData() != 0) {
		curtarget = event.GetIndex();
		OpenDir();
	}

}

void DupFinderDlg3::OpenDir() {
		wxFileName filename = *(wxString *)wResultList->GetItemData(curtarget);

		wxString path = filename.GetPath();

		::wxLaunchDefaultBrowser(path);
}

void DupFinderDlg3::OpenFile() {
		::wxLaunchDefaultBrowser(*(wxString *)wResultList->GetItemData(curtarget));
}

void DupFinderDlg3::OnListItemRightClick(wxListEvent &event)
{
	if(event.GetData() != 0) {
		curtarget = event.GetIndex();

		wxMenu * popupmenu = new wxMenu();
		
		popupmenu->Append(ID_OPENFILE, _T("&Open"));
		popupmenu->Append(ID_OPENDIR, _T("O&pen containing folder"));
		popupmenu->AppendSeparator();
		popupmenu->Append(ID_COPYFILENAME, _T("&Copy filename to clipboard"));
		popupmenu->AppendSeparator();
		popupmenu->Append(ID_DELETE, _T("&Delete"));
		/* not implemented yet
		popupmenu->Append(ID_HARDLINK, _T("Create &hardlink"));
		popupmenu->Append(ID_SOFTLINK, _T("Create &symbolic link));
		*/
		
		wResultList->PopupMenu(popupmenu);

		delete popupmenu; 
	}
}


void DupFinderDlg3::OnOpenFile(wxCommandEvent &WXUNUSED(event)) 
{
	OpenFile();
}

void DupFinderDlg3::OnOpenDir(wxCommandEvent &WXUNUSED(event))
{
	OpenDir();
}

void DupFinderDlg3::OnCopyFileName(wxCommandEvent &WXUNUSED(event))
{
	wxString filename = *(wxString *)wResultList->GetItemData(curtarget);

	wxTextDataObject * wxfile = new wxTextDataObject(filename);

	if(wxTheClipboard->Open()) {
	
		wxTheClipboard->SetData(wxfile);
		wxTheClipboard->Close();
	}
}

void DupFinderDlg3::OnDelete(wxCommandEvent &WXUNUSED(event)) 
{
	// delete file and delete item
	wxString filename = *(wxString *)wResultList->GetItemData(curtarget);

	wxString tmp;
	tmp.Printf(_T("Do you really want to delete \"%s?\" "), filename.c_str());

	int result = wxMessageBox(tmp, _T("Confirmation"), wxYES_NO);

	if(result == wxYES) {
		
		bool bResult = wxRemoveFile(filename);

		if(bResult) {
			wResultList->DeleteItem(curtarget);
		}
	}
}








