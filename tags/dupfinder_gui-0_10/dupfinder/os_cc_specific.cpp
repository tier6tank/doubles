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

bool IsSymLink(const wxString &) {
	// windows has no symlinks
	return false;
}

#endif


#ifdef __UNIX__

bool IsSymLink(const wxString &filename) {
	struct stat st;

	if(lstat(filename, &st) == 0) {
		return S_ISLNK(st.st_mode) ? true : false;
	}
	else {
		return false;
	}
}

#endif

