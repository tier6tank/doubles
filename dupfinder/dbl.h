/******************************************************************************
    Duplicate Files Finder - search for duplicate files
    Copyright (C) 2007-2008 Matthias Boehm

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

******************************************************************************/


#ifndef __DBL_H_123
#define __DBL_H_123

#include "file.h"

struct DuplicatesGroup
{
	wxULongLong size;
	list<File> files;
};

struct SearchPathInfo
{
	wxString path;
	wxULongLong nMinSize;
	wxULongLong nMaxSize; // if 0, no max size
	bool bGoIntoSubDirs;
	bool bSearchHidden;
	wxString Mask;
};

struct GuiInfo {
	// vars for step 1
	wxTextCtrl *out;
	wxStaticText *nfiles;
	bool bContinue;
	wxApp * theApp;
	wxStaticText *cfiles;
	bool bPause;

	// vars for step 2
	wxStaticText *wSpeed;
	wxStaticText *wProgress;
};

struct DuplicateFilesStatistics
{


};

// this is the class which does all the work

class DuplicateFilesFinder
{
public:
	DuplicateFilesFinder(GuiInfo * _gui, bool _bQuiet) 
		: gui(_gui), bQuiet(_bQuiet) {}
	~DuplicateFilesFinder() {}

	void AddPath(const SearchPathInfo &path) {
		paths.push_back(path);
	}
	
	void FindDuplicateFiles(list<DuplicatesGroup> & /* , DuplicateFilesStatistics &stats*/);
private:
	list<SearchPathInfo> paths;

	bool bQuiet;
	GuiInfo * gui;

};

// temporary
typedef SearchPathInfo pathinfo;
typedef GuiInfo guiinfo;



struct fileinfoequal
{
	list<File> files;
};


struct fileinfosize
{
	wxULongLong size;
	// mutable because set iterators are always const
	// but files and equalfiles are not influencing the 
	// order of a set (size is the only relevant member for 
	// ordering), so it's save to declare them mutable
	mutable list<File> files;
	mutable list<fileinfoequal> equalfiles;
};


struct less_fileinfosize : public less<fileinfosize> {
	bool operator () (const fileinfosize &a, const fileinfosize &b) const {
		// the commented code does not work because i don't get bReverse from somewhere, 
		// so now bReverse is used in PrintResults

		//if (bReverse) {
		//	return a.size < b.size;
		// else {
		// bigger because i want to have BIG files first by default!
		return a.size > b.size;
		// }
	}
};
	
typedef multiset<fileinfosize, less_fileinfosize> multiset_fileinfosize;
typedef multiset<fileinfosize, less_fileinfosize>::iterator multiset_fileinfosize_it;
typedef multiset<fileinfosize, less_fileinfosize>::reverse_iterator multiset_fileinfosize_rit;


struct findfileinfo
{
	multiset_fileinfosize *pFilesBySize;
	list<pathinfo> paths;
};





// prototypes
bool 	comparefiles(File &, File &, const wxULongLong &, guiinfo * /*= NULL*/, bool = false);
void	deleteline(int);

void	FindFiles(findfileinfo &, guiinfo * /*= NULL*/, bool = false);
void	GetEqualFiles(multiset_fileinfosize &, guiinfo * /*= NULL*/, bool = false);



#endif /* defined(__DBL_H_123) */

