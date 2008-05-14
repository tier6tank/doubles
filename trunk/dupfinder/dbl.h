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

struct GuiInfo {
	// control
	bool bContinue;
	bool bPause;

	// vars for step 1
	wxStaticText *wStep1;

	wxTextCtrl *out;
	wxApp * theApp;

	wxStaticText *nfiles;
	// wxStaticText *cfiles;

	// vars for step 2
	wxStaticText *wStep2;

	wxStaticText *wSpeed;
	wxStaticText *wProgress;
	wxGauge *wProgressGauge;

	// fonts 
	wxFont normalfont, boldfont;
};

struct DuplicateFilesStats
{
	wxULongLong nDuplicateFiles;
	wxULongLong nWastedSpace;
	wxULongLong nFilesWithDuplicates;
};

// this is the class which does all the work

class DuplicateFilesFinder : public wxExtDirTraverser
{
public:
	DuplicateFilesFinder(GuiInfo * _gui, bool _bQuiet);
	~DuplicateFilesFinder();

	void AddPath(const SearchPathInfo &path);

	void Reset();

	void FindDuplicateFiles();

	list<DuplicatesGroup> &GetDuplicates();

	void SetGui(GuiInfo *_gui) { gui = _gui; }

	void CalculateStats(DuplicateFilesStats &) const;

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


private: // private variables

	// paths to search in
	list<SearchPathInfo> paths;

	// display options
	// console output?
	bool bQuiet;
	// for status display
	GuiInfo * gui;

	// the final results
	list<DuplicatesGroup> duplicates;

	// internal data for computation
	// of duplicate files
	multiset_fileinfosize sortedbysize;
	list<wxString> ExcludeMasks;
	const SearchPathInfo *current_spi;

	enum DupFinderState { 
		DUPF_STATE_ERROR = -1, 
		DUPF_STATE_NOT_STARTED_YET = 0, 
		DUPF_STATE_FIND_FILES, 
		// DUPF_STATE_REMOVE_DOUBLE_FILES, 
		DUPF_STATE_COMPARE_FILES, 
		DUPF_STATE_FINISHED
	};
	DupFinderState state, prevState;

	// variables, which determine
	// whether status display 
	// should be updated
	time_t tlast;
	bool bFirst;
	
	// in console version last 
	// string written on console
	// (needed for deleteline)
	wxString output;

	// statistics (speed)
	wxULongLong nBytesRead;
	wxULongLong nPrevBytesRead;
	wxULongLong nFilesRead;
	wxULongLong nPrevFilesRead;
	wxULongLong nSizesDone;
	wxULongLong nPrevSizesDone;

	// All bytes (not these only read)
	// Range from 0 to nSumBytes!
	wxULongLong nBytesDone; 
	wxULongLong nPrevBytesDone; 

	// total amount of files/bytes to read/sizes to process
	wxULongLong nSumFiles;
	wxULongLong nSumSizes;
	wxULongLong nSumBytes;

	// current directory searched in for status display
	wxString curdir;

private:
	// private procedures
	
	void	FindFiles();
	void	GetEqualFiles();
	void 	ConstructResultList();

	bool	YieldAndTestAbort();

	bool 	CompareFiles(File &, File &, const wxULongLong &);

	bool 	Traverse(const SearchPathInfo *);

	// void RemoveDoubleFiles();
	void RemoveUnimportantSizes();

	// status output
	void UpdateStatusDisplay();
	void ResetLine();

	// prevent direct copies
	DuplicateFilesFinder(const DuplicateFilesFinder &) : wxExtDirTraverser() {}
	DuplicateFilesFinder &operator =(const DuplicateFilesFinder &) { return *this; }

public: // wxExtDirTraverser Funktionen

	virtual wxDirTraverseResult OnFile(const wxString &, const wxULongLong *);

	virtual wxDirTraverseResult OnFile(const wxString & filename) {
		return OnFile(filename, NULL);
	}

	virtual wxDirTraverseResult OnExtFile(const FileData &data) {
		return OnFile(data.name, &data.size);
	}

	virtual wxDirTraverseResult OnDir(const wxString &dirname);

private:
	wxDirTraverseResult UpdateInfo(const wxString *dirname);


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

#endif /* defined(__DBL_H_123) */

