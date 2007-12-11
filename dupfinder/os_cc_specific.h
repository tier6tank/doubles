
#ifndef _WIN32
/* unix provides no unicode, at least not like the windows platform does */

#define _TCHAR char
#define _T(a) a
#define _tmain main
#define _ftprintf fprintf
#define _tcscmp strcmp
#define _tcscpy strcpy
#define _tprintf printf
#define _stprintf sprintf
#define _tcscat strcat
#define _tfopen fopen
#define _vstprintf vsprintf

#define _stscanf_s sscanf_s
#define _tcscpy_s strcpy_s
#define _stprintf_s sprintf_s
#define _stscanf_s sscanf_s
#define _tcscat_s strcat_s
#define _tfopen_s fopen_s

#endif /* !defined(_WIN32) */

#if !defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1400)

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) ((x) = (x))
#endif

int _stprintf_s(_TCHAR *buffer, int buflen, const _TCHAR *format, ... );
_TCHAR* _tcscpy_s(_TCHAR *a, int nLength, const _TCHAR *b);
_TCHAR *_tcscat_s(_TCHAR *a, int nLength, const _TCHAR *b);
int _tfopen_s(FILE **ppf, const _TCHAR *filename, const _TCHAR *mode);

#ifndef _stscanf_s
#define _stscanf_s _tscanf
#endif

#define sscanf_s sscanf

#endif /* !defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1400) */


// MingW does not support unicode wmain
#if defined(__MINGW32_VERSION) && defined(_WIN32) && defined(_UNICODE)
#define DECLARE_MAIN int main(int argc, char *_argv[]) { \
	wchar_t **argv = new wchar_t *[argc+1]; \
	argv[argc] = NULL; \
	{ char **__ptr; int __i; \
	for(__ptr = _argv, __i = 0; __ptr[0] != NULL; __ptr++, __i++) { \
		int __length = strlen(__ptr[0])+1; \
		argv[__i] = new wchar_t[__length*2]; \
		mbstowcs(argv[__i], _argv[__i], __length*2); \
	} } 
#else
#define DECLARE_MAIN int _tmain(int argc, _TCHAR * argv[]) {
#endif /* defined(__MINGW32_VERSION) */

// todouble is there, because msc < 1300 do not handle typecast from unsigned __int64 to double
#if defined(_MSC_VER) && _MSC_VER < 1300

inline double todouble(unsigned __int64 z) {
	return (double)(signed __int64)z;
}
#else
#if defined(_MSC_VER) || defined(__BORLANDC__)
inline double todouble(unsigned __int64 z) {
	return (double)z;
}
#else
inline double todouble(unsigned long long z) {
	return (double)z;
}
#endif /* defined(_MSC_VER) || defined(__BORLANDC__) */
#endif /* _MSC_VER < 1300 */


/* perhaps better if !defined(_WIN32) && defined(__GNUC__) ???? */
#ifndef _WIN32

typedef unsigned long DWORD, BOOL, *LPDWORD;
typedef unsigned long long ULONGLONG;
typedef DWORD HANDLE;
typedef long LONG;
typedef long long LONGLONG;
#define MAX_PATH 260 //?????????????????????????????????

/** attention: is that valid on all machines (little endian <-> high endian) ??? no! */
typedef union _ULARGE_INTEGER { 
    struct {
        DWORD LowPart; 
        DWORD HighPart; 
    };
    ULONGLONG QuadPart;
} ULARGE_INTEGER; 

typedef union _LARGE_INTEGER { 
    struct {
        DWORD LowPart; 
        LONG HighPart; 
    };
    LONGLONG QuadPart;
} LARGE_INTEGER; 

#endif /* !defined(_WIN32) */

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define I64 _T("I64i")
#endif

#ifdef __GNUC__
#define I64 _T("lli")
#endif


union FileHandle {
	HANDLE hFile;
	FILE *f;
}; 

struct FindFile {
	_TCHAR cFileName[MAX_PATH+1];
	ULARGE_INTEGER size;
 };

typedef void (*for_each_file_func)(const FindFile *, void *);

bool	OpenFile(FileHandle *f, const _TCHAR *name);
bool 	ReadFile(const FileHandle *f, char * buffer, DWORD nLength, DWORD *pRead);
bool	CloseFile(FileHandle *f);
bool	IsValidFileHandle(const FileHandle *f);
void	InitFileHandle(FileHandle *f);
bool	SeekFile(const FileHandle *f, const ULARGE_INTEGER *pto);
void	for_each_file(const _TCHAR *pRootDir, for_each_file_func function, void *pData);
















