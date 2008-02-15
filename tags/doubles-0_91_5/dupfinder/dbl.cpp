/******************************************************************************
    dbl - search duplicate files
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

/* TODO: do not search files twice or more (for example by the commandline ..\ .\ or .\ .\: such cases, 
         when a previously searched directory is once again searched
	need a close look to signed/unsigned int32/int64 types!
	erase not needed any more? */

#include "stdinc.h"

using namespace std;
#include "os_cc_specific.h"
#include "dbl.h"
#include "largeint.h"
#include "ulargeint.h"

/********* options ************/

// #define BENCHMARK
// #define TEST
// #define PROFILE
// #define BENCHMARKBUFSIZE

/********* further, of the upper dependent options **********/



#ifdef BENCHMARK
#ifndef TESTCNT
#define TESTCNT 10
#endif /*!defined(TESTCNT) */
#ifndef TESTFUNC
#define TESTFUNC 1
#endif /* !defined(TESTFUNC) */
#endif /* BENCHMARK */

/*********************************/

#include "profile.h"

#ifdef BENCHMARK
#ifdef BENCHMARKBUFSIZE
int __MinBufSize = BASEBUFSIZE;
int __BufSize;
int __MaxFirstBytes;
#define MAXBUFSIZEMULT 10
#define MAXMAXFIRSTBYTESMULT 20
char *__b[2] = { NULL, NULL };
#endif /* defined(BENCHMARKBUFSIZE) */

wxULongLong __nBytesRead = 0;
wxULongLong __nSectorsRead = 0;
wxULongLong __nFilesOpened = 0;

#else /* if !defined(BENCHMARK) */
#define comparefiles comparefiles1
#endif /* defined(BENCHMARK) */

#define REFRESH_INTERVAL 1 /* in seconds */

bool	comparefiles0(fileinfo &, fileinfo &);
bool	comparefiles1(fileinfo &, fileinfo &);
wxULongLong roundup(const wxULongLong &, int);
void	deleteline(void);
void	erase(fileinfo &);

void	FindFiles(findfileinfo &, _TCHAR *[], int);
void	SortFilesBySize(list<fileinfo> &, list<fileinfosize> &, bool);
void	GetEqualFiles(list<fileinfosize> &);
void	PrintResults(list<fileinfosize> &, FileHandle *);
void	RemoveDoubleFiles(findfileinfo &);

#ifdef BENCHMARK
typedef bool (*__comparefunc)(fileinfo &, fileinfo &);
__comparefunc __functions[] = {comparefiles0, comparefiles1 };
#define comparefiles __functions[TESTFUNC]
#endif /* defined(BENCHMARK) */


#ifdef PROFILE
LARGE_INTEGER all;
LARGE_INTEGER disk;
LARGE_INTEGER comparetime;
LARGE_INTEGER fileseek;
LARGE_INTEGER fileopen;
LARGE_INTEGER fileread;
#endif /* defined(PROFILE) */

// int _tmain(int argc, _TCHAR *argv[]) 
// {
// a little bit ugly, this DECLARE_MAIN
// that's because of mingw32 build which
// does not support wmain, but still unicode
DECLARE_MAIN
	list<fileinfo> files;
	list<fileinfosize> orderedbysize;
	findfileinfo ffi;
	ULARGE_INTEGER _nMaxFileSizeIgnore;
	wxULongLong nMaxFileSizeIgnore;

	list<fileinfo>::iterator it, it3;
	list<fileinfosize>::iterator it2;
	list<fileinfoequal>::iterator it4;

	clock_t tstart, tend;
	bool bReverse;
	int i;
	int nOptions;
	FileHandle fOutput;
	bool bGoIntoSubDirs;


	::wxInitialize();


#ifdef BENCHMARK
#ifdef _WIN32
	SetPriorityClass(
		GetCurrentProcess(), 
		REALTIME_PRIORITY_CLASS);

	SetThreadPriority(
		GetCurrentThread(), 
		THREAD_PRIORITY_TIME_CRITICAL);
#endif /* defined(_WIN32) */
#endif /* defined(BENCHMARK) */

	INITPROFILE;

	STARTTIME(all);

	tstart = clock();

	argv++;
	argc--;

	if(argc == 0) {
		_ftprintf(stderr, _T("dbl: Finds duplicates\n\n"));
		_ftprintf(stderr, _T("dbl [OPTIONS] Path1 [Path2] [Path3] ...\n\n"));
		_ftprintf(stderr, _T("Options: \n"));
		_ftprintf(stderr, _T("-m x: Take care only of files with size greater than x (in bytes)\n"));
		_ftprintf(stderr, _T("-r  : small files first (default is big files first)\n"));
		_ftprintf(stderr, _T("-f x: Print results to file x (e.g. if the output to stdout is bad \n"));
		_ftprintf(stderr, _T("      because of unicode characters\n"));
		_ftprintf(stderr, _T("-n  : do not recurse into subdirectories\n"));
		return 1;
	}

	nOptions = 0;
	_nMaxFileSizeIgnore.QuadPart = 0;
	fOutput = /*stdout*/GetStdOutputHandle();
	bReverse = false;
	bGoIntoSubDirs = true;

	for(i = 0; i < argc && argv[i][0] == _T('-'); i++) {
		if(_tcscmp(argv[i], _T("-m")) == 0) {
			if(argv[i+1]) {
				if(_stscanf_s(argv[i+1], _T("%") wxLongLongFmtSpec _T("u"), &_nMaxFileSizeIgnore.QuadPart) == 0) {
					nMaxFileSizeIgnore = _nMaxFileSizeIgnore.QuadPart;
					_ftprintf(stderr, _T("Error in commandline: Number expected! \n"));
					return 1;
				}
			} else {
				_ftprintf(stderr, _T("Error in commandline: Number expected! \n"));
				return 1;
			}
			i++;
			nOptions += 2;
		}
		else if(_tcscmp(argv[i], _T("-r")) == 0) {
			bReverse = true;
			nOptions += 1;
		}
		else if(_tcscmp(argv[i], _T("-f")) == 0) {
			if(!argv[i+1]) {
				_ftprintf(stderr, _T("Error: Filename expected! \n"));
				return 1;
			}
			// _tfopen_s(&fOutput, argv[i+1], _T("w"));
			CreateFile(&fOutput, argv[i+1]);
			if(!IsValidFileHandle(&fOutput)) {
				_ftprintf(stderr, _T("Error: Cannot open %s! \n"), argv[i+1]);
				return 1;
			}
			nOptions += 2;
			i++;
		}
		else if(_tcscmp(argv[i], _T("-n")) == 0) {
			bGoIntoSubDirs = false;
			nOptions += 1;
		}
	}

	if(argc-nOptions <= 0) {
		_ftprintf(stderr, _T("Error in commandline: arguments expected! \n"));
		return 1;
	}

	/* add reverse option -r (printing files with lowest/highest size first) */

	ffi.nMaxFileSizeIgnore = nMaxFileSizeIgnore;
	ffi.pFiles = &files;
	ffi.bGoIntoSubDirs = bGoIntoSubDirs;

	FindFiles(ffi, argv+i, argc-nOptions);
	// RemoveDoubleFiles(ffi);

	SortFilesBySize(files, orderedbysize, bReverse);

	GetEqualFiles(orderedbysize);

	PrintResults(orderedbysize, &fOutput);

	CloseFile(&fOutput);


	tend = clock();

	/* Calculation and output of consumed time */

	const int tmpsize = 20;
	_TCHAR szMinutes[tmpsize] = _T("");
	_TCHAR szHours[tmpsize] = _T("");
	_TCHAR szDays[tmpsize] = _T("");
	
	double dseconds = (double)(tend-tstart)/CLOCKS_PER_SEC;
	int sumseconds = (int)dseconds;
	int seconds = sumseconds % 60;
	int minutes = (sumseconds/60) % 60;
	int hours = (sumseconds/60/60) % 24;
	int days = sumseconds/60/60/24;

	if(days) {
		_stprintf_s(szDays, tmpsize, _T("%i d, "), days);
		_stprintf_s(szHours, tmpsize, _T("%i h, "), hours);
		_stprintf_s(szMinutes, tmpsize, _T("%i min, %i sec"), minutes, seconds);
	}
	else if(hours) {
		_stprintf_s(szHours, tmpsize, _T("%i h, "), hours);
		_stprintf_s(szMinutes, tmpsize, _T("%i min, %i sec"), minutes, seconds);
	}
	else if(minutes) { 
		_stprintf_s(szMinutes, tmpsize, _T("%i min, %i sec"), minutes, seconds);
	}
	else {
		_stprintf_s(szMinutes, tmpsize, _T("%.2f sec"), dseconds);
	}

	_ftprintf(stderr, _T("Time: %s%s%s. \n"), szDays, szHours, szMinutes);

	STOPTIME(all);

#ifdef PROFILE
	_ftprintf(stderr, _T("Profile-Times: \n"));
	_ftprintf(stderr, _T("   all: %.3f\n"), SECONDS(all));
	_ftprintf(stderr, _T("   compare-time: %.3f (%.3f %% of all)\n"), 
		SECONDS(comparetime), SECONDS(comparetime)/SECONDS(all)*100);
	_ftprintf(stderr, _T("   disk: %.3f (%.3f %% of all, %.3f %% of comparetime)\n"), 
		SECONDS(disk), SECONDS(disk)/SECONDS(all)*100, SECONDS(disk)/SECONDS(comparetime)*100);
	_ftprintf(stderr, _T("     fileopen: %.3f (%.3f %%)\n"), SECONDS(fileopen), SECONDS(fileopen)/SECONDS(disk)*100);
	_ftprintf(stderr, _T("     fileseek: %.3f (%.3f %%)\n"), SECONDS(fileseek), SECONDS(fileseek)/SECONDS(disk)*100);
	_ftprintf(stderr, _T("     fileread: %.3f (%.3f %%)\n"), SECONDS(fileread), SECONDS(fileread)/SECONDS(disk)*100);
#endif

	::wxUninitialize();
	
	return 0;
}


class AddFileToList : public wxDirTraverser
{
public:
	AddFileToList(findfileinfo * pInfo) : ffi(pInfo) {}

	virtual wxDirTraverseResult OnFile(const wxString & filename)
	{
		fileinfo fi;
		wxULongLong size = wxFileName::GetSize(filename);

		/* important: > (no null-length files! ) */
		if(size > ffi->nMaxFileSizeIgnore) {
			// printf("adding %s\n", ff->cFileName);
			// _tcscpy_s(fi.name, MAX_PATH, filename);
			fi.name = filename;
			fi.size = size; // ff->size;
			fi.nFirstBytes = 0;
			fi.nMaxFirstBytes = 0;
			fi.firstbytes = NULL;
			fi.pFile = new wxFile();
			ffi->pFiles->push_back(fi); 
		}

		return wxDIR_CONTINUE;
	}

	virtual wxDirTraverseResult OnDir(const wxString &dirname)
	{
		wxFileName dir = wxFileName::DirName(dirname);
		dir.Normalize(wxPATH_NORM_ALL | wxPATH_NORM_CASE);

		if(ffi->Dirs.find(dir) != ffi->Dirs.end()) {
			_ftprintf(stderr, _T("\n        Warning: tried to search in %s more than once! "), dir.GetFullPath().c_str());
			return wxDIR_IGNORE;
		}

		pair<set<wxFileName, less_filename>::iterator, bool> result;

		result = ffi->Dirs.insert(dir); 
		assert(result.second == true);

		return wxDIR_CONTINUE;
	}

private:
	findfileinfo *ffi;

};

void	FindFiles(findfileinfo &ffi, _TCHAR * argv[], int argc)
{
	int i;

	_ftprintf(stderr, _T("Step 1: Searching files... \n"));

	for (i = 0; i < argc; i++) {
		size_t nPreviousSize = ffi.pFiles->size();
		_ftprintf(stderr, _T("        ... in \"%s\" ... "), argv[i]);
		AddFileToList traverser(&ffi);
		wxString dirname = argv[i];
		wxDir dir(dirname);
		dir.Traverse(traverser, wxEmptyString, wxDIR_FILES | (ffi.bGoIntoSubDirs ? wxDIR_DIRS : 0 ));
		_ftprintf(stderr, _T("%i files\n"), ffi.pFiles->size() - nPreviousSize);
	}

	// that is not needed any more
	ffi.Dirs.clear();

	_ftprintf(stderr, _T("        done. Will examine %i file(s). \n\n"), ffi.pFiles->size());

}

/*
void	RemoveDoubleFiles(findfileinfo &ffi) 
{
	// this has to be changed, because wxFileName.operator == is grindingly sluggish

	list<fileinfo>::iterator it, it2;
	// remove files which appear more than once
	size_t n, nsum;

	_ftprintf(stderr, _T("Step 1b: Deleting files which appear more than once (this takes a while :-( )... "));

	nsum = ffi.pFiles->size();

	for(it = ffi.pFiles->begin(), n = 0; it != ffi.pFiles->end(); it++, n++) {
		it2 = it;
		it2++;
		bool bDeleted;
		for(it2; it2 != ffi.pFiles->end(); bDeleted ? it2 : it2++) {
			bDeleted = false;
			// speed....
			\* if(it->name.GetExt() == it2->name.GetExt()) {
				if(it->name.GetName() == it2->name.GetName()) { *\
					if(it->name == it2->name) {
						// delete this entry
						list<fileinfo>::iterator ittmp = it2;
						ittmp++;
						ffi.pFiles->erase(it2);
						bDeleted = true;
						it2 = ittmp;
						assert(false);
					}
			\*	}
			}*\
		}
		if(n % 10 == 0) {
			_ftprintf(stderr, _T("%.2f %%\b\b\b\b\b\b\b\b\b\b\b\b\b"), (double)n/nsum*100);
		}
	}

	
	_ftprintf(stderr, _T("done. \n         Will examine %i file(s). \n"), ffi.pFiles->size());
}
*/

void	SortFilesBySize(list<fileinfo> & files, list<fileinfosize> & orderedbysize, bool bReverse)
{
	bool bSizeFound;
	fileinfosize fis;
	list<fileinfo>::iterator it;
	list<fileinfosize>::iterator it2;
	bool bDeleted;
	wxULongLong nFiles;
	
	
	_ftprintf(stderr, _T("Step 2: Sorting files by size (this might take a while)... "));

	for(it = files.begin(); it != files.end(); it++) {
		bool bResult;
		bSizeFound = false;
		for(it2 = orderedbysize.begin(); it2 != orderedbysize.end(); it2++) {
			if(bReverse) {
				bResult = (*it2).size >= (*it).size; 
			}
			else {
				bResult = (*it2).size <= (*it).size;
			}
			if(/*(*it2).size >= (*it).size*/bResult) {
				bSizeFound = (*it2).size == (*it).size;
				break;
			}
		} 

		if(bSizeFound) {
			(*it2).files.push_back(*it);
		}
		else {
			fis.size = (*it).size;
			fis.files.clear();
			fis.files.push_back(*it);
			orderedbysize.insert(it2, fis);
		}
	} 

	/* now i can delete the first file-list */
	files.clear();

	_ftprintf(stderr, _T("done. \n        %i different sizes, "), orderedbysize.size());

	list<fileinfosize>::iterator it2tmp;
	
	// _ftprintf(stderr, _T("Step 3: Delete unimportant sizes... "));

	// nSizes = 0;

	nFiles = 0;

	for(it2 = orderedbysize.begin(); it2 != orderedbysize.end(); bDeleted ? it2 : it2++) {
		if((*it2).files.size() > 1) {
			// nSizes++;
			bDeleted = false;
			nFiles += (*it2).files.size();
		}
		else { 
			it2tmp = it2;
			it2++;
			orderedbysize.erase(it2tmp);
			bDeleted = true;
		}
	}

	_ftprintf(stderr, _T("%i which matter. \n        %") wxLongLongFmtSpec _T("u files have to be compared. \n\n"), 
		orderedbysize.size(), nFiles.GetValue());

}

void	GetEqualFiles(list<fileinfosize> & orderedbysize)
{
	list<fileinfo>::iterator it, it3;
	list<fileinfosize>::iterator it2;
	list<fileinfoequal>::iterator it4;
	int sizeN;
	bool bHeaderDisplayed = false;
	size_t nOrderedBySizeLen;
	time_t tlast, tnow;
	wxULongLong sumsize;
	wxULongLong nDoubleFiles;
	wxULongLong nDifferentFiles;
#ifdef BENCHMARK 
	clock_t __tstart = 0, __tend = 0;
	int __i;
#ifdef BENCHMARKBUFSIZE
	int __x, __y;
#endif /* defined(BENCHMARKBUFSIZE) */
#endif /* defined(BENCHMARK) */
#ifdef TEST
	list<fileinfo>::iterator __it3;
	list<fileinfo>::iterator __it5, __it;
	list<fileinfosize>::iterator __it2;
	list<fileinfoequal>::iterator __it4;
#endif /*defined(TEST) */
	

	
	_ftprintf(stderr, _T("Step 3: Comparing files with same size for equality... "));

	tlast = time(NULL);

	sizeN = 0;

	nDifferentFiles = 0;
	sumsize = 0;
	nDoubleFiles = 0;

	STARTTIME(comparetime);

#ifdef BENCHMARK

	list<fileinfosize> __oldlist = orderedbysize;

#ifdef BENCHMARKBUFSIZE
	FILE *__fLog;
	_tfopen_s(&__fLog, _T("benchmbufsize.log"), _T("w"));
	assert(__fLog);
	for(__y = 0; __y < MAXMAXFIRSTBYTESMULT; __y++) {
		_ftprintf(__fLog, _T(";%i"), __y);
	}
	_ftprintf(__fLog, _T("\n"));
	/* __x BUFSIZE, __y MAXFIRSTBYTES */
	for(__x = 0; __x < MAXBUFSIZEMULT; __x++) {
		__BufSize = __MinBufSize << __x;
		delete [] __b[0];
		delete [] __b[1];
		__b[0] = new char[__BufSize];
		__b[1] = new char[__BufSize];
		
		_ftprintf(__fLog, _T("%i"), __x);
		for(__y = 0; __y < __x; __y++) {
			_ftprintf(__fLog, _T(";"));
		}
	for(__y = __x; __y < MAXMAXFIRSTBYTESMULT; __y++) {
		__MaxFirstBytes = __MinBufSize << __y;
		_ftprintf(stderr, _T("\n%i, %i\n"), __x, __y);
#endif /* defined(BENCHMARKBUFSIZE) */	

	__tstart = clock();

	for(__i = 0; __i < TESTCNT; __i++) {

	orderedbysize = __oldlist;

#endif /* BENCHMARK*/

	nOrderedBySizeLen = orderedbysize.size();
	
	for(it2 = orderedbysize.begin(); it2 != orderedbysize.end(); it2++) {
		// printf("size %" I64 ": %i file(s) \n", (*it2).size.QuadPart, (*it2).files.size());
		sizeN++;
		tnow = time(NULL);
		if(tnow - tlast >= REFRESH_INTERVAL) {
		// if((sizeN -1) % 100 == 0) {
			if(!bHeaderDisplayed) {
				fprintf(stderr, "\n");
				bHeaderDisplayed = true;
			}
			deleteline();
			_ftprintf(stderr, _T("size %i/%i (%i files of size %") wxLongLongFmtSpec _T("u)"), 
				sizeN, nOrderedBySizeLen, (*it2).files.size(), (*it2).size.GetValue());
			tlast = tnow;
		}
		assert((*it2).files.size() > 1);
		if((*it2).files.size() > 1) { /* in fact, this isn't neccesarry any more */
			bool bDeleted3;
			bool bFirstDouble;
			fileinfoequal fiq;
			list<fileinfo>::iterator ittmp;
			bool bEqual;
			for(it = (*it2).files.begin(); it != (*it2).files.end(); /*it++*/) {
				bFirstDouble = true;
				it3 = it;
				for(it3++; it3 != (*it2).files.end(); bDeleted3 ? it3 : it3++) {
					bDeleted3 = false;
					bEqual = comparefiles(*it, *it3);
#ifdef TEST
					if(bEqual != comparefiles0(*it, *it3)) { assert(0 == 1); abort(); }
#endif
					if(bEqual) {
						if(bFirstDouble) {
							bFirstDouble = false;
							fiq.files.clear();
							fiq.files.push_back(*it);
							fiq.files.push_back(*it3);
							(*it2).equalfiles.push_back(fiq);
						}
						else {
							(*it2).equalfiles.back().files.push_back(*it3);
						}

						nDoubleFiles++;
						sumsize += (*it3).size;
							
						bDeleted3 = true;
						ittmp = it3;
						it3++;
						erase(*ittmp);
						(*it2).files.erase(ittmp);
					}
					// nComparedBytes.QuadPart += (*it).size.QuadPart;
				}
				if(!bFirstDouble) {
					nDifferentFiles++;
				}
#ifdef TEST
				/* no doubles found */
				if(bFirstDouble) {
					bool bNotEqual = true;
					__it3 = it;
					for(__it3++; __it3 != (*it2).files.end(); __it3++) {
						bEqual = comparefiles0(*it, *__it3);
						if(bEqual) {
							bNotEqual = false;
							break;
						}
					}
					assert(bNotEqual);
					if(!bNotEqual) { abort(); }
				}
#endif
				erase((*it2).files.front());
				(*it2).files.pop_front();
				it = (*it2).files.begin();
			}
		}
		/* delete all temporary firstbytes-arrays */
		for(it = (*it2).files.begin(); it != (*it2).files.end(); it++) {
			delete [] (*it).firstbytes;
			(*it).firstbytes = NULL;
			(*it).nFirstBytes = (*it).nMaxFirstBytes = 0;
			//- if(IsValidFileHandle(&(*it).fh)) {
			//- 	CloseFile(&(*it).fh);
			//- 	InitFileHandle(&(*it).fh);
			//- }
			if((*it).pFile->IsOpened()) {
				(*it).pFile->Close();
			}
		}
		(*it2).files.clear();	
	}

#ifdef BENCHMARK
	}

	__tend = clock();

#ifdef BENCHMARKBUFSIZE
	_ftprintf(__fLog, _T(";%.6f"), ((double)(__tend-__tstart))/CLOCKS_PER_SEC/TESTCNT);
	fflush(__fLog);
	__MaxFirstBytes <<= 1;
	}
	_ftprintf(__fLog, _T("\n"));
	__BufSize <<= 1; 
	}
#endif
#endif /* BENCHMARK */


	STOPTIME(comparetime);

	if(bHeaderDisplayed) {
		deleteline();
		_ftprintf(stderr, _T("        "));
	}
	_ftprintf(stderr, _T("done. \n\n"));

	_ftprintf(stderr, _T("Found %") wxLongLongFmtSpec _T("u files, of which exist at least one more copy. \n"), nDifferentFiles.GetValue());
	_ftprintf(stderr, _T("%") wxLongLongFmtSpec _T("u duplicates consume altogether %") wxLongLongFmtSpec
		 _T("u bytes (%") wxLongLongFmtSpec _T("u kb, %") wxLongLongFmtSpec _T("u mb)\n"), 
		nDoubleFiles.GetValue(), sumsize.GetValue(), sumsize.GetValue()/1024, sumsize.GetValue()/1024/1024);

#ifdef BENCHMARK
	_ftprintf(stderr, _T("Time/run: %.3f\n"), ((double)(__tend-__tstart))/CLOCKS_PER_SEC/TESTCNT);
	_ftprintf(stderr, _T("Bytes read:   %10.2f (%10") wxLongLongFmtSpec _T("u)\n"), 
		__nBytesRead.GetDouble()/TESTCNT, 
		__nBytesRead.GetValue()/TESTCNT);
	_ftprintf(stderr, _T("Sectors read: %10.2f (%10") wxLongLongFmtSpec _T("u)\n"), 
		__nSectorsRead.GetDouble()/TESTCNT, 
		__nSectorsRead.GetValue()/TESTCNT);
	_ftprintf(stderr, _T("Files opened: %10.2f (%10") wxLongLongFmtSpec _T("u)\n"), 
		__nFilesOpened.GetDouble()/TESTCNT, 
		__nFilesOpened.GetValue()/TESTCNT);
#endif /* BENCHMARK */

#ifdef TEST
	for(__it2 = orderedbysize.begin(); __it2 != orderedbysize.end(); __it2++) {
		for(__it4 = (*__it2).equalfiles.begin(); __it4 != (*__it2).equalfiles.end(); __it4++) {
			for(__it = (*__it4).files.begin(); __it != (*__it4).files.end(); __it++) {
				__it5 = __it;
				for(__it5++; __it5 != (*__it4).files.end(); __it5++) {
					if(!comparefiles0((*__it), (*__it5))) {
						_ftprintf(stderr, _T("Error: %s != %s\n"), 
							(*__it).name.GetFullPath(), (*__it5).name.GetFullPath());
					}
				}
			}
		}
	}
#endif

}

void	PrintResults(list<fileinfosize> &orderedbysize, FileHandle *pfOutput)
{
	list<fileinfo>::iterator it, it3;
	list<fileinfosize>::iterator it2;
	list<fileinfoequal>::iterator it4;
	const int BUFSIZE = 1000;
	_TCHAR Buffer[BUFSIZE];

	_ftprintf(stderr, _T("Step 4: Printing the results...\n"));

	for(it2 = orderedbysize.begin(); it2 != orderedbysize.end(); it2++) {
		for(it4 = (*it2).equalfiles.begin(); it4 != (*it2).equalfiles.end(); it4++) {
			// _ftprintf(fOutput, _T("+ Equal (%i files of size %") wxLongLongFmtSpec _T("u): \n"), 
			// 	(*it4).files.size(), (*it4).files.front().size.QuadPart);
			_stprintf_s(Buffer, BUFSIZE, _T("+ Equal (%i files of size %") wxLongLongFmtSpec _T("u): \r\n"), 
				(*it4).files.size(), (*it4).files.front().size.GetValue());
			WriteString(pfOutput, Buffer);
			for(it = (*it4).files.begin(); it != (*it4).files.end(); it++) {
				_stprintf_s(Buffer, BUFSIZE, _T("- \"%s\"\r\n"), (*it).name.GetFullPath().c_str());
				WriteString(pfOutput, Buffer);
			}
		}
	}

}


void	deleteline(void) {
	int t;
	for(t = 0; t < 79; t++) {
		_ftprintf(stderr, _T("\b"));
	}
	for(t = 0; t < 79; t++) {
		_ftprintf(stderr, _T(" "));
	}
	for(t = 0; t < 79; t++) {
		_ftprintf(stderr, _T("\b"));
	}
}

void	erase(fileinfo &fi) {
	delete [] fi.firstbytes;
	
	//-if(IsValidFileHandle(&fi.fh)) {
	//-	CloseFile(&fi.fh);
	//-	InitFileHandle(&fi.fh);
	//-}
	if(fi.pFile->IsOpened()) {
		fi.pFile->Close(); 
	}
}

#if defined(BENCHMARK) || defined(TEST)

bool	comparefiles0(fileinfo &f1, fileinfo &f2) {
	FILE *F1, *F2;
	bool bResult;
	const int BUFSIZE = BASEBUFSIZE << 5;
	char b1[BUFSIZE], b2[BUFSIZE];
	size_t n1, n2;
	size_t i;

	F1 = NULL;
	F2 = NULL;
	_tfopen_s(&F1, f1.name.GetFullPath(), _T("rb"));
	_tfopen_s(&F2, f2.name.GetFullPath(), _T("rb"));

#ifdef BENCHMARK
	if(F1) { __nFilesOpened++; }
	if(F2) { __nFilesOpened++; }
#endif

	if(!F1 || !F2)
	{
		bResult = false;
		goto End;
	}

	while(1) {
		n1 = fread(b1, 1, BUFSIZE, F1);
		n2 = fread(b2, 1, BUFSIZE, F2);
#ifdef BENCHMARK
		__nBytesRead += n1;
		__nBytesRead += n2;
		__nSectorsRead += n1/BASEBUFSIZE + (n1%BASEBUFSIZE != 0 ? 1 : 0);
		__nSectorsRead += n2/BASEBUFSIZE + (n2%BASEBUFSIZE != 0 ? 1 : 0);
#endif

		if(n1 != n2 || ferror(F1) || ferror(F2)) {
			bResult = false;
			goto End;
		}

		for(i = 0; i < n1; i++) {
			if(b1[i] != b2[i]) {
				bResult = false;
				goto End;
			}
			/* later exchange by md5 - algorithm!!!! */
		}

		if(n1 < BUFSIZE)
			break;

	}

	bResult = true;

End:	if(F1) fclose(F1);
	if(F2) fclose(F2);
	return bResult;
}

#endif /* defined(BENCHMARK) || defined(TEST) */

wxULongLong roundup(const wxULongLong &a, int b)
{
	wxULongLong c;
	c = (a / b + (a % b != 0 ? 1 : 0)) * b;
	return c;
}

bool	comparefiles1(fileinfo &f1, fileinfo &f2) {
#if !defined(BENCHMARKBUFSIZE) || !defined(BENCHMARK)
	const size_t BUFSIZE = BASEBUFSIZE << 7;
	const unsigned long MAXFIRSTBYTES = BASEBUFSIZE << 7;
	static char *b[2] = {NULL, NULL };
	if(b[0] == NULL) { b[0] = new char[BUFSIZE]; }
	if(b[1] == NULL) { b[1] = new char[BUFSIZE]; }
#else
	size_t BUFSIZE = __BufSize;
	unsigned int MAXFIRSTBYTES = __MaxFirstBytes;
	char *b[2] = { __b[0], __b[1] };
#endif /* !defined(BENCHMARKBUFSIZE) || !defined(BENCHMARK) */

	fileinfo *pfi[2] = {&f1, &f2 };
	char *pbuf[2];
	ssize_t n[2];
	wxULongLong nOffset[2];
	bool usingbuffer[2], writetofirstbytes[2];
	bool bResult;
	int i;
	bool seeked[2];

	assert(MAXFIRSTBYTES % BUFSIZE == 0);

	/* 
	while (1) {
		proc buffer exists and contains valid data
			return false if buffer does not exist
			return true if nOffset < nFirstBytes
			n[i] = nFirstBytes - nOffset
		end proc
	
		proc roundup 
			if filesize % BUFSIZE == 0 
				return filesize/BUFSIZE;
			else 
				return filesize/BUFSIZE+1;
			endif
		end proc

		set seeked := false
			
		for each file {
			if buffer exists at current offset and contains valid data
				usingbuffer := true
			end if
			
			if unsingbuffer then
				point at buffer, set n[i] to remaining bytes
			else 
				if not file was opened before 
					open file
					set maxfirstbytes = min(MAXFIRSTBYTES, roundup(filesize)*BUFSIZE)
					create memory for firstbytes of size maxfirstbytes
				else if not seeked [we have reached the end of FIRSTBYTES]
					set offset into file
					seeked := true
				end

				if buffer+nOffset is smaller than maxfirstbytes, 
					that is when there is still place in pfirstbytes 
					set buffer to firstbytes+offset
				else
					set buffer to normal buffer
				end

				read file into buffer
			end

			increase offset by n[i]

			if nOffset[i] > pfi[i].nOffset 
				add checksum to n[i] to pfi[i].checksum
				add n[i] to pfi[i].nOffset
			end if

			** if not usingbuffer
				calculate checksum of current buffer and size
			endif **

		}

		if n[0] != n[1] 
			result =false;
			exit;
		end

		if checksums don't equal and all is read from disk
			result = false;
			exit;
		end

		compare buffers

		if compare-result is false
			result = false;
			exit;
		end

		if n[0] < BUFSIZE
			result = true;
			exit
		end
	}
	*/

	// _ftprintf(stderr, _T("%s <-> %s\n"), pfi[0]->name, pfi[1]->name);
	
	for(i = 0; i < 2; i++) {
		nOffset[i] = 0;
		seeked[i] = false;
	}

	while(true) {
		for(i = 0; i < 2; i++) {
			usingbuffer[i] = 
				pfi[i]->firstbytes && nOffset[i] < pfi[i]->nFirstBytes;

			if(usingbuffer[i]) {
				assert(!nOffset[i].GetHi());
				pbuf[i] = pfi[i]->firstbytes + nOffset[i].GetLo();
				/* (int)nOffset[i].QuadPart works, because nOffset[i].QuadPart < pfi[i]->nFirstBytes 
				   look up, but nevertheless not very nice, perhaps convert all integers 
				   to ULARGE_INTEGER (better also with big files)? */
				n[i] = min(pfi[i]->nFirstBytes - (unsigned long)nOffset[i].GetLo(), (unsigned long)BUFSIZE);
			}
			else {
				//- if(!IsValidFileHandle(&pfi[i]->fh)) {
				if(!pfi[i]->pFile->IsOpened()) {
					bool bOpenResult;
					STARTTIME(disk);
					STARTTIME(fileopen);
					//- OpenFile(&pfi[i]->fh, pfi[i]->name);
					bOpenResult = pfi[i]->pFile->Open(pfi[i]->name.GetFullPath());
					STOPTIME(fileopen);
					STOPTIME(disk);
#ifdef BENCHMARK
					__nFilesOpened++;
#endif /* BENCHMARK */
					//- if(!IsValidFileHandle(&pfi[i]->fh)) {
					if(!bOpenResult) {				
						bResult = false;
						goto End;
					}
					pfi[i]->nMaxFirstBytes = (unsigned long)min(wxULongLong(MAXFIRSTBYTES).GetValue(), 
						roundup(pfi[i]->size, BUFSIZE).GetValue());
					pfi[i]->firstbytes = new char[pfi[i]->nMaxFirstBytes];
					pfi[i]->nFirstBytes = 0;
				}
				else if(!seeked[i]) {
					bool bRes;
					wxFileOffset pos;
					STARTTIME(disk);
					STARTTIME(fileseek);
					//- bRes = SeekFile(&pfi[i]->fh, &nOffset[i]);
					pos = pfi[i]->pFile->Seek((wxFileOffset)nOffset[i].GetValue());
					bRes = pos != wxInvalidOffset;
					STOPTIME(fileseek);
					STOPTIME(disk);
					if(!bRes) {
						bResult = false;
						goto End;
					}
					seeked[i] = true;
				}

				writetofirstbytes[i] = nOffset[i] < pfi[i]->nMaxFirstBytes;

				if(writetofirstbytes[i]) {
					assert(!nOffset[i].GetHi());
					pbuf[i] = pfi[i]->firstbytes+nOffset[i].GetLo();
				}
				else {
					pbuf[i] = b[i];
				}

				assert(pbuf[i]);
				BOOL bRetVal;
				STARTTIME(disk);
				STARTTIME(fileread);
				//- bRetVal = ReadFile(&pfi[i]->fh, pbuf[i], BUFSIZE, (LPDWORD)&n[i]);
				n[i] = pfi[i]->pFile->Read(pbuf[i], BUFSIZE);
				bRetVal = n[i] != wxInvalidOffset;
				STOPTIME(fileread);
				STOPTIME(disk);
				if(!bRetVal) {
					bResult = false;
					goto End;
				} 
#ifdef BENCHMARK
				__nBytesRead += n[i];
				__nSectorsRead += n[i]/BASEBUFSIZE + (n[i] % BASEBUFSIZE != 0 ? 1 : 0);
#endif /* BENCHMARK */
				if(writetofirstbytes[i]) {
					pfi[i]->nFirstBytes += n[i];
				}
			}

			nOffset[i] += n[i];

		}
	
		if(n[0] != n[1]) {
			bResult = false;
			goto End;
		}

		if(memcmp(pbuf[0], pbuf[1], n[0]) != 0) {
			bResult = false;
			goto End;
		}

		if((size_t)n[0] < BUFSIZE) {
			bResult = true;
			goto End;
		}

	}

End:
	return bResult;
}






