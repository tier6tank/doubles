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

#if !defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1400)

// sprintf_s, scanf_s, ... aren't defined
// so wrap them into the old functions (where buffer overflows unfortunately *can* occur!)

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) ((x) = (x))
#endif

int _stprintf_s(_TCHAR *buffer, int buflen, const _TCHAR *format, ... ) {
	UNREFERENCED_PARAMETER(buflen);

	va_list marker;
	va_start (marker, format);
	int nResult;

	nResult = _vstprintf(buffer, format, marker);

	va_end(marker);
	return nResult;
}

_TCHAR * _tcscpy_s(_TCHAR *a, int nLength, const _TCHAR *b) {
	UNREFERENCED_PARAMETER(nLength);
	return _tcscpy(a, b);
}

_TCHAR * _tcscat_s(_TCHAR *a, int nLength, const _TCHAR *b) {
	UNREFERENCED_PARAMETER(nLength);
	return _tcscat(a, b);
}

int _tfopen_s(FILE **ppf, const _TCHAR *filename, const _TCHAR *mode) {
	*ppf = _tfopen(filename, mode);
	return (*ppf != NULL);
}


#endif /* !defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1400) */

#ifdef _WIN32

void Traverse(const wxString &RootDir, const wxString &mask, int flags, wxExtDirTraverser &sink) 
{
	WIN32_FIND_DATA fd;
	wxString Dir;
	wxString Search;
	HANDLE hFind;
	bool bNoEnd;
	bool bEnd = false;

	wxFileName tmp = wxFileName::DirName(RootDir);

	Dir = tmp.GetPathWithSep();

	// first search directories

	if(flags & wxDIR_DIRS && !bEnd) {

		Search = Dir;
		Search.Append(_T("*"));

		hFind = FindFirstFile(
			Search, 
			&fd);

		if(hFind == INVALID_HANDLE_VALUE)
			return;

		do
		{
			if(!(_tcscmp(fd.cFileName, _T(".")) == 0 || 
			   _tcscmp(fd.cFileName, _T("..")) == 0) || flags & wxDIR_DOTDOT) {
				if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY 
					&& (!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || flags & wxDIR_HIDDEN)) {
					wxString Down;
					Down = Dir;
					Down.Append(fd.cFileName);

					wxDirTraverseResult result = sink.OnDir(Down);

					if(result == wxDIR_STOP) {
						bEnd = true;
						break; 
					}
				
					if(result != wxDIR_IGNORE) {
						Traverse(Down, mask, flags, sink); 
					}
				}
			}

			bNoEnd = FindNextFile(hFind, &fd);

		} while(bNoEnd);

		FindClose(hFind);
	}


	// then search files

	if(flags & wxDIR_FILES && !bEnd) {

		Search = Dir;
		if(mask == wxEmptyString) {
			Search.Append(_T("*"));
		} else {
			Search.Append(mask);
		}

		hFind = FindFirstFile(
			Search, 
			&fd);

		if(hFind == INVALID_HANDLE_VALUE) {
			return;
		}

		do
		{
			if(!(_tcscmp(fd.cFileName, _T(".")) == 0 || 
			   _tcscmp(fd.cFileName, _T("..")) == 0) || flags & wxDIR_DOTDOT) {
				if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					&& (!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || flags & wxDIR_HIDDEN)) {
					FileData data;
					data.name = Dir;
					data.name.Append(fd.cFileName);
					data.size = wxULongLong(fd.nFileSizeHigh, fd.nFileSizeLow);
					
					wxDirTraverseResult result = sink.OnExtFile(data);
					
					if(result == wxDIR_STOP) {
						bEnd = true;
						break;
					}
				}
			}

			bNoEnd = FindNextFile(hFind, &fd);
	
		} while(bNoEnd);

		FindClose(hFind);
	}

}

bool IsSymLink(const wxString &) {
	// windows has no symlinks
	return false;
}

bool IsSymLinkSupported() {
	return false;
}

bool CreateSymLink(const wxString &WXUNUSED(d1), const wxString &WXUNUSED(d2)) {
	// don't even try it
	return false;
}

bool IsHardLinkSupported() {
	wxPlatformInfo pi;
	// is the NT family the only one which supports 
	// hardlinks? What about Microwindows, win CE?
	if(!(pi.GetOperatingSystemId() & wxOS_WINDOWS_NT)) {
		return false;
	} else {
		return true;
	}
}

bool CreateHardLink(const wxString &oldpath, const wxString &newpath) {
	return CreateHardLink(newpath.fn_str(), oldpath.fn_str(), NULL);
}

#endif


#if defined( __UNIX__ ) && !defined(_WIN32)
/*
void Traverse(const wxString &RootDir, const wxString &mask, int flags, for_each_file_func function, void *pData) 
{
	const _TCHAR * array[2] = { pRootDir, NULL };
	FTS *hFind;
	FTSENT *fe;
	wxString FileName;
	wxULongLong size;

	errno = 0;
	hFind = fts_open((_TCHAR *const*)array, FTS_PHYSICAL, NULL);

	if(errno != 0) {
		return;
	}

	do {
		fe = fts_read(hFind);

		if(fe != NULL) {
			if(fe->fts_info == FTS_F) {
				_tcscpy_s(ff.cFileName, MAX_PATH, fe->fts_path);
				ff.size.QuadPart = fe->fts_statp->st_size;
				// ff.bDirectory = fe->fts_info == FTS_D;
				function(&ff, pData);
			}
		}

	} while(fe != NULL);

	fts_close(hFind);
}
*/

bool IsSymLink(const wxString &filename) {
	struct stat st;

	if(lstat(filename, &st) == 0) {
		return S_ISLNK(st.st_mode) ? true : false;
	}
	else {
		return false;
	}
}

bool IsSymLinkSupported() {
	// this is not always true, e.g. if 
	// there are filesystems mounted (e.g fat)
	return true;
}

bool CreateSymLink(const wxString &oldpath, const wxString &newpath) {
	return symlink(oldpath, newpath) == 0;
}

bool IsHardLinkSupported() {
	// unix: yes!
	return true;
}

bool CreateHardLink(const wxString &oldpath, const wxString &newpath) {
	return link(oldpath, newpath) == 0;
}

#endif

