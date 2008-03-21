/*
 *  Duplicate Files Finder - search for duplicate files
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
	: wxDialog(NULL, -1, _T("Duplicate Files Finder"), wxDefaultPosition, wxDefaultSize, 
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX) , 
		ffi(_ffi), parent(_parent), 
		RestrictToDir(_T("")), bRestrictToDir(false), 
		bRestrictToMask(false)
{
}

DupFinderDlg3::~DupFinderDlg3() {
}

bool DupFinderDlg3::bHardLinkWarning = true;

enum {
	TYPE_HEADER, 
	TYPE_ITEM, 
	TYPE_ROOT
};

class TreeItemData : public wxTreeItemData {
public:
	TreeItemData(int _type ) : type(_type) { data.mygroup = NULL; }
	virtual ~TreeItemData() {}

	virtual const wxTreeItemId &GetId() const { return id; }
	virtual void SetId(const wxTreeItemId &_id) { id = _id; }

	void SetGroup(fileinfoequal *_mygroup) {
		// assert(type == TYPE_HEADER);
		data.mygroup = _mygroup;
	}

	fileinfoequal *GetGroup() {
		// assert(type == TYPE_HEADER);
		return data.mygroup;
	}

	void SetIt(list<File>::iterator &it) {
		assert(type  == TYPE_ITEM);
		data.myself = it;
	}

	list<File>::iterator GetIt() {
		assert(type == TYPE_ITEM);
		return data.myself;
	}

	int GetType() const { return type; }


private:
	wxTreeItemId id;

	int type;
	
	struct _data {
	// union _data {
		fileinfoequal *mygroup;
		list<File>::iterator myself;
	} data;

};


enum {
	ID_RESULTLIST = 1, 
	ID_REVERSE, 
	ID_STORE, 
	ID_CONFDELETE, 
	ID_SHOWALL, 
	ID_APPLY, 
	ID_DIRNAME, 
	ID_GETDIR, 
	ID_SUBDIRS, 
	ID_RESTTODIR,
	ID_RESTTOMASK, 
	ID_MASK, 
	ID_EXPANDALL, 
	ID_COLLAPSEALL, 

	// menu

	// single menu 
	ID_MENU_OPENFILE, 
	ID_MENU_OPENDIR, 
	ID_MENU_COPYFILENAME, 
	ID_MENU_DELETE, 
	ID_MENU_HARDLINK, 
	ID_MENU_SYMLINK, 
	ID_MENU_RESTTODIR, 
	ID_MENU_RESTTOSDIR, 
	ID_MENU_DELETEBUTTHIS
};

BEGIN_EVENT_TABLE(DupFinderDlg3, wxDialog)
	EVT_CLOSE(			DupFinderDlg3::OnClose)
	EVT_SIZE(			DupFinderDlg3::OnSize)
	EVT_BUTTON(ID_STORE, 		DupFinderDlg3::OnStore)
	EVT_INIT_DIALOG(		DupFinderDlg3::OnInitDialog)
	EVT_TREE_ITEM_ACTIVATED(ID_RESULTLIST, DupFinderDlg3::OnTreeItemActivated)
	EVT_TREE_ITEM_RIGHT_CLICK(ID_RESULTLIST, DupFinderDlg3::OnTreeItemRightClick)
	EVT_TREE_KEY_DOWN(ID_RESULTLIST, DupFinderDlg3::OnTreeKeyDown)
	EVT_BUTTON(wxID_CANCEL, 	DupFinderDlg3::OnCancel)
	EVT_BUTTON(ID_APPLY, 		DupFinderDlg3::OnApply)
	EVT_BUTTON(ID_SHOWALL, 		DupFinderDlg3::OnShowAll) 
	EVT_TEXT_ENTER(ID_DIRNAME, 	DupFinderDlg3::OnApply)
	EVT_TEXT_ENTER(ID_MASK, 	DupFinderDlg3::OnApply) 
	EVT_TEXT(ID_DIRNAME, 		DupFinderDlg3::OnDirChange)
	EVT_BUTTON(ID_GETDIR, 		DupFinderDlg3::OnGetDir)
	EVT_TEXT(ID_MASK, 		DupFinderDlg3::OnMaskChange)
	EVT_BUTTON(ID_EXPANDALL, 	DupFinderDlg3::OnExpandAll)
	EVT_BUTTON(ID_COLLAPSEALL, 	DupFinderDlg3::OnCollapseAll)

	// Menu events
	EVT_MENU(ID_MENU_OPENFILE, 	DupFinderDlg3::OnOpenFile)
	EVT_MENU(ID_MENU_OPENDIR, 	DupFinderDlg3::OnOpenDir)
	EVT_MENU(ID_MENU_RESTTODIR,	DupFinderDlg3::OnRestToDir)
	EVT_MENU(ID_MENU_RESTTOSDIR, 	DupFinderDlg3::OnRestToSDir) 
	EVT_MENU(ID_MENU_COPYFILENAME, 	DupFinderDlg3::OnCopyFileName)
	EVT_MENU(ID_MENU_DELETE, 	DupFinderDlg3::OnDelete)
	EVT_MENU(ID_MENU_SYMLINK, 	DupFinderDlg3::OnSymLink)
	EVT_MENU(ID_MENU_HARDLINK, 	DupFinderDlg3::OnHardLink)
	EVT_MENU(ID_MENU_DELETEBUTTHIS, DupFinderDlg3::OnDeleteButThis)
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
	wxBoxSizer *restrictsizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer *restrictcontrolssizer = new wxStaticBoxSizer(wxVERTICAL, this,
		_T("Update list"));
	wxStaticBoxSizer *restrictdetailssizer = new wxStaticBoxSizer(wxVERTICAL, this, 
		_T("Show only files and their duplicates..."));
	wxFlexGridSizer *dirsizer = new wxFlexGridSizer(2, 2, 10, 10);
	wxBoxSizer *dirhorzsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *masksizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *controlssizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *expandsizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer *resultssizer = new wxStaticBoxSizer(wxVERTICAL, this, _T("R&esults"));
	
	const int wxTOPLEFT = wxTOP | wxLEFT;
	const int wxTOPLEFTRIGHT = wxTOP | wxLEFT | wxRIGHT;

	topsizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Step 3: \nThe results. Right click on items on the list ")
			_T("for a list of actions. ") ), 
		0, 
		wxTOPLEFTRIGHT, 
		10);

	resultssizer->Add(
		wResultList = new wxTreeCtrl(this, ID_RESULTLIST, 
			wxDefaultPosition, wxSize(wxDefaultSize.GetWidth(), 180), 
			wxBORDER_SUNKEN | wxTR_MULTIPLE /*| wxTR_HIDE_ROOT*/ | wxTR_HAS_BUTTONS), 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	savesizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Store the upper\nlist to a file: ")), 
		0, 
		wxALIGN_CENTER_VERTICAL, 
		10);

	savesizer->Add(
		new wxButton(this, ID_STORE, _T("&Store")), 
		0, 
		wxLEFT, 
		10);

	savesizer->AddStretchSpacer(1);

	savesizer->Add(
		wConfDelete = new wxCheckBox(this, ID_CONFDELETE, 
			_T("Show &confirmation message when deleting")), 
		0, 
		wxLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirsizer->Add(
		wRestrictToDir = new wxCheckBox(this, ID_RESTTODIR, _T("... in &directory: ")), 
		0, 
		wxALIGN_CENTER_VERTICAL, 
		10);

	dirhorzsizer->Add(
		wDirName = new wxTextCtrl(this, ID_DIRNAME, _T(""), 
			wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 
		1, 
		wxALIGN_CENTER_VERTICAL, 
		10);

	dirhorzsizer->Add(
		new wxButton(this, ID_GETDIR, _T("&...")), 
		0, 
		wxLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirsizer->Add(
		dirhorzsizer, 
		1, 
		wxEXPAND, 
		10);

	dirsizer->AddStretchSpacer();

	dirsizer->Add(
		wSubDirs = new wxCheckBox(this, ID_SUBDIRS, _T(" and &its subdir's")), 
		1, 
		wxALIGN_CENTER_VERTICAL, 
		10);

	dirsizer->AddGrowableCol(1);

	masksizer->Add(
		wRestrictToMask = new wxCheckBox(this, ID_RESTTOMASK, _T("... which &match that mask: ")), 
		0, 
		wxALIGN_CENTER_VERTICAL, 
		10);

	masksizer->Add(
		wMask = new wxTextCtrl(this, ID_MASK, _T("")), 
		1, 
		wxLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	restrictcontrolssizer->Add(
		new wxButton(this, ID_APPLY, _T("&Apply")), 
		0, 
		wxTOPLEFT, 
		10);

	restrictcontrolssizer->Add(
		new wxButton(this, ID_SHOWALL, _T("S&how all")), 
		0, 
		wxTOPLEFT | wxRIGHT, 
		10);

	restrictdetailssizer->Add(
		dirsizer, 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	restrictdetailssizer->Add(
		masksizer, 
		0, 
		wxTOPLEFTRIGHT | wxBOTTOM | wxEXPAND, 
		10);

	restrictsizer->Add(
		restrictdetailssizer, 
		1, 
		wxEXPAND);

	restrictsizer->Add(
		restrictcontrolssizer, 
		0, 
		wxEXPAND | wxLEFT, 
		10);

	controlssizer->AddStretchSpacer(1);

	controlssizer->Add(
		new wxButton(this, wxID_CANCEL, _T("Cl&ose")), 
		0, 
		wxALIGN_RIGHT);

	expandsizer->AddStretchSpacer(1);

	expandsizer->Add(
		new wxButton(this, ID_EXPANDALL, _T("Expand all")), 
		0, 
		wxALIGN_RIGHT);

	expandsizer->Add(
		new wxButton(this, ID_COLLAPSEALL, _T("Collapse all")), 
		0, 
		wxLEFT | wxALIGN_RIGHT,  
		10);

	resultssizer->Add(
		expandsizer, 
		0, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	resultssizer->Add(
		restrictsizer, 
		0, 
		wxTOPLEFTRIGHT| wxEXPAND, 
		10);

	resultssizer->Add(
		savesizer, 
		0, 
		wxTOPLEFTRIGHT | wxEXPAND | wxBOTTOM, 
		10);

	topsizer->Add(
		resultssizer, 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	topsizer->Add(
		controlssizer, 
		0, 
		wxTOPLEFTRIGHT | wxBOTTOM | wxEXPAND, 
		10);

	wConfDelete->SetValue(true);

	SetSizer(topsizer);
	topsizer->SetSizeHints(this);
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

struct less_fileiterator : public less<list<File>::iterator > {
	bool operator () (const list<File>::iterator &a, const list<File>::iterator &b) const {
		return a->GetName() < b->GetName();
	}
};

bool DupFinderDlg3::IsMatching(const wxString & string)
{
	bool bMatching = true;

	if(bRestrictToDir && bMatching) {
		wxFileName cur;	

		cur = string;

		// a great speed gain by not using wxPATH_NORM_LONG
		// on windows, but short filesnames are not supported
		cur.Normalize (
			wxPATH_NORM_ALL & ~wxPATH_NORM_LONG);
			cur.Normalize(wxPATH_NORM_CASE);

		bMatching = (wSubDirs->GetValue() ? 
			cur.GetPath().StartsWith(RestrictToDir.GetPath()) :
			cur.GetPath() == RestrictToDir.GetPath());
	}

	if(bRestrictToMask && bMatching) {
		wxFileName name = string;

		bMatching = name.GetFullName().Matches(wMask->GetValue());
	}

	return bMatching;
}

void DupFinderDlg3::DisplayResults() {
	multiset_fileinfosize::iterator it;
	list<fileinfoequal>::iterator it2;
	list<File>::iterator it3;
	// bool bHaveFont = false;
	wxFont font, boldfont;

	if(ffi.pFilesBySize->empty()) {
		wxMessageBox(_T("There are no double files! "), _T("Duplicate Files Finder"), 
			 wxOK | wxICON_INFORMATION, this);

		ReturnToParent();
		return;
	}

	// disable all repaint until the end of the function
	wResultList->Freeze();

	wResultList->DeleteAllItems();
	wxTreeItemId rootItem = wResultList->AddRoot(_T("Duplicate files: (statistics)"));
	wResultList->SetItemData(rootItem, new TreeItemData(TYPE_ROOT));

	for(it = ffi.pFilesBySize->begin(); it != ffi.pFilesBySize->end(); it++) {
		for(it2 = it->equalfiles.begin(); it2 != it->equalfiles.end(); it2++) {
			bool bDisplay;
			multiset<list<File>::iterator, less_fileiterator> matching;

			bool bRestrict = bRestrictToMask || bRestrictToDir;

			// don't include items which have only one element left ?
			if(it2->files.size() == 1 && false) {
				bDisplay = false; 
			} else {
				if(bRestrict) {
					// test if it should be displayed
					bDisplay = false;


					for(it3 = it2->files.begin(); it3 != it2->files.end(); it3++) {
					
						// is in in the directory?
						if(IsMatching(it3->GetName()) ) {
							bDisplay = true;
							matching.insert(it3);
							// break;
						}
					}
				}
				else {
					bDisplay = true;
				}
			}

			if(bDisplay) {
				TreeItemData *itemdata;

				wxString tmp;
				tmp.Printf(_T("%u equal files of size %") wxLongLongFmtSpec _T("u"), 
					it2->files.size(), it->size.GetValue());

				wxTreeItemId item, rootitem;

				item = wResultList->AppendItem(wResultList->GetRootItem(), tmp);

				/* if(!bHaveFont) {
					boldfont = font = wResultList->GetItemFont(item);
					boldfont.SetWeight(wxFONTWEIGHT_BOLD);
					boldfont.SetUnderlined(true);
					bHaveFont = true;
				}
			
				wResultList->SetItemFont(item, boldfont); */

				itemdata = new TreeItemData(TYPE_HEADER);
				itemdata->SetGroup(&*it2);

				wResultList->SetItemData(item, itemdata);

				rootitem = item;

				for(it3 = it2->files.begin(); it3 != it2->files.end(); it3++) {
					item = wResultList->AppendItem(rootitem, it3->GetName());
					itemdata = new TreeItemData(TYPE_ITEM);
					itemdata->SetGroup(&*it2);
					itemdata->SetIt(it3);
					
					// matching ?
					if(matching.find(it3) != matching.end()) {
						wResultList->SetItemBackgroundColour(item, wxColor(250, 120, 120));
					}
					wResultList->SetItemData(item, itemdata);
				}
			}
		}
	}

	// no items in list?
	if(wResultList->GetChildrenCount(wResultList->GetRootItem()) == 0) {
		wResultList->AppendItem(wResultList->GetRootItem(), _T("No items. "));
	}

	wResultList->ExpandAllChildren(wResultList->GetRootItem());

	DeleteOrphanedHeaders();

	// enable repaint
	wResultList->Thaw();

}

void DupFinderDlg3::OnStore(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog * fdlg = new wxFileDialog(this, _T("Save as..."), _T(""), 
		_T("results.txt"), _T("Textfiles (*.txt)|*.txt|All files (*.*)|*.*"), 
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if(fdlg->ShowModal() == wxID_OK) {
		wxFile outfile;
		if(outfile.Create(fdlg->GetPath(), true)) {
			wxTreeItemIdValue cookie, cookie2;
			wxTreeItemId i, j;

			for(i = wResultList->GetFirstChild(wResultList->GetRootItem(), cookie); 
				i.IsOk(); 
				i = wResultList->GetNextChild(wResultList->GetRootItem(), cookie)) {

				wxString tmp;
				tmp.Printf(_T("- %s\r\n"), wResultList->GetItemText(i).c_str());
				outfile.Write(tmp);

				for(j = wResultList->GetFirstChild(i, cookie2);
					j.IsOk();
					j = wResultList->GetNextChild(i, cookie2)) {
					tmp.Printf(_T("  \"%s\"\r\n"), wResultList->GetItemText(j).c_str());
					outfile.Write(tmp);
				}
			}
			outfile.Close();
		}
	}

	delete fdlg;
}



void DupFinderDlg3::OnTreeItemActivated(wxTreeEvent &event) 
{
	if(event.GetItem().IsOk()) {
		TreeItemData *data = (TreeItemData *)wResultList->GetItemData(event.GetItem());
		if(data->GetType() == TYPE_ITEM) {
			OpenDir(event.GetItem());
		} 
	}
}

void DupFinderDlg3::OpenDir(const wxTreeItemId &i) {
	if(i.IsOk()) {
		TreeItemData *data = (TreeItemData *)wResultList->GetItemData(i);
		if(data->GetType() == TYPE_ITEM) {
			wxFileName filename = data->GetIt()->GetName();

			wxString path = wxString(_T("file:///")) + filename.GetPathWithSep();

			::wxLaunchDefaultBrowser(path);
		}
	}
}

void DupFinderDlg3::OpenFile(const wxTreeItemId &i) {
	if(i.IsOk()) {
		TreeItemData *data = (TreeItemData *)wResultList->GetItemData(i);
		if(data->GetType() == TYPE_ITEM) {
			::wxLaunchDefaultBrowser(data->GetIt()->GetName());
		}
	}
}

void DupFinderDlg3::OnTreeItemRightClick(wxTreeEvent &event)
{
	wxMenu * popupmenu = new wxMenu();
	TreeItemData *data = (TreeItemData *)wResultList->GetItemData(event.GetItem());

	rightClickedItem = event.GetItem();
	
	if(data->GetType() == TYPE_ITEM) {
		bool bAddSep;
		
		popupmenu->Append(ID_MENU_OPENFILE, _T("&Open"));
		popupmenu->Append(ID_MENU_OPENDIR, _T("O&pen containing folder"));
		popupmenu->AppendSeparator();
		popupmenu->Append(ID_MENU_RESTTODIR, _T("Sho&w only files in this folder"));
		popupmenu->Append(ID_MENU_RESTTOSDIR, _T("Show onl&y files in this folder and subfolders"));
		popupmenu->AppendSeparator();
		bAddSep = false;
		if(IsSymLinkSupported()) {
			popupmenu->Append(ID_MENU_SYMLINK, _T("C&reate symbolic links to this file"));
			bAddSep = true;
		}
		if(IsHardLinkSupported()) {
			popupmenu->Append(ID_MENU_HARDLINK, _T("Create &hard links to this file"));
			bAddSep = true;
		}
		popupmenu->Append(ID_MENU_DELETEBUTTHIS, _T("Delete all duplicates to this file"));
		if(bAddSep) {
			popupmenu->AppendSeparator();
		}
	}
	
	popupmenu->Append(ID_MENU_COPYFILENAME, _T("&Copy filename(s) to clipboard"));
	popupmenu->AppendSeparator();
	popupmenu->Append(ID_MENU_DELETE, _T("&Delete"));
		
	wResultList->PopupMenu(popupmenu);

	delete popupmenu; 
}


void DupFinderDlg3::OnOpenFile(wxCommandEvent &WXUNUSED(event)) 
{
	OpenFile(rightClickedItem);
}

void DupFinderDlg3::OnOpenDir(wxCommandEvent &WXUNUSED(event))
{
	OpenDir(rightClickedItem);
}

int DupFinderDlg3::GetSelectedFilenameCount(const wxArrayTreeItemIds &selected)
{
	unsigned int i, count;

	count = 0;
	for(i = 0; i < selected.GetCount(); i++) {
		if(((TreeItemData *)wResultList->GetItemData(selected[i]))->GetType() == TYPE_ITEM) {
			count++;
		}
	}

	return count;
}

int DupFinderDlg3::GetFirstSelectedFilename(const wxArrayTreeItemIds &selected)
{
	unsigned int i;
	for(i = 0; i < selected.GetCount(); i++) {
		if(((TreeItemData *)wResultList->GetItemData(selected[i]))->GetType() == TYPE_ITEM) {
			break;
		}
	}
	return i == selected.GetCount() ? -1 : i;
}

int DupFinderDlg3::GetNextSelectedFilename(const wxArrayTreeItemIds &selected, unsigned int i)
{
	for(i++; i < selected.GetCount(); i++) {
		if(((TreeItemData *)wResultList->GetItemData(selected[i]))->GetType() == TYPE_ITEM) {
			break;
		}
	}
	return i != selected.GetCount() ? i : -1;
}

void DupFinderDlg3::OnCopyFileName(wxCommandEvent &WXUNUSED(event))
{
	wxString filename, tmp;
	int i, j, count;
	wxArrayTreeItemIds selected;

	wResultList->GetSelections(selected);
	
	count = GetSelectedFilenameCount(selected);
	
	if(count == 0) return;

	if(count > 1) {
		for(i = GetFirstSelectedFilename(selected), j = 0; 
			i != -1; 
			i = GetNextSelectedFilename(selected, i), j++) {
			tmp.Printf(_T("\"%s\"%s"), 
				((TreeItemData *)wResultList->GetItemData(selected[i]))
					->GetIt()->GetName().c_str(), 
				j == count-1 ? _T("") : _T(" ") );
			filename.Append(tmp);
		}
	}
	else {
		filename = ((TreeItemData *)wResultList->GetItemData(
			selected[GetFirstSelectedFilename(selected)]) )->GetIt()->GetName();
	}

	wxTextDataObject * wxfile = new wxTextDataObject(filename);

	if(wxTheClipboard->Open()) {
	
		wxTheClipboard->SetData(wxfile);
		wxTheClipboard->Close();
	}
}

void DupFinderDlg3::OnDelete(wxCommandEvent &WXUNUSED(event)) 
{
	wxArrayTreeItemIds selected;

	wResultList->GetSelections(selected);

	DeleteFiles(selected);
}

void DupFinderDlg3::DeleteFiles(const wxArrayTreeItemIds &selected)
{
	int i, count;
	wxString tmp;
	wxString filename;
	int result;
	list<wxTreeItemId> delete_this;

	count = GetSelectedFilenameCount(selected);

	if(count == 0) { return; }

	if(wConfDelete->GetValue()) {

		if(count == 1) {
			tmp.Printf(_T("Do you really want to delete \n\"%s?\" "), 
				((TreeItemData *)wResultList->GetItemData(
					selected[GetFirstSelectedFilename(selected)] ))
					->GetIt()->GetName().c_str() );
		}
		else {
			tmp.Printf(_T("Do you really want to delete these %i files? "), count);
		}

		result = wxMessageBox(tmp, _T("Confirmation"), wxYES_NO | wxICON_QUESTION);
	}
	else { 
		result = wxYES;
	}

	if(result == wxYES) {

		i = GetFirstSelectedFilename(selected);
		while(i != -1) {

			TreeItemData *data = (TreeItemData *)wResultList->GetItemData(selected[i]);\
			assert(data->GetType() == TYPE_ITEM);
			filename = data->GetIt()->GetName();

			bool bResult = wxRemoveFile(filename);

			if(bResult) {
				delete_this.push_back(selected[i]);
				data->GetGroup()->files.erase(data->GetIt());
			}
			else {
				tmp.Printf(_T("Error: cannot delete \"%s\"! "), filename.c_str());
				wxMessageBox(tmp, _T("Error"), wxICON_ERROR);
			}

			i = GetNextSelectedFilename(selected, i);
		}
	}

	list<wxTreeItemId>::reverse_iterator rit;

	// from the bottom to the top!
	// deleting only at the end of the function, 
	// because deleting while iterating over the 
	// elements would propably cause errors
	for(rit = delete_this.rbegin(); rit != delete_this.rend(); rit++) {
		wResultList->Delete(*rit);
	}
	

	DeleteOrphanedHeaders();
}


void DupFinderDlg3::ReturnToParent() {

	Hide();
	parent->ReturnToMe();
	Destroy();
}

void DupFinderDlg3::OnTreeKeyDown(wxTreeEvent &event)
{
	// wxTreeItemId focus = wResultList->GetFocusedItem();

	switch(event.GetKeyCode()) {
	/*case WXK_RETURN:
		OpenDir(focus);
		break; */
	case WXK_DELETE:
		{
		wxArrayTreeItemIds selected;
		
		wResultList->GetSelections(selected);

		DeleteFiles(selected);
		break;
		}
	}
}

void DupFinderDlg3::DeleteOrphanedHeaders()
{
	list<wxTreeItemId> delete_this;
	wxTreeItemId i;
	wxTreeItemIdValue cookie;
	
	for(i = wResultList->GetFirstChild(wResultList->GetRootItem(), cookie);
		i.IsOk();
		i = wResultList->GetNextChild(wResultList->GetRootItem(), cookie)) {

		if(wResultList->GetChildrenCount(i) == 0) {
			delete_this.push_back(i);
		}
	}

	list<wxTreeItemId>::reverse_iterator it;

	// delete from the end to the top, 
	// not immideately (see DeleteFiles)
	for(it = delete_this.rbegin(); it != delete_this.rend(); it++) {
		wResultList->Delete(*it);
	}
}


void DupFinderDlg3::OnCancel(wxCommandEvent &WXUNUSED(event))
{
	Close();
}

void DupFinderDlg3::OnApply(wxCommandEvent &WXUNUSED(event))
{
	bRestrictToDir = wRestrictToDir->GetValue();
	bRestrictToMask = wRestrictToMask->GetValue();

	if(bRestrictToDir) {
		wxString dirname = wDirName->GetValue();

		if(!wxFileName::DirExists(dirname)) {
			wxMessageBox(_T("Error: Directory does not exist. ")
			_T("\nPlease enter a valid directory. "), _T("Error"), 
			wxOK | wxICON_ERROR);
			return;
		}

		RestrictViewToDir(dirname);
	} 
	
	DisplayResults();
}

void DupFinderDlg3::RestrictViewToDir(const wxString &dirname)
{
	bRestrictToDir = true;

	RestrictToDir = wxFileName::DirName(dirname);
		
	RestrictToDir.Normalize(wxPATH_NORM_ALL & ~wxPATH_NORM_LONG);
	RestrictToDir.Normalize(wxPATH_NORM_CASE);
}

void DupFinderDlg3::OnShowAll(wxCommandEvent &WXUNUSED(event))
{
	bRestrictToDir = false;
	bRestrictToMask = false;
	DisplayResults();
}

void DupFinderDlg3::MenuRestToDir(bool bSubDirs)
{
	TreeItemData *data = (TreeItemData *)wResultList->GetItemData(rightClickedItem);
	if(data->GetType() == TYPE_ITEM) {
		wxFileName filename = data->GetIt()->GetName();

		wxString path = filename.GetPath();

		wSubDirs->SetValue(bSubDirs);
		wRestrictToDir->SetValue(true);
		wDirName->SetValue(path);

		RestrictViewToDir(path);

		DisplayResults();
	}
}

void DupFinderDlg3::OnRestToDir(wxCommandEvent & WXUNUSED(event)) 
{
	MenuRestToDir(false);
}

void DupFinderDlg3::OnRestToSDir(wxCommandEvent &WXUNUSED(event)) 
{
	MenuRestToDir(true);
}	

		
void DupFinderDlg3::OnGetDir(wxCommandEvent &WXUNUSED(event)) {
	wxDirDialog dirch(this);

	dirch.SetWindowStyle(wxDD_DIR_MUST_EXIST);

	if(bRestrictToDir) {
		dirch.SetPath(RestrictToDir.GetPath());
	} else {
		dirch.SetPath(wDirName->GetValue());
	}

	int ret = dirch.ShowModal();

	if(ret == wxID_OK) {
		wDirName->SetValue(dirch.GetPath());
	}
}


void DupFinderDlg3::OnSymLink(wxCommandEvent & WXUNUSED(event)) {
	CreateLink(CreateSymLink, _T("symbolic"));
}

void DupFinderDlg3::OnHardLink(wxCommandEvent & WXUNUSED(event)) {

	int result;

	if(bHardLinkWarning) {
		result = wxMessageBox(_T("Hardlinks should be used with caution. Also be aware that ")
			_T("hardlinked files will reappear in the next search as duplicates, as the ")
			_T("detection of hardlinked files currently is not supported. \n")
			_T("Do you want to see this warning again or cancel? "), 
			_T("Warning"), wxYES_NO | wxCANCEL | wxICON_WARNING, this);

		if(result == wxCANCEL) {
			return;
		}

		if(result == wxNO) {
			bHardLinkWarning = false;
		}
	}

	CreateLink(CreateHardLink, _T("hard"));
}

void DupFinderDlg3::CreateLink(bool (*link_func)(const wxString &, const wxString &), const wxString &type)
{
	bool bStickyError, bError, bFatalError = false;
	list <wxTreeItemId> remove_me;
	wxTreeItemId parent;
	wxTreeItemId i;
	wxTreeItemIdValue cookie;

	TreeItemData *target_data = (TreeItemData *)wResultList->GetItemData(rightClickedItem);
	if(target_data->GetType() != TYPE_ITEM) {
		return;
	}

	parent = wResultList->GetItemParent(rightClickedItem);
	assert(parent.IsOk());

	bStickyError = false;

	for(i = wResultList->GetFirstChild(parent, cookie);
		i.IsOk();
		i = wResultList->GetNextChild(parent, cookie)) {
		TreeItemData *data = (TreeItemData *)wResultList->GetItemData(i);
		assert(data->GetType() == TYPE_ITEM);

		bError = false;
		if(rightClickedItem != i) {
			wxFileName file = data->GetIt()->GetName();

			wxString tmpfile = wxFileName::CreateTempFileName(file.GetPathWithSep());

			if(tmpfile.Length() != 0) {
				bool bResult = wxRenameFile(file.GetFullPath(), tmpfile);
				
				if(!bResult) {
					bResult = wxRemoveFile(tmpfile);
					if(!bResult) {
						wxString tmp;
						tmp.Printf(_T("Cannot delete %s! "), tmpfile.c_str());
						wxMessageBox(tmp, _T("Error"), wxOK | wxICON_ERROR, this);
					}
					bError = true;
				} else {
					bResult = link_func(target_data->GetIt()->GetName(), file.GetFullPath());

					if(!bResult) {
						bError = true;

						// restore old state
						bFatalError = wxRenameFile(tmpfile, file.GetFullPath()) == false;
					}
					else {
						bool bResult = wxRemoveFile(tmpfile);
						if(!bResult) {
							wxString tmp;
							tmp.Printf(_T("Cannot delete %s! "), tmpfile.c_str());
							wxMessageBox(tmp, _T("Error"), wxOK | wxICON_ERROR, this);
						}
					}
				}
			}
			else {
				bError = true;
			}
			if(!bError) {
				data->GetGroup()->files.erase(data->GetIt());
				remove_me.push_back(i);
			}
			else {
				bStickyError = true;
			}
		}
	}

	// from the bottom to the top! 
	for(list<wxTreeItemId>::reverse_iterator rit = remove_me.rbegin(); 
		rit != remove_me.rend(); 
		rit++) {
		wResultList->Delete(*rit);
	}

	if(bStickyError) {
		wxString tmp;
		tmp.Printf(_T("Not all %s links could be created! "), type.c_str());
		wxMessageBox(tmp, _T("Error"), 
			wxOK | wxICON_ERROR, this);
	}
	if(bFatalError) {
		// this should never happen
		wxMessageBox(_T("Some actions could not be finished. Some files are in ")
			_T("undefined state. I recommend repeating the whole search. "), 
			_T("Sorry, an error which actually (almost) cannot happen"), wxOK | wxICON_ERROR, this);
	}
}

void DupFinderDlg3::OnMaskChange(wxCommandEvent &WXUNUSED(event)) 
{
	wRestrictToMask->SetValue(wMask->GetValue().Length() != 0);
}

void DupFinderDlg3::OnDirChange(wxCommandEvent &WXUNUSED(event))
{
	wRestrictToDir->SetValue(wDirName->GetValue().Length() != 0);
}

void DupFinderDlg3::OnExpandAll(wxCommandEvent &WXUNUSED(event)) 
{
	wxTreeItemId i;
	wxTreeItemIdValue cookie;
	for(i = wResultList->GetFirstChild(wResultList->GetRootItem(), cookie);
		i.IsOk();
		i = wResultList->GetNextChild(wResultList->GetRootItem(), cookie)) {
		wResultList->Expand(i);
	}
}

void DupFinderDlg3::OnCollapseAll(wxCommandEvent &WXUNUSED(event))
{
	wxTreeItemId i;
	wxTreeItemIdValue cookie;
	for(i = wResultList->GetFirstChild(wResultList->GetRootItem(), cookie);
		i.IsOk();
		i = wResultList->GetNextChild(wResultList->GetRootItem(), cookie)) {
		wResultList->Collapse(i);
	}
}


void DupFinderDlg3::OnDeleteButThis(wxCommandEvent &WXUNUSED(event))
{
	wxTreeItemId group;
	TreeItemData *targetdata;
	list<wxTreeItemId> delete_these;

	targetdata = (TreeItemData *)wResultList->GetItemData(rightClickedItem);
	
	if(targetdata->GetType() == TYPE_ITEM) {

		// do not accidentaly erase the other files if the only
		// file do be kept does not exist any more
		if(!wxDir::Exists(targetdata->GetIt()->GetName())) {
			wxString tmp;
			tmp.Printf(_T("The file %s does not exist! I won't delete the files. "), 
				targetdata->GetIt()->GetName().c_str());
			wxMessageBox(tmp, _T("Error"), wxICON_ERROR | wxOK, this);
			return;
		}

		wxTreeItemId i;
		wxTreeItemIdValue cookie;
		wxArrayTreeItemIds todelete;

		group = wResultList->GetItemParent(rightClickedItem);

		for(i = wResultList->GetFirstChild(group, cookie); 
			i.IsOk(); 
			i = wResultList->GetNextChild(group, cookie)) {

			if(i != rightClickedItem) {
				todelete.Add(i);
			}
		}

		DeleteFiles(todelete);
	}
}



