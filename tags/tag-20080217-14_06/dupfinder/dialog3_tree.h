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

#ifndef DIALOG_3_H
#define DIALOG_3_H

#include "stdinc.h"
using namespace std;
#include "dbl.h"
#include "dialog1.h"

class DupFinderDlg3 : public wxDialog
{
public:
	DupFinderDlg3(DupFinderDlg *, findfileinfo &);
	
	~DupFinderDlg3();

	void OnClose(wxCloseEvent &);
	void OnSize(wxSizeEvent &);

	void OnStore(wxCommandEvent &);

	void CreateControls();
	void DisplayResults();

	void OnInitDialog(wxInitDialogEvent &);

	void OnListItemRightClick(wxListEvent &);
	void OnListItemActivated(wxListEvent &);

	void OnOpenFile(wxCommandEvent &);
	void OnOpenDir(wxCommandEvent &);
	void OnCopyFileName(wxCommandEvent &);
	void OnDelete(wxCommandEvent &);

	void OpenDir(long i);
	void OpenFile(long i);

	void ReturnToParent();

	void OnListKeyDown(wxListEvent &);

	void DeleteFiles();

	void GetSelectedFilenameCount(int &count);
	int GetFirstSelectedFilename();
	int GetNextSelectedFilename(int i);

	void DeleteOrphanedHeaders();

	void OnCancel(wxCommandEvent &);

	void OnApplyDir(wxCommandEvent &);
	void OnShowAll(wxCommandEvent &);

	void RestrictViewTo(const wxString &);

	void ClearList();

	void OnRestToDir(wxCommandEvent &);

	void UpdateView();

	void OnDlgChange(wxCommandEvent &);

	void OnGetDir(wxCommandEvent &);

private:

	findfileinfo ffi;

	wxListView *wResultList;
	wxCheckBox *wConfDelete;
	wxTextCtrl *wDirName;
	wxStaticText *wRestrictInfo;
	
	DupFinderDlg *parent;

	wxFileName RestrictToDir;
	bool bRestrict;

	DECLARE_EVENT_TABLE()
};


#endif
