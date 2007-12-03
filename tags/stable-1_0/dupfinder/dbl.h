
#ifndef __DBL_H_123
#define __DBL_H_123

#define BASEBUFSIZE 512

struct fileinfo
{
	char name[MAX_PATH];
	ULARGE_INTEGER size;
	int nFirstBytes;
	int nMaxFirstBytes;
	char *firstbytes;
	FileHandle fh;
};

struct findfileinfo
{
	ULARGE_INTEGER nMaxFileSizeIgnore;
	list<fileinfo> *pFiles;
};

struct fileinfoequal
{
	list<fileinfo> files;
};


struct fileinfosize
{
	ULARGE_INTEGER size;
	list<fileinfo> files;
	list<fileinfoequal> equalfiles;
};

#endif /* defined(__DBL_H_123) */

