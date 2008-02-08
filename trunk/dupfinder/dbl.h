/******************************************************************************
    dupfinder - search for duplicate files
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

#define BASEBUFSIZE 512

struct filedata
{
	unsigned long nFirstBytes;
	unsigned long nMaxFirstBytes;
	char* firstbytes;
	wxFile *pFile;
	bool error;
};

struct fileinfo
{
	wxString name;
	filedata *data;
};

struct fileinfoequal
{
	list<fileinfo> files;
};


struct fileinfosize
{
	wxULongLong size;
	list<fileinfo> files;
	list<fileinfoequal> equalfiles;
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

struct pathinfo
{
	wxString path;
	wxULongLong nMaxFileSizeIgnore;
	bool bGoIntoSubDirs;
	bool bSearchHidden;
	wxString Mask;
};


struct findfileinfo
{
	multiset_fileinfosize *pFilesBySize;
	list<pathinfo> paths;
};


struct guiinfo {
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


/// prototypes


bool	comparefiles0(fileinfo &, fileinfo &);
bool	comparefiles1(fileinfo &, fileinfo &, guiinfo * = NULL);
wxULongLong roundup(const wxULongLong &, int);
void	deleteline(int);
void	erase(fileinfo &);

void	FindFiles(findfileinfo &, guiinfo * = NULL);
void	GetEqualFiles(multiset_fileinfosize &, guiinfo * = NULL);
void	PrintResults(multiset_fileinfosize &, wxFile &, bool);
void	RemoveDoubleFiles(findfileinfo &);



#endif /* defined(__DBL_H_123) */

