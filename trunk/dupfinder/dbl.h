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


#ifndef __DBL_H_123
#define __DBL_H_123

#define BASEBUFSIZE 512

struct fileinfo
{
	_TCHAR name[MAX_PATH];
	ULARGE_INTEGER size;
	unsigned int nFirstBytes;
	unsigned int nMaxFirstBytes;
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

