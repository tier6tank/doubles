

#if !defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1400)

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) ((x) = (x))
#endif

int sprintf_s(char *buffer, int buflen, const char *format, ... );
int sscanf_s(char *buffer, const char *format, ... );
char* strcpy_s(char *a, int nLength, const char *b);
char *strcat_s(char *a, int nLength, const char *b);
int fopen_s(FILE **ppf, const char *filename, const char *mode);

#endif /* !(defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1400) */

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
#define I64 "I64i"
#endif

#ifdef __GNUC__
#define I64 "lli"
#endif


union FileHandle {
	HANDLE hFile;
	FILE *f;
}; 

struct FindFile {
	char cFileName[MAX_PATH+1];
	ULARGE_INTEGER size;
 };

typedef void (*for_each_file_func)(const FindFile *, void *);

bool	OpenFile(FileHandle *f, const char *name);
bool 	ReadFile(const FileHandle *f, char * buffer, DWORD nLength, DWORD *pRead);
bool	CloseFile(FileHandle *f);
bool	IsValidFileHandle(const FileHandle *f);
void	InitFileHandle(FileHandle *f);
bool	SeekFile(const FileHandle *f, const ULARGE_INTEGER *pto);
void	for_each_file(const char *pRootDir, for_each_file_func function, void *pData);
















