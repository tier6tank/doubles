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
#include "dialog1.h"
#include "dialog2.h"
#include "os_cc_specific.h"
#include "os_cc_specific_gui.h"

// abbreviation for ... = dynamic_cast<ClassName *> ((ClassName *)object);
// question is if it makes any sense at all...
#define safe_cast(a, b) dynamic_cast<b>((b)(a))

enum {
	ID_DIRLIST = 1, 
	ID_GETDIR, 
	ID_ADDDIR, 
	ID_CHANGEDIR, 
	ID_RMDIR, 
	ID_DIRNAME, 
	// options
	ID_RECURSIVE, 
	ID_HIDDEN, 
	ID_INCLUDE, 
	ID_EXCLUDE, 
	ID_MINSIZE, 
	ID_RMALL, 
	ID_MAXSIZE, 
	ID_DEFAULTS, 
	ID_EMPTYFILES
};

BEGIN_EVENT_TABLE(DupFinderDlg, wxDialog)
	EVT_CLOSE(			DupFinderDlg::OnClose)
	EVT_SIZE(			DupFinderDlg::OnSize)
	EVT_BUTTON(wxID_CANCEL,		DupFinderDlg::OnEscape)
	EVT_BUTTON(wxID_OK, 		DupFinderDlg::OnOk)
	// the following is for UpdateView()
	EVT_TEXT(ID_DIRNAME, 		DupFinderDlg::OnDlgChange)
	EVT_LIST_ITEM_SELECTED(ID_DIRLIST, DupFinderDlg::OnListSelChange)
	EVT_LIST_ITEM_DESELECTED(ID_DIRLIST, DupFinderDlg::OnListSelChange)
	EVT_LIST_ITEM_ACTIVATED(ID_DIRLIST, DupFinderDlg::OnListSelChange)
	EVT_TEXT(ID_MINSIZE, 		DupFinderDlg::OnDlgChange)

	EVT_BUTTON(ID_ADDDIR, 		DupFinderDlg::OnDirAdd)
	EVT_BUTTON(ID_RMDIR, 		DupFinderDlg::OnDirRemove)
	EVT_BUTTON(ID_GETDIR, 		DupFinderDlg::OnGetDir)
	EVT_BUTTON(ID_CHANGEDIR, 	DupFinderDlg::OnChangeDir)
	EVT_TEXT_ENTER(ID_DIRNAME, 	DupFinderDlg::OnDirNameEnter)
	EVT_BUTTON(ID_RMALL, 		DupFinderDlg::OnRemoveAll)
	EVT_INIT_DIALOG(		DupFinderDlg::OnInitDialog)
	EVT_BUTTON(wxID_ABOUT, 		DupFinderDlg::OnAbout)
	EVT_BUTTON(ID_DEFAULTS, 	DupFinderDlg::OnDefaults)
END_EVENT_TABLE()



DupFinderDlg::DupFinderDlg(wxWindow * parent) 
	: wxDialog(parent, -1, _T("Duplicate Files Finder - Start"), 
		wxDefaultPosition, wxDefaultSize, 
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX), 
	  m_dupfinder(NULL, false) {

	CreateControls();

	InitControls();

	CenterOnScreen();

	SetDefaults();
}

DupFinderDlg::~DupFinderDlg() {
	long i;
	long count = m_wDirList->GetItemCount();

	// delete item related memory
	for(i = 0; i < count; i++) {
		// delete item <item>
		SearchPathInfo *pspi = 
			safe_cast(m_wDirList->GetItemData(i), SearchPathInfo *);
		
		delete pspi;
	}
}

void DupFinderDlg::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *dirsizer = new wxStaticBoxSizer(wxVERTICAL, this, 
		_T("Direc&tories"));

	wxStaticBoxSizer *dirinfosizer = new wxStaticBoxSizer(wxVERTICAL, this, 
		_T("Directory name and search parameters"));

	wxStaticBoxSizer *actionssizer = new wxStaticBoxSizer(wxVERTICAL, this, 
		_T("Edit list"));

	wxBoxSizer *infosizer = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer *dirrow1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *dirrow2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *dirrow3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *dirrow4 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer *controlssizer = new wxBoxSizer(wxHORIZONTAL);

	const int wxTOPLEFT = wxLEFT | wxTOP;
	const int wxTOPLEFTRIGHT = wxLEFT | wxTOP | wxRIGHT;
	// const int wxTOPRIGHT = wxRIGHT | wxTOP;

	// &_ used
	// tdenixshyfarogcbl
	// abcdefghilnorstxy

	/***************************** Dialog creation **********************************/

	topsizer->Add(
		new wxStaticText(this, wxID_STATIC, 
			wxString(_T("Step 1: \nFor each directory to search in fill out the fields\n")
				_T("below (leave fields you do not need blank), then click on \"Add\". \n")
				_T("You can concatenate include and exclude masks (e.g. \"*.txt") ) + 
				wxString(GetPathSepChar()) + wxString(_T("*.jpg\"). ") ) ), 
			0, 
			wxTOPLEFTRIGHT | wxEXPAND, 
			10 );

	dirsizer->Add(
		m_wDirList = new wxListView(this, ID_DIRLIST, wxDefaultPosition, 
			wxSize(wxDefaultSize.GetWidth(), 100),
			wxBORDER_SUNKEN | wxLC_REPORT | wxLC_SINGLE_SEL), 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	dirrow1->Add(
		new wxStaticText(this, wxID_STATIC, _T("&Directory: ")),
		0,
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow1->Add(
		m_wDirName = new wxTextCtrl(this, ID_DIRNAME, _T(""), wxDefaultPosition, 
			wxDefaultSize, wxTE_PROCESS_ENTER), 
		1, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow1->Add(
		new wxButton(this, ID_GETDIR, _T("&...")), 
		0, 
		wxTOPLEFTRIGHT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow3->Add(
		new wxStaticText(this, wxID_STATIC, _T("M&inimal\nfile size: ") ), 
		0,
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow3->Add(
		m_wMinSize = new wxTextCtrl(this, ID_MINSIZE, _T(""), wxDefaultPosition, 
			wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC) ), 
		1, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow3->Add(
		new wxStaticText(this, wxID_STATIC, _T("Ma&ximal\nfile size: ")), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow3->Add(
		m_wMaxSize = new wxTextCtrl(this, ID_MAXSIZE, _T(""), wxDefaultPosition, 
			wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC) ), 
		1, 
		wxTOPLEFTRIGHT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow2->Add(
		new wxStaticText(this, wxID_STATIC, _T("I&nclude: ") ), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow2->Add(
		m_wInclude = new wxTextCtrl(this, ID_INCLUDE, _T("*")), 
		1, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow2->Add(
		new wxStaticText(this, wxID_STATIC, _T("&Exclude: ") ), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);
	
	dirrow2->Add(
		m_wExclude = new wxTextCtrl(this, ID_EXCLUDE, _T("")), 
		1, 
		wxTOPLEFTRIGHT | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow4->Add(
		new wxStaticText(this, wxID_STATIC, _T("Include ...") ), 
		0, 
		wxTOPLEFT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow4->Add(
		m_wRecursive = new wxCheckBox(this, ID_RECURSIVE, _T("&subdir's")), 
		0, 
		wxTOPLEFT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow4->Add( 
		m_wHidden = new wxCheckBox(this, ID_HIDDEN, _T("&hidden files") ), 
		0, 
		wxTOPLEFT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow4->Add(
		m_wEmptyFiles = new wxCheckBox(this, ID_EMPTYFILES, _T("&empty files") ),
		0, 
		wxTOPLEFT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 
		10);

	dirrow4->AddStretchSpacer(1);

	dirrow4->Add(
		new wxButton(this, ID_DEFAULTS, _T("De&faults")), 
		0, 
		wxTOPLEFTRIGHT | wxBOTTOM | wxRIGHT | wxALIGN_CENTER_VERTICAL, 
		10);

	actionssizer->Add(
		new wxButton(this, ID_ADDDIR, _T("&Add")), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	actionssizer->Add(
		new wxButton(this, ID_CHANGEDIR, _T("&Change")), 
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	actionssizer->Add(
		new wxButton(this, ID_RMDIR, _T("&Remove")), 	
		0, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10);

	actionssizer->AddStretchSpacer();

	actionssizer->Add(
		new wxButton(this, ID_RMALL, _T("Rem&ove all")), 
		0, 
		wxTOPLEFTRIGHT | wxBOTTOM | wxALIGN_BOTTOM, 
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
		new wxButton(this, wxID_CANCEL, _T("C&lose")), 
		0, 
		wxALIGN_RIGHT | wxLEFT, 
		10);

	dirinfosizer->Add(dirrow1, 
		0, 
		wxTOPLEFT | wxEXPAND, 
		0);

	dirinfosizer->Add(dirrow2, 
		0, 
		wxTOPLEFT | wxEXPAND, 
		0);

	dirinfosizer->Add(dirrow3, 
		0, 
		wxTOPLEFT | wxEXPAND, 
		0);

	dirinfosizer->Add(dirrow4, 
		0, 
		wxTOPLEFT | wxEXPAND, 
		0);

	infosizer->Add(
		dirinfosizer, 
		1, 
		wxTOP | wxEXPAND, 
		10);

	infosizer->Add(
		actionssizer, 
		0, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	dirsizer->Add(
		infosizer, 
		0, 
		wxTOPLEFT | wxBOTTOM | wxEXPAND, 
		10);

	topsizer->Add(dirsizer,
		1, 
		wxTOPLEFTRIGHT | wxBOTTOM | wxEXPAND, 
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

	// add coloumns to dir list control
	// which order?...
	m_wDirList->InsertColumn(1, _T("Subdirs"), wxLIST_FORMAT_LEFT, 30);
	m_wDirList->InsertColumn(2, _T("Hidden"), wxLIST_FORMAT_LEFT, 30);
	m_wDirList->InsertColumn(3, _T("Path"), wxLIST_FORMAT_LEFT, 200);
	m_wDirList->InsertColumn(4, _T("Include"), wxLIST_FORMAT_LEFT, 60);
	m_wDirList->InsertColumn(5, _T("Exclude"), wxLIST_FORMAT_LEFT, 60);
	m_wDirList->InsertColumn(6, _T("Min size"), wxLIST_FORMAT_LEFT, 30);
	m_wDirList->InsertColumn(7, _T("Max size"), wxLIST_FORMAT_LEFT, 30);
	m_wDirList->InsertColumn(8, _T("Empty files"), wxLIST_FORMAT_LEFT, 30);

	
	// set focus
	m_wDirName->SetFocus();
}

void DupFinderDlg::UpdateView() {

	// disable/enable controls

	// add button only enabled if there's a (valid) dir in 
	// dir text control and there is a valid number/no number
	// in minsize field
	// this functionality is disabled (it's not that nice)

	/* wxString dir = m_wDirName->GetValue();

	FindWindow(ID_ADDDIR)->Enable(
		wxFileName::DirExists(dir)  && wMinSize->GetValue().IsNumber()
	);
	*/

	// add button disabled when dir field empty

	FindWindow(ID_ADDDIR)->Enable(
		m_wDirName->GetValue().Length() != 0
	);

	// remove button only if there is a selection 
	// in the dir list

	FindWindow(ID_RMDIR)->Enable(
		m_wDirList->GetFirstSelected() != -1
	);

	// Go button only activated if there is at least
	// one path in the list

	FindWindow(wxID_OK)->Enable(
		m_wDirList->GetItemCount() != 0
	);

	// RemoveAll enabled if there's something in the list
	FindWindow(ID_RMALL)->Enable(
		m_wDirList->GetItemCount() != 0
	);

}

void DupFinderDlg::OnClose(wxCloseEvent &WXUNUSED(event) ) {
	Destroy();
}

void DupFinderDlg::OnEscape(wxCommandEvent &WXUNUSED(event)) {
	Close();
}

void DupFinderDlg::OnDlgChange(wxCommandEvent &WXUNUSED(event))
{
	UpdateView();
}

void DupFinderDlg::OnListSelChange(wxListEvent &WXUNUSED(event))
{
	UpdateView();
	UpdateControls();
}

void DupFinderDlg::OnSize(wxSizeEvent &WXUNUSED(event)) {
	wxSizer *pSizer = GetSizer();
	if(pSizer) {
		pSizer->SetDimension(0, 0, 
			GetClientSize().GetWidth(), GetClientSize().GetHeight());
	}
}

void DupFinderDlg::OnOk(wxCommandEvent &WXUNUSED(event)) {
	int i, count;

	count = m_wDirList->GetItemCount();

	for(i = 0; i < count; i++) {
		// wxMessageBox(it->path);
		m_dupfinder.AddPath(*safe_cast(m_wDirList->GetItemData(i), SearchPathInfo *));
	}

	this->Hide();

	DupFinderDlg2 *progress = new DupFinderDlg2(m_dupfinder, this);	

	progress->Show();
	wxTheApp->SetTopWindow(progress);

}

bool DupFinderDlg::GetInformation(SearchPathInfo &pi)
{
	wxULongLong minsize;
	wxULongLong maxsize;
	bool bResult;
	
	pi.path = m_wDirName->GetValue();

	if(!wxFileName::DirExists(pi.path)) {
		wxMessageBox(_T("Please enter a valid path! "), _T("Error"), 
			wxOK | wxICON_ERROR, this);
		return false;
	}

	bResult = StringToULongLong(m_wMinSize->GetValue(), minsize);

	if(!bResult) {
		wxMessageBox(_T("Invalid minimal size! "), 
			_T("Error"), 
			wxOK | wxICON_ERROR, 
			this);
		return false;
	}

	bResult = StringToULongLong(m_wMaxSize->GetValue(), maxsize);

	if(!bResult) {
		wxMessageBox(_T("Invalid maximal size! "), 
			_T("Error"), 
			wxOK | wxICON_ERROR, 
			this);
		return false;
	}

	if(minsize > maxsize && maxsize != 0) {
		wxMessageBox(_T("The maximal size must not be greater than the minimal size! "), 
			_T("Error"), 
			wxOK | wxICON_ERROR, this);
		return false;
	}
	
	pi.nMinSize = minsize;
	pi.nMaxSize = maxsize; 

	pi.bRecursive = m_wRecursive->GetValue();
	pi.bHidden = m_wHidden->GetValue();
	pi.Include = !m_wInclude->GetValue().IsEmpty() ? m_wInclude->GetValue() : wxString(_T("*"));
	pi.Exclude = m_wExclude->GetValue();
	pi.bEmptyFiles = m_wEmptyFiles->GetValue();
	return true;
}

void DupFinderDlg::OnDirAdd(wxCommandEvent &WXUNUSED(event)) {
	wxListItem c1, c2, c3, c4, c5, c6, c7, c8;
	SearchPathInfo spi;

	if(!GetInformation(spi)) {
		return;
	}
	
	PrepareListItem(spi, c1, c2, c3, c4, c5, c6, c7, c8);

	m_wDirList->InsertItem(c1);
	m_wDirList->SetItem(c2);
	m_wDirList->SetItem(c3);
	m_wDirList->SetItem(c4);
	m_wDirList->SetItem(c5);
	m_wDirList->SetItem(c6);
	m_wDirList->SetItem(c7);
	m_wDirList->SetItem(c8);

	UpdateView();
}

void DupFinderDlg::OnDirRemove(wxCommandEvent &WXUNUSED(event)) {
	long item;

	item = m_wDirList->GetFirstSelected();

	if(item == -1) {
		return;
	}

	// delete item <item>
	SearchPathInfo *pspi = 
			safe_cast(m_wDirList->GetItemData(item), SearchPathInfo *);
		
	delete pspi;

	// ? delete all items ? 
	m_wDirList->DeleteItem(item);

	UpdateView();
}

void DupFinderDlg::OnGetDir(wxCommandEvent &WXUNUSED(event)) {
	wxDirDialog dirch(this);

	dirch.SetWindowStyle(wxDD_DIR_MUST_EXIST);

	dirch.SetPath(m_wDirName->GetValue());
	// dirch.SetMessage(
	int ret = dirch.ShowModal();

	if(ret == wxID_OK) {
		m_wDirName->SetValue(dirch.GetPath());
	}
}

void DupFinderDlg::OnRemoveAll(wxCommandEvent &WXUNUSED(event)) {
	int i;
	int count = m_wDirList->GetItemCount();

	for(i = 0; i < count; i++) {
		delete (safe_cast(m_wDirList->GetItemData(i), SearchPathInfo *));
	}

	m_wDirList->DeleteAllItems();

	UpdateView();

}

void DupFinderDlg::OnInitDialog(wxInitDialogEvent &event) {
	wxDialog::OnInitDialog(event);

	UpdateView();
}

void DupFinderDlg::OnAbout(wxCommandEvent &WXUNUSED(event)) {
	wxAboutDialogInfo info;

	// change the version with every release!

	AboutBox(
		_T("Matthias Boehm"), 
		_T("(c) Matthias Boehm 2008"), 
		_T("Find and delete duplicate files"), 
		_T("Duplicate Files Finder"), 
		_T("0.46")
		);
}

void DupFinderDlg::ReturnToMe()
{
	CleanUp();
	Show();
}


void DupFinderDlg::CleanUp()
{
	m_dupfinder.Reset();
}

void DupFinderDlg::SetDefaults()
{
	wxPlatformInfo pi;

	m_wInclude->SetValue(_T("*"));
	
	if(pi.GetOperatingSystemId() & wxOS_WINDOWS) {
		// ignore links on Windows
		m_wExclude->SetValue(_T("*.lnk"));
	}
	else {
		m_wExclude->SetValue(_T(""));
	}
	// m_wDirName->SetValue(_T("");
	m_wRecursive->SetValue(true);
	m_wHidden->SetValue(false);
	m_wMinSize->SetValue(_T(""));
	m_wMaxSize->SetValue(_T(""));
	m_wEmptyFiles->SetValue(false);
}

void DupFinderDlg::OnDefaults(wxCommandEvent & WXUNUSED(evt)) {
	SetDefaults();
}

void DupFinderDlg::UpdateControls()
{
	wxString tmp;
	// fill controls with list contents

	long selectedItem = m_wDirList->GetFirstSelected();

	if(selectedItem == -1) {
		return;
	}

	SearchPathInfo *pspi = safe_cast(m_wDirList->GetItemData(selectedItem), SearchPathInfo *);

	assert(pspi);
	if(!pspi) {
		return;
	}

	m_wDirName->SetValue(pspi->path);
	m_wInclude->SetValue(pspi->Include);
	m_wExclude->SetValue(pspi->Exclude);
	m_wRecursive->SetValue(pspi->bRecursive);
	m_wHidden->SetValue(pspi->bHidden);
	m_wMinSize->SetValue(pspi->nMinSize == 0 ? 
		_T("") : 
		wxString::Format(_T("%") wxLongLongFmtSpec _T("u"), pspi->nMinSize) );
	m_wMaxSize->SetValue(pspi->nMaxSize == 0 ?
		 _T("") : 
		wxString::Format(_T("%") wxLongLongFmtSpec _T("u"), pspi->nMaxSize) );
	m_wEmptyFiles->SetValue(pspi->bEmptyFiles); 

}

void DupFinderDlg::OnChangeDir(wxCommandEvent &WXUNUSED(event))
{
	wxListItem c1, c2, c3, c4, c5, c6, c7, c8;
	SearchPathInfo spi;

	if(!GetInformation(spi)) {
		return;
	}

	long curItem = m_wDirList->GetFirstSelected();

	if(curItem == -1) {
		return;
	}

	SearchPathInfo *pspi = safe_cast(m_wDirList->GetItemData(curItem), SearchPathInfo *);
	assert(pspi);
	if(!pspi) {
		return;
	}

	delete pspi;

	PrepareListItem(spi, c1, c2, c3, c4, c5, c6, c7, c8);

	c1.SetId(curItem);
	c2.SetId(curItem);
	c3.SetId(curItem);
	c4.SetId(curItem);
	c5.SetId(curItem);
	c6.SetId(curItem);
	c7.SetId(curItem);
	c8.SetId(curItem);


	m_wDirList->SetItem(c1);
	m_wDirList->SetItem(c2);
	m_wDirList->SetItem(c3);
	m_wDirList->SetItem(c4);
	m_wDirList->SetItem(c5);
	m_wDirList->SetItem(c6);
	m_wDirList->SetItem(c7);
	m_wDirList->SetItem(c8);

	UpdateView();
}

void DupFinderDlg::PrepareListItem(const SearchPathInfo &pi, wxListItem &c1, wxListItem &c2, 
	wxListItem &c3, wxListItem &c4, wxListItem &c5, wxListItem &c6, 
	wxListItem &c7, wxListItem &c8)
{
	wxString tmp1, tmp2;

	// c1.SetColumn(0); // don't set column, else you will get strange assert messages
	c2.SetColumn(1);
	c3.SetColumn(2);
	c4.SetColumn(3);
	c5.SetColumn(4);
	c6.SetColumn(5);
	c7.SetColumn(6);
	c8.SetColumn(7);
	c1.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
	c2.SetMask(wxLIST_MASK_TEXT);
	c3.SetMask(wxLIST_MASK_TEXT);
	c4.SetMask(wxLIST_MASK_TEXT);
	c5.SetMask(wxLIST_MASK_TEXT);
	c6.SetMask(wxLIST_MASK_TEXT);
	c7.SetMask(wxLIST_MASK_TEXT);
	c8.SetMask(wxLIST_MASK_TEXT);
	
	c1.SetData(new SearchPathInfo(pi) );

	c1.SetText(pi.bRecursive ? _T("x") : _T(""));
	c2.SetText(pi.bHidden ? _T("x") : _T(""));
	c3.SetText(pi.path);
	c4.SetText(pi.Include);
	c5.SetText(pi.Exclude);
	tmp1 = wxEmptyString; // borland ...
	tmp2 = pi.nMinSize.ToString();
	c6.SetText(m_wMinSize->GetValue().IsEmpty() ? tmp1 : tmp2);
	tmp1 = wxEmptyString;
	tmp2 = pi.nMaxSize.ToString();
	c7.SetText(m_wMaxSize->GetValue().IsEmpty() ? tmp1 : tmp2);
	c8.SetText(pi.bEmptyFiles ? _T("x") : _T(""));
}





