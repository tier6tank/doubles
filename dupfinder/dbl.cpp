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

#include "stdinc.h"

using namespace std;
#include "os_cc_specific.h"
#include "dbl.h"
#include "file.h"

/********* options ************/

#if defined(__UNIX__) && !defined(_WIN32)

// for unix, there is no native routine implemented, 
// so use wxWidgets
#define FINDFILES_USE_WXWIDGETS

#else

// for windows, there is a native routine, 
// which is much faster than the wxWidget's one

#ifndef FINDFILES_USE_WXWIDGETS
// #define FINDFILES_USE_WXWIDGETS
#endif

#endif /* defined(__UNIX__) && !defined(_WIN32) */


// option, which controls profiling
// on windows platforms
// include "profile.h" always after this definition!
// #define PROFILE

#include "profile.h"


// for testing
// #define TEST
// #define RANDOM_NOISE

#define REFRESH_INTERVAL 1 /* in seconds */


#ifdef PROFILE
LARGE_INTEGER all;
LARGE_INTEGER disk;
LARGE_INTEGER comparetime;
LARGE_INTEGER fileseek;
LARGE_INTEGER fileopen;
LARGE_INTEGER fileread;
#endif /* defined(PROFILE) */


#ifdef TEST
#include "filetest.cpp"
#endif


/******************************************************************************************/
/********************************                       ***********************************/
/******************************** DuplicateFilesFinder  ***********************************/
/********************************    public methods     ***********************************/
/********************************                       ***********************************/
/******************************************************************************************/


DuplicateFilesFinder::DuplicateFilesFinder(GuiInfo * _gui, bool _bQuiet) 
	: bQuiet(_bQuiet), gui(_gui)
{ 
	Reset(); 
}

DuplicateFilesFinder::~DuplicateFilesFinder()
{
}

void DuplicateFilesFinder::AddPath(const SearchPathInfo &path)
{
	paths.push_back(path);
}

void DuplicateFilesFinder::Reset()
{
	paths.clear();
	duplicates.clear();
	sortedbysize.clear();

	bFirst = true;

	state = prevState 
		= DUPF_STATE_NOT_STARTED_YET;
	
	nBytesRead = nPrevBytesRead
		= nFilesRead 
		= nPrevFilesRead
		= nSizesDone 
		= nPrevSizesDone 
		= nBytesDone
		= nPrevBytesDone
		= 0;

	nSumFiles = nSumSizes
		= nSumBytes
		= 0;

	curdir = wxEmptyString;
	output = wxEmptyString;

	current_spi = NULL;
}

list<DuplicatesGroup> &DuplicateFilesFinder::GetDuplicates()
{
	// only return duplicates if the process of finding
	// them really has been successful and is finished
	assert(state == DUPF_STATE_FINISHED); 
	return duplicates;
}

void DuplicateFilesFinder::FindDuplicateFiles() {

	state = DUPF_STATE_FIND_FILES;

	FindFiles();

	if(YieldAndTestAbort()) {
		return;
	}

	// Would be nice, but MUCH too slow!
	// state = DUPF_STATE_REMOVE_DOUBLE_FILES
	// 
	// RemoveDoubleFiles();

	RemoveUnimportantSizes();

	state = DUPF_STATE_COMPARE_FILES;

	GetEqualFiles();

	if(YieldAndTestAbort()) {
		return;
	}

	ConstructResultList();

	// Really finished!
	state = DUPF_STATE_FINISHED;
}

void DuplicateFilesFinder::CalculateStats(DuplicateFilesStats &stats) const
{
	list<DuplicatesGroup>::const_iterator it;

	stats.nDuplicateFiles = 0;
	stats.nWastedSpace = 0;
	stats.nFilesWithDuplicates = 0;

	for(it = duplicates.begin(); 
		it != duplicates.end(); 
		it++) {

		if(it->files.size() > 1) {
			stats.nFilesWithDuplicates++;
			stats.nDuplicateFiles += it->files.size()-1;
			stats.nWastedSpace += wxULongLong(it->files.size()-1)*it->size;
		}
	}
}

/******************************************************************************************/
/********************************                        **********************************/
/******************************** wxExtDirTraverser impl **********************************/
/********************************                        **********************************/
/******************************************************************************************/

wxDirTraverseResult DuplicateFilesFinder::OnFile(const wxString &filename, const wxULongLong *pSize)
{
	assert(current_spi);
	const SearchPathInfo *spi = current_spi;

	if(IsSymLink(filename)) {
		return UpdateInfo(NULL);
	}

	bool bMatches = false;
	for(list<wxString>::iterator it = ExcludeMasks.begin(); it != ExcludeMasks.end(); it ++) {
		if(filename.Matches(*it) ) {
			bMatches = true;
			break;
		}
	}

	if(bMatches) { 
		return UpdateInfo(NULL);
	}

	File f;
	fileinfosize fis;
	multiset_fileinfosize::iterator it2;
	wxULongLong size;
	STARTTIME(__OnFile);
	STARTTIME(__findsize);
	if(pSize == NULL) {
		// slow
		size = wxFileName::GetSize(filename);
	} else {
		size = *pSize;
	}
	STOPTIME(__findsize);
	const bool bIncludeZeroFiles = true; // later make an option out of this?
		
	bool bFitsMinSize = size >= spi->nMinSize;
	bool bFitsMaxSize = size <= spi->nMaxSize || spi->nMaxSize == 0;

	assert(spi->nMaxSize >= spi->nMinSize || spi->nMaxSize == 0);
		
	if(size != wxInvalidSize && 
		(size != 0 || bIncludeZeroFiles) &&
		bFitsMinSize &&
		bFitsMaxSize) {
		// init structure
		STARTTIME(__insert);
		f.SetName(filename);

		fis.size = size;
		it2 = sortedbysize.find(fis);

			
		if(it2 != sortedbysize.end()) {
			unconst(*it2).files.push_back(f);
		}
		else {
			fis.size = size;
			fis.files.push_back(f);
			sortedbysize.insert(fis);
			nSumSizes++;
		}
		STOPTIME(__insert);
	}
	STOPTIME(__OnFile);

	nSumFiles++;
	nSumBytes += size;

	return UpdateInfo(NULL);
}

wxDirTraverseResult DuplicateFilesFinder::OnDir(const wxString &dirname)
{
	// do NOT follow links! 
	if(IsSymLink(dirname)) {
		return wxDIR_IGNORE;
	}
		
	return UpdateInfo(&dirname);
}

wxDirTraverseResult DuplicateFilesFinder::UpdateInfo(const wxString *dirname) 
{
	if(YieldAndTestAbort()) {
		return wxDIR_STOP;
	}

	if(dirname) {
		curdir = *dirname;
	}

	UpdateStatusDisplay();

	return wxDIR_CONTINUE;
}

/******************************************************************************************/
/********************************                       ***********************************/
/******************************** DuplicateFilesFinder  ***********************************/
/********************************   internal methods    ***********************************/
/********************************                       ***********************************/
/******************************************************************************************/

bool DuplicateFilesFinder::Traverse(const SearchPathInfo *spi) 
{
	int flags = wxDIR_FILES | 
		(spi->bGoIntoSubDirs ? wxDIR_DIRS : 0 ) | 
		(spi->bSearchHidden ? wxDIR_HIDDEN : 0);

	wxStringTokenizer tok_excl(spi->Exclude, GetPathSepChar());

	ExcludeMasks.clear();

	while (tok_excl.HasMoreTokens()) {
		ExcludeMasks.push_back(tok_excl.GetNextToken());
	}

	current_spi = spi;

#if defined( FINDFILES_USE_WXWIDGETS )
	bool bSuccess = true;
	wxStringTokenizer tok(spi->Include, GetPathSepChar());
	while(tok.HasMoreTokens()) {
		wxString curmask = tok.GetNextToken();
		wxDir root(spi->path);
		size_t result = root.Traverse(*this, curmask, flags);
		bSuccess = bSuccess && result != (size_t)-1;
	}

	return bSuccess;
#else
	wxStringTokenizer tok(spi->Include, GetPathSepChar());
	int length = tok.CountTokens();
	wxString *masks = new wxString[length];

	int i = 0;
	while(tok.HasMoreTokens()) {
		masks[i] = tok.GetNextToken();
		i++;
	}
	::Traverse(spi->path, masks, length, flags, *this);

	delete [] masks;

	return true;
#endif /* FINDFILES_USE_WXWIDGETS */
}


void	DuplicateFilesFinder::FindFiles()
{
	fileinfosize fis;
	list<File>::iterator it;
	multiset_fileinfosize::iterator it2;
	list<SearchPathInfo>::iterator it3;

#ifdef PROFILE
	LARGE_INTEGER __all = {0, 0};
#endif


	if(!bQuiet) {
		wxLogMessage(_T("Step 1: Searching files... "));
	}

	for (it3 = paths.begin(); it3 != paths.end(); it3++) {
		if(!bQuiet) {
			wxLogMessage(_T("        ... in \"%s\" ... "), it3->path.c_str());
		}
		
#ifdef PROFILE
		__OnFile.QuadPart = 0;
		__OnDir.QuadPart = 0;
		__findsize.QuadPart = 0;
		__normalize.QuadPart = 0;
		__finddir.QuadPart = 0;
		__insert.QuadPart = 0;
#endif

		STARTTIME(__all);
		Traverse(&(*it3));
		STOPTIME(__all);

		if(YieldAndTestAbort()) {
			return;
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

	UpdateStatusDisplay();
}

void	DuplicateFilesFinder::GetEqualFiles()
{
	list<File>::iterator it, it3;
	multiset_fileinfosize::iterator it2;
	list<fileinfoequal>::iterator it4;

#ifdef TEST
	list<File>::iterator __it3;
	list<File>::iterator __it5, __it;
	multiset_fileinfosize::iterator __it2;
	list<fileinfoequal>::iterator __it4;
#endif /*defined(TEST) */

#ifdef RANDOM_NOISE
	{
		srand(time(NULL));
		const int randmax = 10000;
		for(int i = 0; i < randmax; i++) {
			int j, k;
			list<File>::iterator st1, st2;
			int a, b;
			a = rand() % sortedbysize.size();
			for(it2 = sortedbysize.begin(), j = 0; j < a; it2++, j++) {}
			b = rand() % it2->files.size();
			for(k = 0, it = it2->files.begin(); k < b; k++, it++) { }
			st1 = it;
			a = rand() % sortedbysize.size();
			for(it2 = sortedbysize.begin(), j = 0; j < a; it2++, j++) {}
			b = rand() % it2->files.size();
			for(k = 0, it = it2->files.begin(); k < b; k++, it++) {}
			st2 = it;

			wxString tmp = st2->GetName();
			st2->SetName(st1->GetName());
			st1->SetName(tmp);
		}
	}
#endif /* RANDOM_NOISE */
	

	if(!bQuiet) {
		wxLogMessage(_T("Step 2: Comparing files with same size for equality... "));
	}

	// nSizesDone = 0; // Reset() resets this automatically

	STARTTIME(comparetime);

	for(it2 = sortedbysize.begin(); it2 != sortedbysize.end(); it2++) {
		// printf("size %" I64 ": %i file(s) \n", it2->size.QuadPart, it2->files.size());
		nSizesDone++;
		UpdateStatusDisplay();
		assert(it2->files.size() > 1);
		if(it2->files.size() > 1) { /* in fact, this isn't neccesarry any more */
			bool bDeleted3;
			bool bFirstDouble;
			fileinfoequal fiq;
			list<File>::iterator ittmp;
			bool bEqual;
			for(it = unconst(*it2).files.begin(); it != unconst(*it2).files.end(); /*it++*/) {
				bFirstDouble = true;
				it3 = it;
				for(it3++; it3 != it2->files.end(); bDeleted3 ? it3 : it3++) {
					bDeleted3 = false;
					bEqual = CompareFiles(*it, *it3, it2->size);

					if(YieldAndTestAbort()) {
						return;
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
							unconst(*it2).equalfiles.push_back(fiq);
						}
						else {
							unconst(*it2).equalfiles.back().files.push_back(*it3);
						}

						bDeleted3 = true;
						ittmp = it3;
						it3++;
						ittmp->Close();
						unconst(*it2).files.erase(ittmp);

						nBytesDone += it2->size;
						nFilesRead++;
					}

					UpdateStatusDisplay();

					// nComparedBytes.QuadPart += (*it).size.QuadPart;
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
				unconst(*it2).files.front().Close();
				unconst(*it2).files.pop_front();
				it = unconst(*it2).files.begin();

				nBytesDone += it2->size;
				nFilesRead++;
			} /* for it */
		} /* if size > 1 */
		/* Close files */
		for(it = unconst(*it2).files.begin(); it != unconst(*it2).files.end(); it++) {
			it->Close();
		} 
		unconst(*it2).files.clear();
	}

	STOPTIME(comparetime);

	if(!bQuiet) {
		ResetLine();
		wxLogMessage(_T("        done. \n"));
	}

#ifdef TEST
	for(__it2 = sortedbysize.begin(); __it2 != sortedbysize.end(); __it2++) {
		for(__it4 = __it2->equalfiles.begin(); __it4 != __it2->equalfiles.end(); __it4++) {
			for(__it = __it4->files.begin(); __it != __it4->files.end(); __it++) {
				__it5 = __it;
				for(__it5++; __it5 != __it4->files.end(); __it5++) {
					if(!comparefiles0((*__it), (*__it5))) {
						_ftprintf(stderr, _T("Error: %s != %s\n"), 
							__it->GetName().c_str(), 
							__it5->GetName().c_str());
					}
				}
			}
		}
	}
#endif

}

bool	DuplicateFilesFinder::CompareFiles(File &f1, File &f2, const wxULongLong &size) {
	// for testing
	// return true;

	bool bResult;
	static char *b1, *b2;
	char *pb1, *pb2;
	if(b1 == NULL) { b1 = new char[File::GetBufSize()]; }
	if(b2 == NULL) { b2 = new char[File::GetBufSize()]; }
	unsigned int BUFSIZE = File::GetBufSize();
	unsigned int n1, n2;

	if(!f1.Open(size)) {
		return false;
	}
	if(!f2.Open(size)) {
		return false;
	}

	// seek to the beginning
	if( !f1.Restart() ) {
		return false;
	}
	if( !f2.Restart() ) {
		return false;
	}

	while(1) {
		UpdateStatusDisplay();

		if(YieldAndTestAbort()) {
			return false;
		}

		n1 = n2 = BUFSIZE;
		bool br1, br2;

		pb1 = b1;
		pb2 = b2;

		br1 = f1.Read(&pb1, n1);
		br2 = f2.Read(&pb2, n2);

		nBytesRead += n1 + n2;
		

		if(n1 != n2 || !br1 || !br2) {
			bResult = false;
			goto End;
		}

		if(memcmp(pb1, pb2, n1) != 0) {
			bResult = false;
			goto End;
		}

		if(n1 < BUFSIZE)
			break;
	}

	bResult = true;

End:	ResetLine();
	return bResult;
}

void DuplicateFilesFinder::UpdateStatusDisplay()
{
	time_t tcurrent;
	wxString tmp;
	wxString tmp2;

	tcurrent = time(NULL);

	if(tcurrent - tlast >= REFRESH_INTERVAL || bFirst) {
		if(gui) {
			if(prevState != state) {
				switch(state) {
				case DUPF_STATE_FIND_FILES:
					gui->wStep1->SetFont(gui->boldfont);
					break;
				case DUPF_STATE_COMPARE_FILES:
					gui->wStep1->SetFont(gui->normalfont);

					gui->out->Disable();
					gui->out->SetValue(_T(""));

					gui->wStep2->SetFont(gui->boldfont);

					break;
				default:
					break;
				}
			} /* if (prevState != state) */

			if((prevState == DUPF_STATE_FIND_FILES &&
				state > DUPF_STATE_FIND_FILES) || 
				state == DUPF_STATE_FIND_FILES) {
				tmp.Printf(_T("%") wxLongLongFmtSpec _T("u file(s), %")
					wxLongLongFmtSpec _T("u size(s)"), 
					nSumFiles.GetValue(), 
					nSumSizes.GetValue());

				gui->nfiles->SetLabel(tmp);
			}
				
			switch(state) {
			case DUPF_STATE_FIND_FILES:
				gui->out->SetValue(curdir);

				break;
			case DUPF_STATE_COMPARE_FILES:
				tmp.Printf(_T("%") wxLongLongFmtSpec _T("u file(s), %")
					wxLongLongFmtSpec _T("u size(s)"), 
					nSumFiles.GetValue(), 
					nSumSizes.GetValue());
				gui->nfiles->SetLabel(tmp);

				// tmp.Printf(_T("%") wxLongLongFmtSpec _T("u file(s), %u sizes"), 
				// 	nFiles2.GetValue(), sortedbysize.size());
				// gui->cfiles->SetLabel(tmp);

				tmp.Printf(_T("size %") wxLongLongFmtSpec _T("u/%")
					wxLongLongFmtSpec _T("u (%.2f %%)(%") wxLongLongFmtSpec _T("u bytes, %u files)"), 
					nSizesDone.GetValue(), nSumSizes.GetValue(), 
					nSizesDone.ToDouble()/nSumSizes.ToDouble()*100, 
					/*it2->size.GetValue(), it2->files.size()*/
					wxULongLong(0).GetValue(), 0);

				tmp2.Printf(_T(" %.2f / %.2f (%.2f %%) "),
					nBytesDone.ToDouble()/1024/1024, 
					nSumBytes.ToDouble()/1024/1024, 
					nBytesDone.ToDouble()/nSumBytes.ToDouble()*100);
				tmp += tmp2;

				tmp2.Printf(_T(" %") wxLongLongFmtSpec _T("u / ")
					_T("%") wxLongLongFmtSpec _T("u (%.2f %%)"), 
					nFilesRead.GetValue(), 
					nSumFiles.GetValue(), 
					nFilesRead.ToDouble()/nSumFiles.ToDouble()*100.0);
				tmp += tmp2;
				
				
				gui->wProgress->SetLabel(tmp);

				tmp.Printf(_T("%.2f mb/sec"), 
					(nBytesRead-nPrevBytesRead).ToDouble()/REFRESH_INTERVAL/1024/1024
					);
				gui->wSpeed->SetLabel(tmp);

				// gui->wProgressGauge->SetValue((nBytesDone*1000/nSumBytes).GetValue());
				gui->wProgressGauge->SetValue((nFilesRead*1000/nSumFiles).ToULong());
				break;
			default:
				break;
			}
		}
		else if(!bQuiet && state == DUPF_STATE_COMPARE_FILES) {
			ResetLine();
			output.Printf(_T(" %.2f mb/sec"), (nBytesRead-nPrevBytesRead).ToDouble()
				/REFRESH_INTERVAL/1024.0/1024.0);

			tmp.Printf(_T("size %")
				wxLongLongFmtSpec _T("u/%")
				wxLongLongFmtSpec _T("u (%i files of size %") wxLongLongFmtSpec _T("u)")
				/*" %i kb/s" */, 
				nSizesDone.GetValue(), nSumSizes.GetValue(), 
				0, wxULongLong(0).GetValue()/*it2->files.size(), it2->size.GetValue() */);

			output += tmp;

			_ftprintf(stderr, _T("%s"), output.c_str());

		}

		nPrevBytesRead = nBytesRead;
		nPrevSizesDone = nSizesDone;
		nPrevFilesRead = nFilesRead;
		nPrevBytesDone = nBytesDone;
		prevState = state;
		tlast = tcurrent;
		bFirst = false;
	}
	// Sleep(1);
}

void DuplicateFilesFinder::ResetLine() 
{
	if(!bQuiet) {
		int n = output.Length();
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
}

void DuplicateFilesFinder::ConstructResultList()
{
	multiset_fileinfosize::iterator it1;
	list<fileinfoequal>::iterator it2;
	DuplicatesGroup dupl;

	for(it1 = sortedbysize.begin(); 
		!sortedbysize.empty(); 
		it1 = sortedbysize.begin()) {
		dupl.size = it1->size;
		for(it2 = unconst(*it1).equalfiles.begin(); 
			!it1->equalfiles.empty(); 
			it2 = unconst(*it1).equalfiles.begin()) {

			dupl.files = it2->files;
			// immediately delete memory not needed, to be keeping memory 
			// usage low
			duplicates.push_back(dupl);
			unconst(*it1).equalfiles.erase(it2);
		}
		sortedbysize.erase(it1);
	}

	assert(sortedbysize.empty());
}

bool DuplicateFilesFinder::YieldAndTestAbort()
{
	if(gui) {
		gui->theApp->Yield();
		while(gui->bPause && gui->bContinue) {
			wxMilliSleep(10);
			gui->theApp->Yield();
		}

		if(!gui->bContinue) {
			state = DUPF_STATE_ERROR;
			return true;
		}
	}
	return false;
}

#if 0
void DuplicateFilesFinder::RemoveDoubleFiles() {
	multiset_fileinfosize::iterator it;
	list<File>::iterator it2;
	list<File>::iterator it3;
	bool bDeleted;

	for(it = sortedbysize.begin(); it != sortedbysize.end(); it++) {
		for(it2 = it->files.begin(); it2 != it->files.end(); it2++) {
			it3 = it2;
			for(it3++; it3 != it->files.end(); bDeleted ? it3 : it3++) {
				bDeleted = false;
				if(it2->GetNormName() == it3->GetNormName()) {
					list<File>::iterator it3tmp;
					it3tmp = it3;
					it3++;
					it->files.erase(it3tmp);
					bDeleted = true;
				}
			}
		}
	}
}
#endif

void DuplicateFilesFinder::RemoveUnimportantSizes() {

	multiset_fileinfosize::iterator it2;
	bool bDeleted;
	multiset_fileinfosize::iterator it2tmp;
	
	/* sort all sizes out where only one file has that particular size */

	for(it2 = sortedbysize.begin(); it2 != sortedbysize.end(); bDeleted ? it2 : it2++) {
		if(it2->files.size() > 1) {
			bDeleted = false;
		}
		else { 
			nSumBytes -= it2->size;

			it2tmp = it2;
			it2++;

			sortedbysize.erase(it2tmp);
			bDeleted = true;

			nSumFiles--;
		}
	}

	if(!bQuiet) {
		wxLogMessage(_T("        %") wxLongLongFmtSpec _T("u files have to be compared. \n"), 
		nSumFiles.GetValue());
	}
}
