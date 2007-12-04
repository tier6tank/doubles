
#include "stdinc.h"
using namespace std;

#include "os_cc_specific.h"

#if !defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1400)

// sprintf_s, scanf_s, ... aren't defined
// so wrap them into the old functions

/* that works ???????????????, because there aren't any variables declared, 
   so bp and sp don't change */

/* implement perhaps own versions of *_s functions ... */

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) ((x) = (x))
#endif

int sprintf_s(char *buffer, int buflen, const char *format, ... ) {
	UNREFERENCED_PARAMETER(buflen);
	return sprintf(buffer, format);
}

int sscanf_s(char *buffer, const char *format, ... ) {
	return sscanf(buffer, format);
}

char* strcpy_s(char *a, int nLength, const char *b) {
	UNREFERENCED_PARAMETER(nLength);
	return strcpy(a, b);
}

char *strcat_s(char *a, int nLength, const char *b) {
	UNREFERENCED_PARAMETER(nLength);
	return strcat(a, b);
}

int fopen_s(FILE **ppf, const char *filename, const char *mode) {
	*ppf = fopen(filename, mode);
	return (*ppf != NULL);
}

#endif /* !(defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1400) */

/****************************************************************/

#if !defined(_WIN32)

bool OpenFile(FileHandle *f, const char *name) {
	return fopen_s(&f->f, name, "rb");
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

void	for_each_file(const char *pRootDir, for_each_file_func function, void *pData)
{
	const char * array[2] = { pRootDir, NULL };
	FTS *hFind;
	FTSENT *fe;
	FindFile ff;

	errno = 0;
	hFind = fts_open((char *const*)array, FTS_PHYSICAL, NULL);

	if(errno != 0) {
		return;
	}

	do {
		fe = fts_read(hFind);

		if(fe != NULL) {
			if(fe->fts_info == FTS_F) {
				strcpy_s(ff.cFileName, MAX_PATH, fe->fts_path);
				ff.size.QuadPart = fe->fts_statp->st_size;
				// ff.bDirectory = fe->fts_info == FTS_D;
				function(&ff, pData);
			}
		}

	} while(fe != NULL);

	fts_close(hFind);
}


#else /* defined(_WIN32) */

bool OpenFile(FileHandle *f, const char *name) {
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
		
bool ReadFile(const FileHandle *f, char *buffer, DWORD nLength, DWORD *pRead) {
	BOOL bRetVal;
	bRetVal = ReadFile(f->hFile, buffer, nLength, pRead, NULL);
	
	return bRetVal == TRUE;
}

bool	SeekFile(const FileHandle *f, const ULARGE_INTEGER *pto) {
	ULARGE_INTEGER tmp;
	tmp = *pto;
	tmp.LowPart = SetFilePointer(f->hFile, tmp.LowPart, (PLONG)&tmp.HighPart, FILE_BEGIN);
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

void for_each_file(const char *pDir, for_each_file_func function, void *pData) 
{
	WIN32_FIND_DATA fd;
	char Dir[MAX_PATH+1];
	char Search[MAX_PATH+1];
	char Down[MAX_PATH+1];
	HANDLE hFind;
	// fileinfo fi;
	BOOL bNoEnd;

	// printf("(%s\n", pDir);

	strcpy_s(Dir, MAX_PATH, pDir);
	PathAddBackslash(Dir);
	strcpy_s(Search, MAX_PATH, Dir);
	strcat_s(Search, MAX_PATH, "*");

	hFind = FindFirstFile(
		Search, 
		&fd);

	if(hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if(!(strcmp(fd.cFileName, ".") == 0 || 
		   strcmp(fd.cFileName, "..") == 0)) {
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				strcpy_s(Down, MAX_PATH, Dir);
				strcat_s(Down, MAX_PATH, fd.cFileName);
				// findfiles(Down, files);
				for_each_file(Down, function, pData);
			}
			else {
				FindFile ff;
				strcpy_s(ff.cFileName, MAX_PATH, Dir);
				strcat_s(ff.cFileName, MAX_PATH, fd.cFileName);
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

#endif /* defined(_WIN32) */

