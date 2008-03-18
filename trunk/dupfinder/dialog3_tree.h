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

	void OnTreeItemRightClick(wxTreeEvent &);
	void OnTreeItemActivated(wxTreeEvent &);

	void OnOpenFile(wxCommandEvent &); 
	void OnOpenDir(wxCommandEvent &); 
	void OnCopyFileName(wxCommandEvent &);
	void OnDelete(wxCommandEvent &);
	void OnSymLink(wxCommandEvent &);
	void OnHardLink(wxCommandEvent &);
	
	void OpenDir(const wxTreeItemId &);
	void OpenFile(const wxTreeItemId &);

	void ReturnToParent();

	void OnTreeKeyDown(wxTreeEvent &);

	void DeleteFiles();

	int GetSelectedFilenameCount(const wxArrayTreeItemIds &);
	int GetFirstSelectedFilename(const wxArrayTreeItemIds &);
	int GetNextSelectedFilename(const wxArrayTreeItemIds &, unsigned int i);

	void DeleteOrphanedHeaders();

	void OnCancel(wxCommandEvent &);

	/*
	void OnApply(wxCommandEvent &);
	void OnShowAll(wxCommandEvent &);

	void RestrictViewToDir(const wxString &);

	void OnRestToDir(wxCommandEvent &);
	void OnRestToSDir(wxCommandEvent &);
	
	void MenuRestToDir(bool);

	*/
	void OnGetDir(wxCommandEvent &);

	void CreateLink(bool (*)(const wxString &, const wxString &), const wxString &);

	bool IsMatching(const wxString &);

	void OnMaskChange(wxCommandEvent &);
	void OnDirChange(wxCommandEvent &);

	void OnExpandAll(wxCommandEvent &);
	void OnCollapseAll(wxCommandEvent &);

private:
	static bool bHardLinkWarning;

	findfileinfo ffi;

	wxTreeCtrl *wResultList;
	wxCheckBox *wConfDelete;
	wxTextCtrl *wDirName;
	wxCheckBox *wSubDirs;
	wxCheckBox *wRestrictToDir;
	wxCheckBox *wRestrictToMask;
	wxTextCtrl *wMask;
	
	DupFinderDlg *parent;

	wxFileName RestrictToDir;
	bool bRestrictToDir;
	bool bRestrictToMask;

	wxTreeItemId rightClickedItem;

	DECLARE_EVENT_TABLE()
};


#endif
