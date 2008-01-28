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

#ifndef DIALOG_3_H
#define DIALOG_3_H

#include "stdinc.h"
using namespace std;
#include "dbl.h"

class DupFinderDlg3 : public wxDialog
{
public:
	DupFinderDlg3(wxWindow *, findfileinfo &);
	
	~DupFinderDlg3();

	void OnClose(wxCloseEvent &);
	void OnSize(wxSizeEvent &);

	void OnStore(wxCommandEvent &);

	void CreateControls();
	void DisplayResults();

private:

	findfileinfo &ffi;

	wxListView *wResultList;
	wxCheckBox *wReverse;

	wxLog *oldlogtarget;

	DECLARE_EVENT_TABLE()
};

enum {
	ID_RESULTLIST = 1, 
	ID_REVERSE, 
	ID_STORE
};


#endif
