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

using namespace std;
#include "os_cc_specific.h"
#include "dbl.h"
#include "largeint.h"
#include "ulargeint.h"
#include "file.h"

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
#define comparefiles comparefiles2
#endif /* defined(BENCHMARK) */

#define REFRESH_INTERVAL 1 /* in seconds */


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



class AddFileToList : public wxDirTraverser
{
public:
	AddFileToList(findfileinfo * pInfo, const pathinfo *ppi, wxULongLong &_nFiles, 
		guiinfo * _guii = NULL) : ffi(pInfo), pi(ppi), guii(_guii), 
		bDirChanged(true), nFiles(_nFiles) {
#ifdef PROFILE
		__OnFile.QuadPart = 0;
		__OnDir.QuadPart = 0;
		__findsize.QuadPart = 0;
		__normalize.QuadPart = 0;
		__finddir.QuadPart = 0;
		__insert.QuadPart = 0;
#endif
		tlast = time(NULL);
	}

	virtual wxDirTraverseResult OnFile(const wxString & filename)
	{
		STARTTIME(__OnFile);
		// fileinfo fi;
		File f;
		fileinfosize fis;
		multiset_fileinfosize_it it2;
		wxULongLong size;
		STARTTIME(__findsize);
		// slow
		size = wxFileName::GetSize(filename);
		STOPTIME(__findsize);
		
		if(size != wxInvalidSize && size > pi->nMaxFileSizeIgnore) {
			// init structure
			STARTTIME(__insert);
			// fi.name = filename;
			// fi.data = NULL;
			f.SetName(filename);

			fis.size = size;
			it2 = ffi->pFilesBySize->find(fis);

			
			if(it2 != ffi->pFilesBySize->end()) {
				it2->files.push_back(f);
			}
			else {
				// the next line actually is not needed
				fis.size = size;
				fis.files.push_back(f);
				ffi->pFilesBySize->insert(fis);
			}
			STOPTIME(__insert);
		}
		STOPTIME(__OnFile);

		nFiles++;

		return UpdateInfo(NULL);
	}

	virtual wxDirTraverseResult OnDir(const wxString &dirname)
	{
		// do NOT follow links! 
		if(IsSymLink(dirname)) {
			return wxDIR_IGNORE;
		}
		
		return UpdateInfo(&dirname);
	}

	wxDirTraverseResult UpdateInfo(const wxString *dirname) 
	{
		if(guii) { 
			guii->theApp->Yield();
			while(guii->bPause && guii->bContinue) {
				wxMilliSleep(10);
				guii->theApp->Yield();
			}
			if(!guii->bContinue) {
				return wxDIR_STOP;
			}

			if(dirname) {
				bDirChanged = true;
				curdir = *dirname;
			}

			time_t tcur;
			tcur = time(NULL);
				
			if(tcur - tlast >= REFRESH_INTERVAL ) {
				if(bDirChanged) {
					guii->out->SetValue(curdir);
				}
				wxString tmp;
				tmp.Printf(_T("%") wxLongLongFmtSpec _T("u file(s), %i size(s)"), 
					nFiles.GetValue(), 
					ffi->pFilesBySize->size());
				guii->nfiles->SetLabel(tmp);
			}

			tlast = tcur;
		}
		return wxDIR_CONTINUE;
	}

private:
	findfileinfo *ffi;
	const pathinfo *pi;
	time_t tlast;
	
	guiinfo *guii;

	// for status display
	wxString curdir;
	bool bDirChanged;
	wxULongLong &nFiles;

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

void	FindFiles(findfileinfo &ffi, guiinfo *guii)
{
	fileinfosize fis;
	list<File>::iterator it;
	multiset_fileinfosize_it it2;
	bool bDeleted;
	wxULongLong nFiles;
	wxULongLong nDroppedFiles;
	list<pathinfo> &paths = ffi.paths;
	list<pathinfo>::const_iterator it3;

#ifdef PROFILE
	LARGE_INTEGER __all = {0, 0};
#endif


	wxLogMessage(_T("Step 1: Searching files... "));

	nFiles = 0;

	for (it3 = paths.begin(); it3 != paths.end(); it3++) {
		wxLogMessage(_T("        ... in \"%s\" ... "), it3->path.c_str());
		
		AddFileToList traverser(&ffi, &*it3, nFiles, guii);
		wxString dirname = it3->path;
		wxDir dir(dirname);
		STARTTIME(__all);
		dir.Traverse(traverser, it3->Mask, 
			wxDIR_FILES | (it3->bGoIntoSubDirs ? wxDIR_DIRS : 0 ) | 
			(it3->bSearchHidden ? wxDIR_HIDDEN : 0));
		STOPTIME(__all);
		// _ftprintf(stderr, _T("\n"));
		if(guii) {
			if(!guii->bContinue) {
				return;
			}
		}

		
	#ifdef PROFILE
		_ftprintf(stderr, _T("all:      % 8.3f\n"), SECONDS(__all));
		_ftprintf(stderr, _T("OnFile:   % 8.3f (%.3f %%) \n"), SECONDS(traverser.__OnFile), 
			SECONDS(traverser.__OnFile)/SECONDS(__all)*100.0);
		_ftprintf(stderr, _T("OnDir:    % 8.3f (%.3f %%) \n"), SECONDS(traverser.__OnDir), 
			SECONDS(traverser.__OnDir)/SECONDS(__all)*100.0);
		_ftprintf(stderr, _T("Rest:     % 8.3f (%.3f %%) \n\n"), SECONDS(__all)-SECONDS(traverser.__OnFile)-SECONDS(traverser.__OnDir), 
			(SECONDS(__all)-SECONDS(traverser.__OnFile)-SECONDS(traverser.__OnDir))/SECONDS(__all)*100.0);
		_ftprintf(stderr, _T("findsize: % 8.3f (%.3f %%) \n\n"), SECONDS(traverser.__findsize), 
			SECONDS(traverser.__findsize)/SECONDS(traverser.__OnFile)*100.0);
		_ftprintf(stderr, _T("insert:   % 8.3f (%.3f %%) \n"), SECONDS(traverser.__insert), 
			SECONDS(traverser.__insert)/SECONDS(traverser.__OnFile)*100.0);
		_ftprintf(stderr, _T("normalize:% 8.3f (%.3f %%) \n"), SECONDS(traverser.__normalize), 
			SECONDS(traverser.__normalize)/SECONDS(traverser.__OnDir)*100.0);
		_ftprintf(stderr, _T("finddir:  % 8.3f (%.3f %%) \n"), SECONDS(traverser.__finddir), 
			SECONDS(traverser.__finddir)/SECONDS(traverser.__OnDir)*100.0);

	#endif
	}

	multiset_fileinfosize_it it2tmp;
	
	nFiles = 0;
	nDroppedFiles = 0;

	for(it2 = ffi.pFilesBySize->begin(); it2 != ffi.pFilesBySize->end(); bDeleted ? it2 : it2++) {
		if(it2->files.size() > 1) {
			bDeleted = false;
			nFiles += it2->files.size();
		}
		else { 
			it2tmp = it2;
			it2++;

			ffi.pFilesBySize->erase(it2tmp);
			bDeleted = true;

			nDroppedFiles++;
		}
	}

	wxLogMessage(_T("        %") wxLongLongFmtSpec _T("u files have to be compared. \n"), 
		nFiles.GetValue());

	if(guii) {
		wxString tmp;
		tmp.Printf(_T("%") wxLongLongFmtSpec _T("u file(s), %u sizes"), 
			nFiles.GetValue(), ffi.pFilesBySize->size());
		guii->cfiles->SetLabel(tmp);
	}


}

void	GetEqualFiles(multiset_fileinfosize & sortedbysize, guiinfo *guii)
{
	list<File>::iterator it, it3;
	multiset_fileinfosize_it it2;
	list<fileinfoequal>::iterator it4;
	int sizeN;
	size_t nSortedBySizeLen;
	time_t tlast, tnow;
	wxULongLong sumsize;
	wxULongLong nDoubleFiles;
	wxULongLong nDifferentFiles;
	wxString output;
	bool bStart = true;

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
	multiset_fileinfosize_it __it2;
	list<fileinfoequal>::iterator __it4;
#endif /*defined(TEST) */
	

	
	wxLogMessage(_T("Step 2: Comparing files with same size for equality... "));

	tlast = time(NULL);

	sizeN = 0;

	nDifferentFiles = 0;
	sumsize = 0;
	nDoubleFiles = 0;

	STARTTIME(comparetime);

#ifdef BENCHMARK

	list<fileinfosize> __oldlist = sortedbytsize;

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

	sortedbysize = __oldlist;

#endif /* BENCHMARK*/

	nSortedBySizeLen = sortedbysize.size();
	
	for(it2 = sortedbysize.begin(); it2 != sortedbysize.end(); it2++) {
		// printf("size %" I64 ": %i file(s) \n", it2->size.QuadPart, it2->files.size());
		sizeN++;
		tnow = time(NULL);
		if(tnow - tlast >= REFRESH_INTERVAL || bStart == true) {
			bStart = false; // at the beginning information should also be displayed!
			if(guii) {
				output.Printf(_T("size %i/%i (%") wxLongLongFmtSpec _T("u bytes)"), 
					sizeN, nSortedBySizeLen, it2->size.GetValue() );
				guii->wProgress->SetLabel(output);
				guii->wSpeed->SetLabel(_T("--"));
			}
			else {
				deleteline(output.Length());
				output.Printf(_T("size %i/%i (%i files of size %") wxLongLongFmtSpec _T("u)")
					/*" %i kb/s" */, 
					sizeN, nSortedBySizeLen, it2->files.size(), it2->size.GetValue() /*, 0*/);

				_ftprintf(stderr, _T("%s"), output.c_str());
			}
			tlast = tnow;	
		}
		assert(it2->files.size() > 1);
		if(it2->files.size() > 1) { /* in fact, this isn't neccesarry any more */
			bool bDeleted3;
			bool bFirstDouble;
			fileinfoequal fiq;
			list<File>::iterator ittmp;
			bool bEqual;
			for(it = it2->files.begin(); it != it2->files.end(); /*it++*/) {
				bFirstDouble = true;
				it3 = it;
				for(it3++; it3 != it2->files.end(); bDeleted3 ? it3 : it3++) {
					bDeleted3 = false;
					bEqual = comparefiles(*it, *it3, guii);

					if(guii) {
						if(!guii->bContinue) {
							return;
						}
					}

#ifdef TEST
					if(bEqual != comparefiles0(*it, *it3)) { assert(0 == 1); abort(); }
#endif
					if(bEqual) {
						if(bFirstDouble) {
							bFirstDouble = false;
							fiq.files.clear();
							fiq.files.push_back(*it);
							fiq.files.push_back(*it3);
							it2->equalfiles.push_back(fiq);
						}
						else {
							it2->equalfiles.back().files.push_back(*it3);
						}

						nDoubleFiles++;
						sumsize += it2->size;
						
						bDeleted3 = true;
						ittmp = it3;
						it3++;
						ittmp->Close();
						it2->files.erase(ittmp);
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
					for(__it3++; __it3 != it2->files.end(); __it3++) {
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
				it2->files.front().Close();
				it2->files.pop_front();
				it = it2->files.begin();
			}
		} /* if size > 1 */
		/* Close files */
		for(it = it2->files.begin(); it != it2->files.end(); it++) {
			it->Close();
		} 
		it2->files.clear();
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

	deleteline(output.Length());
	wxLogMessage(_T("        done. \n"));

	wxLogMessage(_T("Found %") wxLongLongFmtSpec _T("u files, of which exist at least one more copy. "), 
		nDifferentFiles.GetValue());
	wxLogMessage(_T("%") wxLongLongFmtSpec _T("u duplicates consume altogether %") wxLongLongFmtSpec
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
	for(__it2 = sortedbysize.begin(); __it2 != sortedbysize.end(); __it2++) {
		for(__it4 = (*__it2)->equalfiles.begin(); __it4 != (*__it2)->equalfiles.end(); __it4++) {
			for(__it = (*__it4).files.begin(); __it != (*__it4).files.end(); __it++) {
				__it5 = __it;
				for(__it5++; __it5 != (*__it4).files.end(); __it5++) {
					if(!comparefiles0((*__it), (*__it5))) {
						_ftprintf(stderr, _T("Error: %s != %s\n"), 
							(*__it).name.c_str(), 
							(*__it5).name.c_str());
					}
				}
			}
		}
	}
#endif

}

void	PrintResults(multiset_fileinfosize &sortedbysize, wxFile & fOutput, bool bReverse)
{
	list<File>::const_iterator it, it3;
	multiset_fileinfosize::const_iterator it2;
	multiset_fileinfosize::const_reverse_iterator rit2;
	list<fileinfoequal>::const_iterator it4;
	wxString Buffer;
	// bConOut because output to console does NOT support unicode/utf-8/... in windows (but it does in unix! )
	wxPlatformInfo platform;
	bool bConOut = (fOutput.GetKind() == wxFILE_KIND_TERMINAL) && 
		(platform.GetOperatingSystemId() & wxOS_WINDOWS);
	bool bDisplayWarning = false;

	_ftprintf(stderr, _T("Printing the results...\n\n"));

	for(
		rit2 = sortedbysize.rbegin(), it2 = sortedbysize.begin();
		bReverse ? rit2 != ((const multiset_fileinfosize &)sortedbysize).rend() : it2 != sortedbysize.end();
		rit2++, it2++) {


		for(
			bReverse ? it4 = rit2->equalfiles.begin() : it4 = it2->equalfiles.begin();
			bReverse ? it4 != rit2->equalfiles.end() : it4 != it2->equalfiles.end(); 
			it4++) {
			Buffer.Printf(_T("- Equal (%i files of size %") wxLongLongFmtSpec _T("u): \r\n"), 
				it4->files.size(), 
				bReverse ? rit2->size.GetValue() : it2->size.GetValue());
			if(bConOut) {
				fOutput.Write(Buffer.ToAscii(), Buffer.Length());
				if(!Buffer.IsAscii()) {
					bDisplayWarning = true;
				}
			}
			else {
				fOutput.Write(Buffer, bConOut ? (wxMBConv &)wxConvUTF8 : (wxMBConv &)wxConvUTF8);
			}
			for(it = it4->files.begin(); it != it4->files.end(); it++) {
				Buffer.Printf(_T("  \"%s\"\r\n"), it->GetName().c_str());
				if(bConOut) {
					fOutput.Write(Buffer.ToAscii(), Buffer.Length());
					if(!Buffer.IsAscii()) {
						bDisplayWarning = true;
					}
				}
				else {
					fOutput.Write(Buffer, bConOut ? (wxMBConv &)wxConvUTF8: (wxMBConv &)wxConvUTF8);
				}
			}
		}
	}

	if(bDisplayWarning) {
		_ftprintf(stderr, _T("\n--- WARNING --- \nThe output contains unicode characters which cannot be displayed correctly \n")
			_T("on the console screen! \nIf you want to get the correct filenames, use the -o option! \n\n"));
	}

}


void	deleteline(int n) {
	int t;
	for(t = 0; t < n; t++) {
		_ftprintf(stderr, _T("\b"));
	}
	for(t = 0; t < n; t++) {
		_ftprintf(stderr, _T(" "));
	}
	for(t = 0; t < n; t++) {
		_ftprintf(stderr, _T("\b"));
	}
}

#if defined(BENCHMARK) || defined(TEST)

bool	comparefiles0(fileinfo &f1, fileinfo &f2) {
	FILE *F1, *F2;
	bool bResult;
	const size_t BUFSIZE = BASEBUFSIZE << 5;
	char b1[BUFSIZE], b2[BUFSIZE];
	size_t n1, n2;
	size_t i;

	F1 = NULL;
	F2 = NULL;
	_tfopen_s(&F1, f1.name, _T("rb"));

	if(F1) {

#ifdef BENCHMARK
		__nFilesOpened++;
#endif

		_tfopen_s(&F2, f2.name, _T("rb"));
		
		if(!F2) {
			f2.data->error = true;
			bResult = false;
			goto End;
		}
#ifdef BENCHMARK
		else {
			__nFilesOpened++;
		}
#endif
	}
	else {
		f1.data->error = true;
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

#ifdef ____notdefined

wxULongLong roundup(const wxULongLong &a, int b)
{
	wxULongLong c;
	c = (a / b + (a % b != 0 ? 1 : 0)) * b;
	return c;
}

bool	comparefiles1(fileinfo &f1, fileinfo &f2, guiinfo *guii) {
#if !defined(BENCHMARKBUFSIZE) || !defined(BENCHMARK)
	static char *b[2] = {NULL, NULL };
	if(b[0] == NULL) { b[0] = new char[File::GetBufSize()]; }
	if(b[1] == NULL) { b[1] = new char[File::GetBufSize()]; }
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
	wxULongLong nBytesRead = 0, nPrevBytesRead = 0;
	wxString output;
	time_t tstart, tcurrent;
	
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

	for(i = 0; i < 2; i++) {
		if(!pfi[i]->data) {
			pfi[i]->data = new filedata;
			// pfi[i]->data->size = size;
			pfi[i]->data->nFirstBytes = 0;
			pfi[i]->data->nMaxFirstBytes = 0;
			pfi[i]->data->firstbytes = NULL;
			pfi[i]->data->pFile = new wxFile();
			pfi[i]->data->error = false;
		}
	}


	// do not try to open files unneccessarily often
	// assume that if a file could once not be opened, 
	// it will never be opened (makes the results also 
	// more reliable)
	if(pfi[0]->data->error || pfi[1]->data->error) {
		bResult = false;
		goto End;
	}

	tstart = time(NULL);

	// _ftprintf(stderr, _T("%s <-> %s\n"), pfi[0]->name, pfi[1]->name);
	
	for(i = 0; i < 2; i++) {
		nOffset[i] = 0;
		seeked[i] = false;
	}

	while(true) {

		if(guii) {
			wxTheApp->Yield();
			while(guii->bPause && guii->bContinue) {
				wxMilliSleep(10);
				wxTheApp->Yield();
			}
			if(!guii->bContinue) {
				return false;
			}
			
		}
		for(i = 0; i < 2; i++) {
			usingbuffer[i] = 
				pfi[i]->data->firstbytes && nOffset[i] < pfi[i]->data->nFirstBytes;

			if(usingbuffer[i]) {
				assert(!nOffset[i].GetHi());
				pbuf[i] = pfi[i]->data->firstbytes + nOffset[i].GetLo();
				/* (int)nOffset[i].QuadPart works, because nOffset[i].QuadPart < pfi[i]->data->nFirstBytes 
				   look up, but nevertheless not very nice, perhaps convert all integers 
				   to ULARGE_INTEGER (better also with big files)? */
				n[i] = min(pfi[i]->data->nFirstBytes - (unsigned long)nOffset[i].GetLo(), (unsigned long)BUFSIZE);
			}
			else {
				if(!pfi[i]->data->pFile->IsOpened()) {
					bool bOpenResult;
					STARTTIME(disk);
					STARTTIME(fileopen);
					bOpenResult = pfi[i]->data->pFile->Open(pfi[i]->name);
					STOPTIME(fileopen);
					STOPTIME(disk);
#ifdef BENCHMARK
					__nFilesOpened++;
#endif /* BENCHMARK */
					if(!bOpenResult) {
						pfi[i]->data->error = true;
						bResult = false;
						goto End;
					}
					pfi[i]->data->nMaxFirstBytes = (unsigned long)min(wxULongLong(MAXFIRSTBYTES).GetValue(), 
						roundup(pfi[i]->data->pFile->Length(), BUFSIZE).GetValue());
					pfi[i]->data->firstbytes = new char[pfi[i]->data->nMaxFirstBytes];
					pfi[i]->data->nFirstBytes = 0;
				}
				else if(!seeked[i]) {
					wxFileOffset pos;
					STARTTIME(disk);
					STARTTIME(fileseek);
					pos = pfi[i]->data->pFile->Seek((wxFileOffset)nOffset[i].GetValue());
					STOPTIME(fileseek);
					STOPTIME(disk);
					if(pos == wxInvalidOffset) {
						pfi[i]->data->error = true;
						bResult = false;
						goto End;
					}
					seeked[i] = true;
				}

				writetofirstbytes[i] = nOffset[i] < pfi[i]->data->nMaxFirstBytes;

				if(writetofirstbytes[i]) {
					assert(!nOffset[i].GetHi());
					pbuf[i] = pfi[i]->data->firstbytes+nOffset[i].GetLo();
				}
				else {
					pbuf[i] = b[i];
				}

				assert(pbuf[i]);
				STARTTIME(disk);
				STARTTIME(fileread);
				n[i] = pfi[i]->data->pFile->Read(pbuf[i], BUFSIZE);
				STOPTIME(fileread);
				STOPTIME(disk);
				if(n[i] == wxInvalidOffset) {
					pfi[i]->data->error = true;
					bResult = false;
					goto End;
				} 
				nBytesRead += n[i];
#ifdef BENCHMARK
				__nBytesRead += n[i];
				__nSectorsRead += n[i]/BASEBUFSIZE + (n[i] % BASEBUFSIZE != 0 ? 1 : 0);
#endif /* BENCHMARK */
				if(writetofirstbytes[i]) {
					pfi[i]->data->nFirstBytes += n[i];
				}
			}

			nOffset[i] += n[i];

		} /* for(i = 0; i < 2; i++) */
	
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


		tcurrent = time(NULL);
		if(tcurrent - tstart >= REFRESH_INTERVAL) {
			// display status
			if(guii) {
				output.Printf(_T(" %.2f mb/sec"), 
					(nBytesRead-nPrevBytesRead).ToDouble()/REFRESH_INTERVAL/1024.0/1024.0);
				guii->wSpeed->SetLabel(output);

			}
			else {
				deleteline(output.Length());
				output.Printf(_T(" %.2f mb/sec"), (nBytesRead-nPrevBytesRead).ToDouble()/REFRESH_INTERVAL/1024.0/1024.0);
				_ftprintf(stderr, _T("%s"), output.c_str());

			}

			nPrevBytesRead = nBytesRead;
			tstart = tcurrent;
		}
		

	} /* while(true) */

End:
	deleteline(output.Length());
	return bResult;
}

#endif



bool	comparefiles2(File &f1, File &f2, guiinfo *guii) {
	bool bResult;
	static char *b1, *b2;
	if(b1 == NULL) { b1 = new char[File::GetBufSize()]; }
	if(b2 == NULL) { b2 = new char[File::GetBufSize()]; }
	int BUFSIZE = File::GetBufSize();
	int n1, n2;

	if(!f1.Open()) {
		return false;
	}
	if(!f2.Open()) {
		return false;
	}

	// seek to the beginning
	if( !f1.Seek(wxULongLong(0)) ) {
		return false;
	}
	if( !f2.Seek(wxULongLong(0)) ) {
		return false;
	}

	while(1) {
		if(guii) {
			guii->theApp->Yield();
			while(guii->bPause && guii->bContinue) {
				wxMilliSleep(10);
				guii->theApp->Yield();
			}
			if(!guii->bContinue) {
				return false;
			}
		}

		n1 = n2 = BUFSIZE;
		bool br1, br2;

		br1 = f1.Read(b1, n1);
		br2 = f2.Read(b2, n2);

#ifdef BENCHMARK
		__nBytesRead += n1;
		__nBytesRead += n2;
		__nSectorsRead += n1/BASEBUFSIZE + (n1%BASEBUFSIZE != 0 ? 1 : 0);
		__nSectorsRead += n2/BASEBUFSIZE + (n2%BASEBUFSIZE != 0 ? 1 : 0);
#endif

		if(n1 != n2 || !br1 || !br2) {
			bResult = false;
			goto End;
		}

		if(memcmp(b1, b2, n1) != 0) {
			bResult = false;
			goto End;
		}

		if(n1 < BUFSIZE)
			break;

	}

	bResult = true;

End:	return bResult;
}






