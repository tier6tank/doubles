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

#ifndef _FILE_H_123
#define _FILE_H_123

#define BASEBUFSIZE 512

class File
{
public:
	File();
	File(const wxString &);
	~File();

	File(const File &);
	File & operator = (const File &);

	void SetName(const wxString & _name) { data->name = _name; }
	wxString GetName() const { return data->name; }

	// bool IsOk();

	// void Close();

	bool Read(char *, int &);
	bool Seek(const wxULongLong &);

	static int GetBufSize() { return File::BUFSIZE; }

	bool Open();
	void Close();

private:
	struct extfiledata {
		unsigned long nFirstBytes;
		unsigned long nMaxFirstBytes;
		char* firstbytes;
		wxFile file;
	};

	struct filedata
	{
		int rcount;
		extfiledata *extdata;
		wxString name;
	};


	void ReleaseData();
	void ReleaseExtData();

	void init();

	static const int MAXFIRSTBYTES;
	static const int BUFSIZE;

	filedata *data;

};

#endif
























