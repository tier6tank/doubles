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


#ifndef __DIALOG_1
#define __DIALOG_1

#include "stdinc.h"
using namespace std;

#include "dbl.h"

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

class DupFinderDlg : public wxDialog {
public:
	DupFinderDlg(wxWindow *);

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

	void OnDirNameEnter(wxCommandEvent &event) { OnDirAdd(event); }

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


#endif

