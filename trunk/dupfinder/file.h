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

#ifndef _FILE_H_123
#define _FILE_H_123

#define BASEBUFSIZE 512


// File class
// important: this class is always assuming *sequential* read!!!
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


	// Read - replaces buffer with static buffer if needed
	// else writes into given buffer
	bool Read(char **, unsigned int &);
	bool Restart();

	static int GetBufSize() { return File::BUFSIZE; }

	// need to know the size of the file (cached)
	bool Open(const wxULongLong &);
	void Close();

private:
	struct extfiledata {
		char *cache;
		unsigned int maxcachesize;
		unsigned int cachesize;
		wxFile file;
		wxFileOffset pos;
		wxULongLong size;
		bool bChangeToDiskRead;
	};

	struct filedata
	{
		int rcount;
		extfiledata *extdata;
		wxString name;
	};


	void ReleaseData();
	void ReleaseExtData();

	unsigned int RoundUpToBufSize(unsigned int);

	void init();

	static const unsigned int MAXCACHESIZE;
	static const unsigned int BUFSIZE;

	filedata *data;

};

#endif
























