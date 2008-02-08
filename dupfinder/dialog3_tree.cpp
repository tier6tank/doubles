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
#include "os_cc_specific.h"

DupFinderDlg3::DupFinderDlg3(DupFinderDlg *_parent, findfileinfo &_ffi) 
	: wxDialog(NULL, -1, _T("DupFinder"), wxDefaultPosition, wxDefaultSize, 
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) , ffi(_ffi), parent(_parent), 
		RestrictToDir(_T("")), bRestrict(false)
{
}

DupFinderDlg3::~DupFinderDlg3() {
	ClearList();
}


enum {
	ID_RESULTLIST = 1, 
	ID_REVERSE, 
	ID_STORE, 
	ID_CONFDELETE, 
	ID_SHOWALL, 
	ID_APPLYDIR, 
	ID_DIRNAME, 
	ID_RESTRICTINFO, 
	ID_GETDIR, 

	// menu
	ID_OPENFILE, 
	ID_OPENDIR, 
	ID_COPYFILENAME, 
	ID_DELETE, 
	ID_HARDLINK, 
	ID_SOFTLINK, 
	ID_RESTTODIR
};

BEGIN_EVENT_TABLE(DupFinderDlg3, wxDialog)
	EVT_CLOSE(			DupFinderDlg3::OnClose)
	EVT_SIZE(			DupFinderDlg3::OnSize)
	EVT_BUTTON(ID_STORE, 		DupFinderDlg3::OnStore)
	EVT_INIT_DIALOG(		DupFinderDlg3::OnInitDialog)
	EVT_LIST_ITEM_ACTIVATED(ID_RESULTLIST, DupFinderDlg3::OnListItemActivated)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_RESULTLIST, DupFinderDlg3::OnListItemRightClick)
	EVT_LIST_KEY_DOWN(ID_RESULTLIST, DupFinderDlg3::OnListKeyDown)
	EVT_BUTTON(wxID_CANCEL, 	DupFinderDlg3::OnCancel)
	EVT_BUTTON(ID_APPLYDIR, 	DupFinderDlg3::OnApplyDir)
	EVT_BUTTON(ID_SHOWALL, 		DupFinderDlg3::OnShowAll)
	EVT_TEXT_ENTER(ID_DIRNAME, 	DupFinderDlg3::OnApplyDir)
	EVT_TEXT(ID_DIRNAME, 		DupFinderDlg3::OnDlgChange)
	EVT_BUTTON(ID_GETDIR, 		DupFinderDlg3::OnGetDir)
	// Menu
	EVT_MENU(ID_OPENFILE, 		DupFinderDlg3::OnOpenFile)
	EVT_MENU(ID_OPENDIR, 		DupFinderDlg3::OnOpenDir)
	EVT_MENU(ID_RESTTODIR, 		DupFinderDlg3::OnRestToDir)
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

	UpdateView();
}

void DupFinderDlg3::CreateControls() {
	
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *savesizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *dirsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *controlssizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer *resultssizer = new wxStaticBoxSizer(wxVERTICAL, this, _T("R&esults"));

	const int wxTOPLEFT = wxTOP | wxLEFT;
	const int wxTOPLEFTRIGHT = wxTOP | wxLEFT | wxRIGHT;

	topsizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Step 3: \nThe results")), 
		0, 
		wxTOPLEFT, 
		10);

	resultssizer->Add(
		wRestrictInfo = new wxStaticText(this, ID_RESTRICTINFO, _T("no restrictions"), 
			wxDefaultPosition, wxDefaultSize), 
		0, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	resultssizer->Add(
		wResultList = new wxListView(this, ID_RESULTLIST, 
			wxDefaultPosition, wxSize(500, 300), 
			wxBORDER_SUNKEN | wxLC_REPORT | wxLC_NO_HEADER), 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	savesizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Store the upper list to a file: ")), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	savesizer->Add(
		new wxButton(this, ID_STORE, _T("&Store")), 
		0, 
		wxTOPLEFT | wxRIGHT, 
		10);

	savesizer->AddStretchSpacer(1);

	savesizer->Add(
		wConfDelete = new wxCheckBox(this, ID_CONFDELETE, _T("&Confirm delete")), 
		0, 
		wxTOPLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirsizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Show only files and their \nduplicates in directory: ")), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirsizer->Add(
		wDirName = new wxTextCtrl(this, ID_DIRNAME, _T(""), 
			wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 
		1, 
		wxTOPLEFT, 
		10);

	dirsizer->Add(
		new wxButton(this, ID_GETDIR, _T("&...")), 
		0, 
		wxTOPLEFT, 
		10);

	dirsizer->Add(
		new wxButton(this, ID_APPLYDIR, _T("&Apply")), 
		0, 
		wxTOPLEFT, 
		10);

	dirsizer->Add(
		new wxButton(this, ID_SHOWALL, _T("S&how all")), 
		0, 
		wxTOPLEFT | wxRIGHT, 
		10);

	controlssizer->AddStretchSpacer(1);

	controlssizer->Add(
		new wxButton(this, wxID_CANCEL, _T("Cl&ose")), 
		0, 
		wxTOPLEFTRIGHT | wxBOTTOM | wxALIGN_RIGHT, 
		10);

	resultssizer->Add(
		dirsizer, 
		0, 
		wxEXPAND | wxBOTTOM, 
		10);

	resultssizer->Add(
		savesizer, 
		0, 
		wxBOTTOM | wxEXPAND, 
		10);

	topsizer->Add(
		resultssizer, 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	topsizer->Add(
		controlssizer, 
		0, 
		wxEXPAND  | wxALIGN_RIGHT, 
		10);

	topsizer->Hide(wRestrictInfo, true);

	wResultList->InsertColumn(0, _T(""), wxLIST_FORMAT_LEFT, 1000);

	wConfDelete->SetValue(true);

	SetSizer(topsizer);
	topsizer->SetSizeHints(this);

	wRestrictInfo->SetForegroundColour(wxColor(_T("#FF0000")));
}

void DupFinderDlg3::OnClose(wxCloseEvent &WXUNUSED(event)) {

	int result;

	result = ::wxMessageBox(_T("Do you really want to close the results page? "), 
		_T("Confirm close"), wxYES_NO | wxICON_QUESTION, this);

	if(result == wxYES) {
		ReturnToParent();
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
	multiset_fileinfosize::const_iterator it;
	list<fileinfoequal>::const_iterator it2;
	list<fileinfo>::const_iterator it3;
	wxFont font, boldfont;
	int item;

	if(ffi.pFilesBySize->size() == 0) {
		wxMessageBox(_T("There are no double files! "), _T("DupFinder"), 
			 wxOK | wxICON_INFORMATION, this);

		ReturnToParent();
		return;
	}

	ClearList();

	for(it = ffi.pFilesBySize->begin(); it != ffi.pFilesBySize->end(); it++) {
		for(it2 = it->equalfiles.begin(); it2 != it->equalfiles.end(); it2++) {
			bool bDisplay;

			if(bRestrict) {
				// test if it should be displayed
				bDisplay = false;


				for(it3 = it2->files.begin(); it3 != it2->files.end(); it3++) {
					wxFileName cur;	

					cur = it3->name;

					// a great speed gain by not using wxPATH_NORM_LONG
					// on windows, but short filesnames are not supported
					cur.Normalize (
						wxPATH_NORM_ALL & ~wxPATH_NORM_LONG);
					cur.Normalize(wxPATH_NORM_CASE);

					// is in in the directory?
					if(cur.GetPath() == RestrictToDir.GetPath()) {
						bDisplay = true;
						break;
					}
				}
			}
			else {
				bDisplay = true;
			}

			if(bDisplay) {

				wxString tmp;
				tmp.Printf(_T("%u equal files of size %") wxLongLongFmtSpec _T("u"), 
					it2->files.size(), it->size.GetValue());

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
	}

	// no items in list?
	if(wResultList->GetItemCount() == 0) {
		wResultList->InsertItem(0, _T("No items. "));
	}

}

void DupFinderDlg3::OnStore(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog * fdlg = new wxFileDialog(this, _T("Save as..."), _T(""), 
		_T("results.txt"), _T("Textfiles (*.txt)|*.txt|All files (*.*)|*.*"), 
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if(fdlg->ShowModal() == wxID_OK) {
		wxFile outfile;
		if(outfile.Create(fdlg->GetPath(), true)) {
			int count, i;
			count = wResultList->GetItemCount();

			for(i = 0; i < count; i++) {
				wxString *data = (wxString *)wResultList->GetItemData(i);
				wxString tmp;
				if(data) {
					tmp.Printf(_T("  \"%s\"\r\n"), wResultList->GetItemText(i).c_str());
				}
				else {
					// header item
					tmp.Printf(_T("- %s\r\n"), wResultList->GetItemText(i).c_str());
				}
				outfile.Write(tmp);
			}
			outfile.Close();
		}
	}

	delete fdlg;
}


void DupFinderDlg3::OnListItemActivated(wxListEvent &event) 
{
		
	if(event.GetData() != 0) {
		OpenDir(event.GetIndex());
	}

}

void DupFinderDlg3::OpenDir(long i) {
	wxString *data = (wxString *)wResultList->GetItemData(i);
	if(data) {
		wxFileName filename = *data;

		wxString path = wxString(_T("file:///")) + filename.GetPathWithSep();

		::wxLaunchDefaultBrowser(path);
	}
}

void DupFinderDlg3::OpenFile(long i) {
	wxString *data = (wxString *)wResultList->GetItemData(i);
	if(data) {
		::wxLaunchDefaultBrowser(*data);
	}
}

void DupFinderDlg3::OnListItemRightClick(wxListEvent &event)
{
	wxMenu * popupmenu = new wxMenu();

	if(event.GetData() != 0) {
		
		
		popupmenu->Append(ID_OPENFILE, _T("&Open"));
		popupmenu->Append(ID_OPENDIR, _T("O&pen containing folder"));
		popupmenu->AppendSeparator();
		popupmenu->Append(ID_RESTTODIR, _T("Sho&w only files in this folder"));
		popupmenu->AppendSeparator();
	}
	
	popupmenu->Append(ID_COPYFILENAME, _T("&Copy filename(s) to clipboard"));
	popupmenu->AppendSeparator();
	popupmenu->Append(ID_DELETE, _T("&Delete"));
	/* not implemented yet
	popupmenu->Append(ID_HARDLINK, _T("Create &hardlink"));
	popupmenu->Append(ID_SOFTLINK, _T("Create &symbolic link));
	*/
		
	wResultList->PopupMenu(popupmenu);

	delete popupmenu; 
}


void DupFinderDlg3::OnOpenFile(wxCommandEvent &WXUNUSED(event)) 
{
	int focus = wResultList->GetFocusedItem(); 
	if(focus != -1) {
		OpenFile(focus);
	}
}

void DupFinderDlg3::OnOpenDir(wxCommandEvent &WXUNUSED(event))
{
	int focus = wResultList->GetFocusedItem();
	if(focus != -1) {
		OpenDir(focus);
	}
}

void DupFinderDlg3::GetSelectedFilenameCount(int &count)
{
	int i;

	count = 0;
	for(i = wResultList->GetFirstSelected(); i != -1; i = wResultList->GetNextSelected(i)) {
		if(wResultList->GetItemData(i)) {
			count++;
		}
	}

}

int DupFinderDlg3::GetFirstSelectedFilename()
{
	int i;
	for(i = wResultList->GetFirstSelected(); i != -1; i = wResultList->GetNextSelected(i)) {
		if(wResultList->GetItemData(i)) {
			break;
		}
	}
	return i;
}

int DupFinderDlg3::GetNextSelectedFilename(int i)
{
	for(i = wResultList->GetNextSelected(i); i != -1; i = wResultList->GetNextSelected(i)) {
		if(wResultList->GetItemData(i)) {
			break;
		}
	}
	return i;
}

void DupFinderDlg3::OnCopyFileName(wxCommandEvent &WXUNUSED(event))
{
	wxString filename, tmp;
	int i, j, count;
	
	GetSelectedFilenameCount(count);
	
	if(count == 0) return;

	if(count > 1) {
		for(i = GetFirstSelectedFilename(), j = 0; i != -1; i = GetNextSelectedFilename(i), j++) {
			tmp.Printf(_T("\"%s\"%s"), 
				((wxString *)wResultList->GetItemData(i))->c_str(), 
				j == count-1 ? _T("") : _T(" ") );
			filename.Append(tmp);
		}
	}
	else {
		filename = *(wxString *)wResultList->GetItemData(GetFirstSelectedFilename());
	}

	wxTextDataObject * wxfile = new wxTextDataObject(filename);

	if(wxTheClipboard->Open()) {
	
		wxTheClipboard->SetData(wxfile);
		wxTheClipboard->Close();
	}
}

void DupFinderDlg3::OnDelete(wxCommandEvent &WXUNUSED(event)) 
{
	DeleteFiles();
}

void DupFinderDlg3::DeleteFiles()
{
	int i, count;
	wxString tmp;
	wxString filename;
	int result;
	list<int> delete_this;

	GetSelectedFilenameCount(count);

	if(count == 0) { return; }

	if(wConfDelete->GetValue()) {

		if(count == 1) {
			tmp.Printf(_T("Do you really want to delete \n\"%s?\" "), 
				( (wxString *)wResultList->GetItemData(GetFirstSelectedFilename()) )->c_str() );
		}
		else {
			tmp.Printf(_T("Do you really want to delete these %i files? "), count);
		}

		result = wxMessageBox(tmp, _T("Confirmation"), wxYES_NO);
	}
	else { 
		result = wxYES;
	}

	if(result == wxYES) {

		i = GetFirstSelectedFilename();
		while(i != -1) {

			filename = *(wxString *)wResultList->GetItemData(i);

			bool bResult = wxRemoveFile(filename);

			if(bResult) {
				delete_this.push_back(i);
			}
			else {
				tmp.Printf(_T("Error: cannot delete \"%s\"! "), filename.c_str());
				wxMessageBox(tmp, _T("Error"), wxICON_ERROR);
			}

			i = GetNextSelectedFilename(i);
		}
	}

	list<int>::reverse_iterator rit;

	// from the bottom to the top!
	for(rit = delete_this.rbegin(); rit != delete_this.rend(); rit++) {
		wResultList->DeleteItem(*rit);
	}

	DeleteOrphanedHeaders();
}

void DupFinderDlg3::ReturnToParent() {

	Hide();
	parent->ReturnToMe();
	Destroy();
}

void DupFinderDlg3::OnListKeyDown(wxListEvent &event)
{
	long focus = wResultList->GetFocusedItem();

	switch(event.GetKeyCode()) {
	case WXK_RETURN:
		OpenDir(focus);
		break;
	case WXK_DELETE:
		DeleteFiles();
		break;
	}

}


void DupFinderDlg3::DeleteOrphanedHeaders()
{
	int i, count;
	list<int> delete_this;

	count = wResultList->GetItemCount();
	
	for(i = 0; i < count; i++) {
		if(wResultList->GetItemData(i) == 0) {
			if((i < count-1 && wResultList->GetItemData(i+1) == 0) ||
				i == count-1) {
				delete_this.push_back(i);
			}
		}	
	}

	list<int>::reverse_iterator it;

	// delete from the end to the top, 
	// so that the item indexes remain valid!!!!
	for(it = delete_this.rbegin(); it != delete_this.rend(); it++) {
		wResultList->DeleteItem(*it);
	}
}

void DupFinderDlg3::OnCancel(wxCommandEvent &WXUNUSED(event))
{
	Close();
}



void DupFinderDlg3::OnApplyDir(wxCommandEvent &WXUNUSED(event))
{
	RestrictViewTo(wDirName->GetValue());
	DisplayResults();
}

void DupFinderDlg3::OnShowAll(wxCommandEvent &WXUNUSED(event))
{
	bRestrict = false;
	DisplayResults();
	UpdateView();
}

void DupFinderDlg3::RestrictViewTo(const wxString &dir) {
	RestrictToDir = wxFileName::DirName(dir);
	// get a little speed by not normalizing all
	RestrictToDir.Normalize(wxPATH_NORM_ALL & ~wxPATH_NORM_LONG);
	RestrictToDir.Normalize(wxPATH_NORM_CASE);
	bRestrict = true;
	UpdateView();
}

void DupFinderDlg3::ClearList() {
	int count = wResultList->GetItemCount();
	int i;

	for(i = 0; i < count; i++) {
		if(wResultList->GetItemData(i)) {
			delete (wxString *)wResultList->GetItemData(i);
		}
	}

	wResultList->DeleteAllItems();
}

void DupFinderDlg3::OnRestToDir(wxCommandEvent & WXUNUSED(event)) 
{
	int focus = wResultList->GetFocusedItem();
	if(focus != -1) {
		wxString *data = (wxString *)wResultList->GetItemData(focus);
		if(data) {
			wxFileName filename = *data;

			wxString path = filename.GetPath();

			RestrictViewTo(path);
			DisplayResults();
		}
	}
}

void DupFinderDlg3::OnDlgChange(wxCommandEvent &WXUNUSED(event))
{
	UpdateView();
}

void DupFinderDlg3::UpdateView() {
	wxString dirname = wDirName->GetValue();
	wxSize cs1, cs2;

	// correct dir must be entered
	FindWindow(ID_APPLYDIR)->Enable(
		wxFileName::DirExists(dirname));

	// show restriction info
	
	wxString tmp;
	tmp.Printf(_T("Showing only files in %s and their duplicates! "), 
		RestrictToDir.GetFullPath().c_str() );

	wRestrictInfo->SetLabel(tmp);

	GetSizer()->Show(wRestrictInfo, bRestrict, true);
	GetSizer()->Layout();
	cs1 = GetClientSize();
	GetSizer()->SetSizeHints(this);
	cs2 = GetClientSize();
	GetSizer()->SetDimension(0, 0, 
		max(cs1.GetWidth(), cs2.GetWidth()), 
		max(cs1.GetHeight(), cs2.GetHeight()));

}
		
void DupFinderDlg3::OnGetDir(wxCommandEvent &WXUNUSED(event)) {
	wxDirDialog dirch(this);

	dirch.SetWindowStyle(wxDD_DIR_MUST_EXIST);

	if(bRestrict) {
		dirch.SetPath(RestrictToDir.GetPath());
	} else {
		dirch.SetPath(wDirName->GetValue());
	}

	int ret = dirch.ShowModal();

	if(ret == wxID_OK) {
		wDirName->SetValue(dirch.GetPath());
	}
}













