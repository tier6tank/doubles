/******************************************************************************
    dbl - search duplicate files
    Copyright (C) 2007-2008 Matthias Böhm

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

/****************************************************************/

#if !defined(_WIN32)

bool OpenFile(FileHandle *f, const _TCHAR *name) {
	return _tfopen_s(&f->f, name, _T("rb"));
}

bool CreateFile(FileHandle *f, const _TCHAR *name) {
	return _tfopen_s(&f->f, name, _T("wb"));
}

bool 	ReadFile(const FileHandle *f, char * buffer, DWORD nLength, DWORD *pRead) {
	*pRead = fread(buffer, 1, nLength, f->f);
	return !ferror(f->f);
}

bool	CloseFile(FileHandle *f) {
	int val;
	val = fclose(f->f);
	f->f = NULL;
	return val;
}

bool	IsValidFileHandle(const FileHandle *f) {
	return f->f != NULL;
}

void	InitFileHandle(FileHandle *f) {
	f->f = NULL;
}


bool	SeekFile(const FileHandle *f, const ULARGE_INTEGER *pto) {
	int res;
	/* can't seek with c functions above 4 GB - but this 
	   is not needed in my program */
	assert(pto->HighPart == 0);
	if(pto->HighPart != 0) { 
		return false;
	}
	res = fseek(f->f, pto->LowPart, SEEK_SET);
	return res == 0;
}

void	for_each_file(const _TCHAR *pRootDir, for_each_file_func function, void *pData)
{
	const _TCHAR * array[2] = { pRootDir, NULL };
	FTS *hFind;
	FTSENT *fe;
	FindFile ff;

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

FileHandle GetStdOutputHandle() {
	FileHandle f;
	f.f = stdout;
	return f;
}

bool	WriteString(FileHandle *f, const _TCHAR *buffer) {
	_fputts(buffer, f->f);
	return !ferror(f->f);
}


#else /* defined(_WIN32) */

bool OpenFile(FileHandle *f, const _TCHAR *name) {
	DWORD dwFileAttributes = GetFileAttributes(name);
	f->hFile = CreateFile(
		name, 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		dwFileAttributes, 
		NULL);
	return (f->hFile != INVALID_HANDLE_VALUE);
}

bool CreateFile(FileHandle *f, const _TCHAR *name) {
	f->hFile = CreateFile(
		name, 
		GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);
	return (f->hFile != INVALID_HANDLE_VALUE);
}

bool ReadFile(const FileHandle *f, char *buffer, DWORD nLength, DWORD *pRead) {
	BOOL bRetVal;
	bRetVal = ReadFile(f->hFile, buffer, nLength, pRead, NULL);
	
	return bRetVal == TRUE;
}

bool	SeekFile(const FileHandle *f, const ULARGE_INTEGER *pto) {
	ULARGE_INTEGER tmp;
	tmp = *pto;
	tmp.LowPart = SetFilePointer(f->hFile, (LONG)tmp.LowPart, (PLONG)&tmp.HighPart, FILE_BEGIN);
	return !(tmp.LowPart == 0xffffffff && GetLastError() != 0);
}
	

bool	CloseFile(FileHandle *f) {
	BOOL val;
	val = CloseHandle(f->hFile);
	f->hFile = INVALID_HANDLE_VALUE;
	return val != 0;
}

bool	IsValidFileHandle(const FileHandle *f) {
	return f->hFile != INVALID_HANDLE_VALUE;
}

void InitFileHandle(FileHandle *f) {
	f->hFile = INVALID_HANDLE_VALUE; 
}

void for_each_file(const _TCHAR *pDir, for_each_file_func function, void *pData) 
{
	WIN32_FIND_DATA fd;
	_TCHAR Dir[MAX_PATH+1];
	_TCHAR Search[MAX_PATH+1];
	_TCHAR Down[MAX_PATH+1];
	HANDLE hFind;
	// fileinfo fi;
	BOOL bNoEnd;

	// printf("(%s\n", pDir);

	_tcscpy_s(Dir, MAX_PATH, pDir);
	PathAddBackslash(Dir);
	_tcscpy_s(Search, MAX_PATH, Dir);
	_tcscat_s(Search, MAX_PATH, _T("*"));

	hFind = FindFirstFile(
		Search, 
		&fd);

	if(hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if(!(_tcscmp(fd.cFileName, _T(".")) == 0 || 
		   _tcscmp(fd.cFileName, _T("..")) == 0)) {
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				_tcscpy_s(Down, MAX_PATH, Dir);
				_tcscat_s(Down, MAX_PATH, fd.cFileName);
				// findfiles(Down, files);
				for_each_file(Down, function, pData);
			}
			else {
				FindFile ff;
				_tcscpy_s(ff.cFileName, MAX_PATH, Dir);
				_tcscat_s(ff.cFileName, MAX_PATH, fd.cFileName);
				ff.size.LowPart = fd.nFileSizeLow;
				ff.size.HighPart = fd.nFileSizeHigh;
				// fprintf(stderr, "%" I64 " %s\n", ff.size, ff.cFileName);
				function(&ff, pData);
			}
		}

		bNoEnd = FindNextFile(hFind, &fd);

	} while(bNoEnd);

	FindClose(hFind);

	// printf(")%s\n", pDir);
}

FileHandle GetStdOutputHandle() {
	FileHandle f;
	f.hFile = GetStdHandle(STD_OUTPUT_HANDLE);
	return f;
}

bool WriteString(FileHandle *f, const _TCHAR *buffer)
{
	size_t nLength = _tcslen(buffer)*sizeof(_TCHAR);
	BOOL bResult;
	DWORD nWritten;

#ifdef _UNICODE
	/* output to console */
	/* console does not support unicode strings */
	if(GetFileType(f->hFile) == FILE_TYPE_CHAR) { 
		size_t length = _tcslen(buffer)+1;
		char *buffer2 = new char[length];
		
		WideCharToMultiByte(
			CP_THREAD_ACP, 
			0, 
			buffer, 
			-1, 
			buffer2, 
			(unsigned int)length, 
			NULL,
			NULL);
		
		bResult = WriteFile(f->hFile, 
			buffer2, 
			(unsigned int)strlen(buffer2), 
			&nWritten, NULL);

		goto End;
	}
#endif
		
		

	bResult = WriteFile(
		f->hFile, 
		buffer, 
		(unsigned int)nLength, 
		&nWritten, 
		NULL);
End:
	return bResult && nWritten == nLength;
}

#endif /* defined(_WIN32) */

