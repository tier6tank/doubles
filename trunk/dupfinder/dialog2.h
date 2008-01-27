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

#ifndef __DIALOG_2__H
#define __DIALOG_2__H

#include "dbl.h"

class DupFinderDlg2 : public wxDialog 
{
public:
	DupFinderDlg2(findfileinfo, wxWindow *);
	~DupFinderDlg2();
	
	void OnClose(wxCloseEvent &);
	void OnSize(wxSizeEvent &);

	void CreateControls();

	void OnInitDialog(wxInitDialogEvent &);

	void OnIdle(wxIdleEvent &);
	void OnCancel(wxCommandEvent &);

	void OnShowMessages(wxCommandEvent &);
	
private:

	findfileinfo ffi;
	multiset_fileinfosize sortedbysize;

	bool bStarted;

	wxTextCtrl * wDirName;
	wxStaticText *wnFiles;
	wxStaticText *wcFiles;

	wxStaticText *wProgress;
	wxStaticText *wSpeed;

	guiinfo guii;

	wxStaticText *wStep1, *wStep2;

	DECLARE_EVENT_TABLE()

};

enum {
	ID_SEARCHDIRNAME = 1, 
	ID_NFILES, 
	ID_SHOWMESSAGES, 
	ID_CFILES, 
	ID_PROGRESS, 
	ID_SPEED, 
	ID_STEP1, 
	ID_STEP2
};


#endif

