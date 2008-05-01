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
#include "os_cc_specific.h"

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
	wxString Include;
	wxString Exclude;
};

// that is still subject to change
struct GuiInfo {
	// vars for step 1
	wxTextCtrl *out;
	wxStaticText *nfiles;
	bool bContinue;
	wxApp * theApp;
	wxStaticText *cfiles;
	bool bPause;
	wxStaticText *wStep1;

	// vars for step 2
	wxStaticText *wSpeed;
	wxStaticText *wProgress;
	wxStaticText *wStep2;

	// fonts 
	wxFont normalfont, boldfont;
};

struct DuplicateFilesStats
{
	wxULongLong nDuplicateFiles;
	wxULongLong nWastedSpace;
	wxULongLong nFilesWithDuplicates;

	// to be implemented
	wxULongLong nBytesRead;
	double fAverageSpeed;
};

// this is the class which does all the work

class DuplicateFilesFinder
{
public:
	DuplicateFilesFinder(GuiInfo * _gui, bool _bQuiet) 
		: bQuiet(_bQuiet), gui(_gui) {}
	~DuplicateFilesFinder() {}

	void AddPath(const SearchPathInfo &path) {
		paths.push_back(path);
	}

	void Reset() {
		paths.clear();
		duplicates.clear();
		sortedbysize.clear();
	}

	void FindDuplicateFiles();

	list<DuplicatesGroup> &GetDuplicates() { return duplicates; }

	// that later will be changed
	void SetGui(GuiInfo *_gui) { gui = _gui; }

	void CalculateStats(DuplicateFilesStats &_stats) const;

private:
	// private structures

	struct fileinfoequal
	{
		list<File> files;
	};

	struct fileinfosize
	{
		wxULongLong size;
		list<File> files;
		list<fileinfoequal> equalfiles;
	};

	
	struct less_fileinfosize : public less<fileinfosize> {
		bool operator () (const fileinfosize &a, const fileinfosize &b) const {
			return a.size > b.size;
		}
	};

	typedef multiset<fileinfosize, less_fileinfosize> multiset_fileinfosize;

	
	class AddFileToList : public wxExtDirTraverser
	{
	public:
		AddFileToList(multiset_fileinfosize &, const SearchPathInfo *, wxULongLong &, GuiInfo *);

		virtual wxDirTraverseResult OnFile(const wxString &, const wxULongLong *);
		virtual wxDirTraverseResult OnFile(const wxString & filename);

		virtual wxDirTraverseResult OnExtFile(const FileData &data);

		virtual wxDirTraverseResult OnDir(const wxString &dirname);

		wxDirTraverseResult UpdateInfo(const wxString *dirname);

	private:
		multiset_fileinfosize & sortedbysize;
		const SearchPathInfo *pi;
		time_t tlast;
	
		GuiInfo *guii;

		// for status display
		wxString curdir;
		bool bDirChanged;
		wxULongLong &nFiles;
		list<wxString> ExcludeMasks;

	#ifdef PROFILE
	public:
		LARGE_INTEGER __OnFile;
		LARGE_INTEGER __OnDir;
		LARGE_INTEGER __findsize;
		LARGE_INTEGER __normalize;
		LARGE_INTEGER __finddir;
		LARGE_INTEGER __insert;
	#endif

	};

private:
	// private variables

	list<SearchPathInfo> paths;

	bool bQuiet;

	// subject to change
	GuiInfo * gui;

	multiset_fileinfosize sortedbysize;

	list<DuplicatesGroup> duplicates;


private:
	// private procedures
	
	void	FindFiles();
	void	GetEqualFiles();

	bool 	CompareFiles(File &, File &, const wxULongLong &);

	// prevent direct copies
	DuplicateFilesFinder(const DuplicateFilesFinder &) {}
	DuplicateFilesFinder &operator =(const DuplicateFilesFinder &) { return *this; }


};

#endif /* defined(__DBL_H_123) */

