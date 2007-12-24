/******************************************************************************
    dbl - search duplicate files
    Copyright (C) 2007-2008 Matthias B�hm

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
	// wxString name;
	wxULongLong size;
	// ULARGE_INTEGER size;
	unsigned long nFirstBytes;
	unsigned long nMaxFirstBytes;
	char* firstbytes;
	// wxFile file;
	FileHandle fh;
};

struct findfileinfo
{
	wxULongLong nMaxFileSizeIgnore;
	list<fileinfo> *pFiles;
	bool bGoIntoSubDirs;
};

struct fileinfoequal
{
	list<fileinfo> files;
};


struct fileinfosize
{
	wxULongLong size;
	list<fileinfo> files;
	list<fileinfoequal> equalfiles;
};

#endif /* defined(__DBL_H_123) */

