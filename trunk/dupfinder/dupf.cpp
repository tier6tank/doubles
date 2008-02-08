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

#include "stdinc.h"
#include "dbl.h"
#include "os_cc_specific.h"
#include "profile.h"

// int _tmain(int argc, _TCHAR *argv[]) 
// {
// a little bit ugly, this DECLARE_MAIN
// that's because of mingw32 build which
// does not support wmain, but still unicode
DECLARE_MAIN
	multiset_fileinfosize sortedbysize;
	findfileinfo ffi;

	list<fileinfo>::iterator it, it3;
	multiset_fileinfosize_it it2;
	list<fileinfoequal>::iterator it4;
	list<pathinfo>::iterator it5;

	clock_t tstart, tend;
	bool bReverse;
	int i;
	int nOptions;
	wxFile fOutput;

	::wxInitialize();
	// timestamps are ugly
	wxLog::GetActiveTarget()->SetTimestamp(NULL);

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
		_ftprintf(stderr, _T("dupf: Finds duplicate files\n"));
		_ftprintf(stderr, _T("Copyright (c) Matthias Boehm 2007-2008\n\n"));
		_ftprintf(stderr, _T("dupf [GENERAL OPTIONS] Path1 [PATH-OPTIONS] [Path2 [PATH-OPTIONS]] ...\n\n"));
		_ftprintf(stderr, _T("General options: \n\n"));
		_ftprintf(stderr, _T("-r  : small files first (default is big files first)\n"));
		_ftprintf(stderr, _T("-o x: Print results to file x (e.g. if the output to stdout is useless \n"));
		_ftprintf(stderr, _T("      because of unicode characters) \n"));
		_ftprintf(stderr, _T("\nOptions for each path: \n\n"));
		_ftprintf(stderr, _T("-i x: Ignore files with size lower or equal to x (in bytes)\n"));
		_ftprintf(stderr, _T("-n  : do not recurse into subdirectories\n"));
		_ftprintf(stderr, _T("-h  : include hidden files in search (default: off)\n"));
		_ftprintf(stderr, _T("-m x: limit your search to those files which match the file mask\n"));
		return 1;
	}

	// defaults
	nOptions = 0;
	fOutput.Attach(wxFile::fd_stdout);
	bReverse = false;

	// get general options
	for(i = 0; i < argc && argv[i][0] == _T('-'); i++) {
		if(_tcscmp(argv[i], _T("-r")) == 0) {
			bReverse = true;
			nOptions += 1;
		}
		else if(_tcscmp(argv[i], _T("-o")) == 0) {
			bool bResult;
			if(!argv[i+1]) {
				_ftprintf(stderr, _T("Error: Filename expected! \n"));
				return 1;
			}
			bResult = fOutput.Create(argv[i+1], true);
			if(!bResult) {
				_ftprintf(stderr, _T("Error: Cannot open %s! \n"), argv[i+1]);
				return 1;
			}
			nOptions += 2;
			i++;
		} else {
			_ftprintf(stderr, _T("Error: unrecognized option %s. \n"), argv[i]);
			return 1;
		}			
	}

	if(argc-nOptions <= 0) {
		_ftprintf(stderr, _T("Error in commandline: arguments expected! \n"));
		return 1;
	}

	for(/*i*/; i < argc; ) {
		pathinfo pi;
		pi.path = argv[i];
		pi.nMaxFileSizeIgnore = 0;
		pi.bGoIntoSubDirs = true;
		pi.bSearchHidden = false;
		pi.Mask = wxEmptyString;

		for(i++; i < argc && argv[i][0] == '-'; i++) {
			if(_tcscmp(argv[i], _T("-i")) == 0) {
				if(argv[i+1]) {
					ULARGE_INTEGER _nMaxFileSizeIgnore;
					if(_stscanf_s(argv[i+1], _T("%") wxLongLongFmtSpec _T("u"), &_nMaxFileSizeIgnore.QuadPart) == 0) {
						_ftprintf(stderr, _T("Error in commandline: Number expected! \n"));
						return 1;
					}
					pi.nMaxFileSizeIgnore = _nMaxFileSizeIgnore.QuadPart;
				} else {
					_ftprintf(stderr, _T("Error in commandline: Number expected! \n"));
					return 1;
				}
				i++;
			}
			else if(_tcscmp(argv[i], _T("-n")) == 0) {
				pi.bGoIntoSubDirs = false;
			}
			else if(_tcscmp(argv[i], _T("-h")) == 0) {
				pi.bSearchHidden = true;
			}
			else if(_tcscmp(argv[i], _T("-m")) == 0) {
				if(!argv[i+1]) {
					_ftprintf(stderr, _T("Error: expression expected! \n"));
					return 1;
				}
				pi.Mask = argv[i+1];
				i ++;
			}
			else {
				_ftprintf(stderr, _T("Error: unrecognized option %s. \n"), argv[i]);
				return 1;
			}
		}
		ffi.paths.push_back(pi);
	}

	list<wxString> dirs;
	for(it5 = ffi.paths.begin(); it5 != ffi.paths.end(); it5++) {
		if(!wxFileName::DirExists(it5->path)) {
			_ftprintf(stderr, _T("Error: \"%s\" does not exist! \n"), it5->path.c_str());
			return 1;
		}


		wxFileName dir = wxFileName::DirName(it5->path.c_str());
		dir.Normalize(wxPATH_NORM_ALL | wxPATH_NORM_CASE); 

		dirs.push_back(dir.GetFullPath());
	}

	list<wxString>::iterator it;
	bool bDisplayedHeader = false;
	for(it = dirs.begin(); it != dirs.end(); it++) {
		list<wxString>::iterator it2;
		it2 = it;
		for(it2++; it2 != dirs.end(); it2++) {
			// C:\dir1\dir2 C:\dir\dir3 is correct
			// C:\dir1  C:\dir1\dir2 not

			if(it->StartsWith(*it2) || it2->StartsWith(*it)) {
				bool bCase = it->StartsWith(*it2);
				if(!bDisplayedHeader) {
					_ftprintf(stderr, _T("\nWARNING: \n---------\n"));
					bDisplayedHeader = true;
				}
				_ftprintf(stderr, _T("The Path \"%s\" is contained in \"%s\"! \n"), 
					bCase ? it->c_str() : it2->c_str(), bCase ? it2->c_str() : it->c_str() );
			}			
		}
	}

	if(bDisplayedHeader) {
		_ftprintf(stderr, _T("Correct the command line for avoiding trivial duplicates. \n\n\n"));
	}

	ffi.pFilesBySize = &sortedbysize;

	// BEGINLOG, ENDLOG -> log errors temporarily to string-stream

#define BEGINLOG { xxx\
		/* for error output */\
		ostrstream log; \
		wxLogStream logstr(&log); \
		wxLog * poldlog = wxLog::SetActiveTarget(&logstr); \
		logstr.SetTimestamp(NULL); \

#define ENDLOG /* print errors */ \
		if(log.pcount() != 0) { xxx\
			_ftprintf(stderr, _T("\n        The following errors occured: \n")); \
			fwrite(log.str(), log.pcount(), 1, stderr); \
			_ftprintf(stderr, _T("\n")); \
		} \
		 \
		wxLog::SetActiveTarget(poldlog); \
	} \


	// BEGINLOG

	FindFiles(ffi);
	// RemoveDoubleFiles(ffi); // perhaps this IS necessary some time

	// ENDLOG


	// no wxWidgets errors possible here
	// SortFilesBySize(files, sortedbysize);

	// for error testing (e.g. delete files before GetEqualFiles)
	// int i;
	// scanf("%i", &i);

	// BEGINLOG
	
	GetEqualFiles(sortedbysize);

	// ENDLOG

	
	PrintResults(sortedbysize, fOutput, bReverse);

	sortedbysize.clear();


	fOutput.Close();

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

#ifdef TEST
	_ftprintf(stderr, _T("all test ok! \n"));
#endif

	::wxUninitialize();
	
	return 0;
}

