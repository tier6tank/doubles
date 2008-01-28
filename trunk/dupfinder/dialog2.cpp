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
using namespace std;

#include "dialog2.h"
#include "dialog3.h"
#include "dbl.h"

BEGIN_EVENT_TABLE(DupFinderDlg2, wxDialog)
	EVT_CLOSE(			DupFinderDlg2::OnClose)
	EVT_SIZE( 			DupFinderDlg2::OnSize)
	EVT_INIT_DIALOG(		DupFinderDlg2::OnInitDialog)
	EVT_IDLE(			DupFinderDlg2::OnIdle)
	EVT_BUTTON(wxID_CANCEL, 	DupFinderDlg2::OnCancel)
	EVT_CHECKBOX(ID_SHOWMESSAGES, 	DupFinderDlg2::OnShowMessages)
END_EVENT_TABLE()

DupFinderDlg2::DupFinderDlg2(findfileinfo _ffi, wxWindow *parent) : 
	wxDialog(parent, -1, _T("DupFinder"), wxDefaultPosition, wxDefaultSize, 
	(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) & ~wxCLOSE_BOX), ffi(_ffi), 
	bStarted(false)
{
	
	CreateControls();
	CenterOnParent();
}

DupFinderDlg2::~DupFinderDlg2() 
{
	multiset_fileinfosize_it it;
	list<fileinfo>::iterator it2;

	wxLogWindow *logw = (wxLogWindow *)wxLog::GetActiveTarget();
	wxLog::SetActiveTarget(NULL);
	delete logw;
	
	// delete sortedbysize
	// are there memory leaks if i don't delete 
	// the equalfiles-list?
	for(it = sortedbysize.begin(); it != sortedbysize.end(); it++) {
		for(it2 = (*it)->files.begin(); 
			it2 != (*it)->files.end();
			it2++) {
			erase(*it2);
		}
		/*for(it3 = (*it)->equalfiles.begin(); 
			it3 != (*it)->equalfiles.end();
			it3++) {
			for(it4 = it3->files.begin(); 
				it4 != it3->files.end();
				it4++) {
				erase (*it4);
			}
		}*/

		delete *it;
	}
}

void DupFinderDlg2::OnClose(wxCloseEvent &WXUNUSED(event)) 
{
	assert(false);
	EndModal(0);
}

void DupFinderDlg2::CreateControls()
{
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *progresssizer = new wxStaticBoxSizer(wxVERTICAL, this, _T("Progress"));
	wxBoxSizer *nfilessizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *cfilessizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *progress2sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *progress2sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *controls = new wxBoxSizer(wxHORIZONTAL);

	const int wxTOPLEFT = wxLEFT | wxTOP;
	const int wxTOPLEFTRIGHT = wxLEFT | wxTOP | wxRIGHT;
	const int wxTOPRIGHT = wxRIGHT | wxTOP;

	topsizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Step 2: \nSearching for duplicates")), 
		0, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	progresssizer->Add(
		wStep1 = new wxStaticText(this, ID_STEP1, _T("1) Searching files: In directory ")), 
		0, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	progresssizer->Add(
		wDirName = new wxTextCtrl(this, ID_SEARCHDIRNAME, _T("----"), 
			wxDefaultPosition, wxSize(300, wxDefaultSize.GetHeight()), wxTE_READONLY), 
		0, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	nfilessizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Files: ")), 
		0, 
		wxTOP,
		10);

	nfilessizer->Add(
		wnFiles = new wxStaticText(this, ID_NFILES, _T("")), 
		1, 
		wxTOPLEFT, 
		10);

	cfilessizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Considered files: ")), 
		0, 
		wxTOP,
		10);

	cfilessizer->Add(
		wcFiles = new wxStaticText(this, ID_CFILES, _T("")), 
		1, 
		wxTOPLEFT, 
		10);

	progress2sizer->Add(
		new wxStaticText(this, wxID_STATIC, _T("Progress: ")), 
		0, 
		wxTOPLEFT, 
		10);

	progress2sizer->Add(
		wProgress = new wxStaticText(this, ID_PROGRESS, _T("")), 
		1, 
		wxTOPLEFT, 
		10);	

	progress2sizer_2->Add(
		new wxStaticText(this, wxID_STATIC, _T("Speed: ")), 
		0, 
		wxTOPLEFT, 
		10);

	progress2sizer_2->Add(
		wSpeed = new wxStaticText(this, ID_SPEED, _T("")), 
		1,
		wxTOPLEFTRIGHT, 
		10);
		


	progresssizer->Add(
		nfilessizer, 
		0, 
		wxLEFT | wxRIGHT, 
		10);

	progresssizer->Add(
		cfilessizer, 
		0, 
		wxLEFT | wxRIGHT, 
		10);

	progresssizer->Add(
		wStep2 = new wxStaticText(this, ID_STEP2, _T("2) Comparing files: ")), 
		0, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	progresssizer->Add(
		progress2sizer, 
		0, 
		wxEXPAND, 
		10);

	progresssizer->Add(
		progress2sizer_2, 
		0, 
		wxEXPAND | wxBOTTOM, 
		10);


	topsizer->Add(
		progresssizer, 
		1, 
		wxTOPLEFTRIGHT | wxEXPAND, 
		10);

	controls->Add(
		new wxCheckBox(this, ID_SHOWMESSAGES, _T("&Show messages")), 
		1, 
		wxTOPLEFT | wxALIGN_CENTER_VERTICAL, 
		10); 

	controls->Add(
		new wxButton(this, wxID_CANCEL, _T("&Cancel")), 
		0, 
		wxTOPLEFTRIGHT | wxALIGN_RIGHT, 
		10);

	topsizer->Add(
		controls, 
		0, 
		wxBOTTOM | wxEXPAND, 
		10);

	// disable <enter> ?
	SetAffirmativeId(wxID_ANY);

	SetSizer(topsizer);
	topsizer->SetSizeHints(this);

	((wxCheckBox *)FindWindow(ID_SHOWMESSAGES))->SetValue(true);
}

void DupFinderDlg2::OnSize(wxSizeEvent &WXUNUSED(event)) 
{
	wxSizer *pSizer = GetSizer();
	if(pSizer) {
		pSizer->SetDimension(0, 0, 
			GetClientSize().GetWidth(), GetClientSize().GetHeight());
		pSizer->Layout();
	}
}

void DupFinderDlg2::OnInitDialog(wxInitDialogEvent &event) {
	wxDialog::OnInitDialog(event);	
}


void DupFinderDlg2::OnIdle(wxIdleEvent &WXUNUSED(event)) {
	// start search if the first idle
	// event is caught
	// a bit dangerous, doing it that way?
	// because what if not becoming idle?

	// that is so to say the main function 
	// of the dialog
	// all in the following if
	// is executed once (!), the results are displayed, 
	// if there are any, 
	// and then the dialog box is closed again

	if(!bStarted) {
		wxFont font, boldfont;

		// do not change this and LEAVE THIS AT THE BEGINNING
		bStarted = true;

		// do not pass messages to old (gui) log target!
		wxLogWindow *logw = new wxLogWindow(NULL, _T("Messages"), true, false);
		wxLog::SetActiveTarget(logw);

		guii.out = wDirName;
		guii.nfiles = wnFiles;
		guii.bContinue = true;
		guii.theApp = wxTheApp;
		guii.cfiles = wcFiles;

		guii.wSpeed = wSpeed;
		guii.wProgress = wProgress;

		font = boldfont = wStep1->GetFont();
		boldfont.SetWeight(wxFONTWEIGHT_BOLD);
		wStep1->SetFont(boldfont);

		ffi.pFilesBySize = &sortedbysize;
		FindFiles(ffi, &guii);

		wStep1->SetFont(font);

		// test if aborted
		if(!guii.bContinue) {
			EndModal(1);
			return;
		}

		wStep2->SetFont(boldfont);

		GetEqualFiles(sortedbysize, &guii);

		wStep2->SetFont(font);

		if(!guii.bContinue) {
			EndModal(1);
			return;
		}

		Hide();
		((wxLogWindow *)wxLog::GetActiveTarget())->Show(false);
		
		DupFinderDlg3 * resultdlg;

		resultdlg = new DupFinderDlg3(this, ffi);

		resultdlg->ShowModal();

		delete resultdlg;

		EndModal(0);
	}
}

void DupFinderDlg2::OnCancel(wxCommandEvent &WXUNUSED(event)) {
	guii.bContinue = false;
	// no EndModal / Destroy here!
	// EndModal(0);
}



void DupFinderDlg2::OnShowMessages(wxCommandEvent &WXUNUSED(event)) {
	bool bCheck = 
		((wxCheckBox *)FindWindow(ID_SHOWMESSAGES))->GetValue();

	((wxLogWindow *)wxLog::GetActiveTarget())->Show(bCheck);
}









