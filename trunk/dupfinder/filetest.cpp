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

#include "stdinc.h"
#include "FileTest.h"
#include "dbl.h"

const int FileTest::BUFSIZE = BASEBUFSIZE << 7;

FileTest::FileTest(){
	init();
}

FileTest::FileTest(const wxString &_name) {
	init();
	data->name = _name;
}

void FileTest::init() {
	data = new FileTestdata;
	data->extdata = NULL;
	data->rcount = 1;
}

FileTest::~FileTest() {
	ReleaseData();
}

FileTest::FileTest(const FileTest & rhs) {
	data = rhs.data;
	data->rcount++;
}

FileTest & FileTest::operator = (const FileTest &rhs) {
	if(&rhs != this) {
		ReleaseData();
		data = rhs.data;
		data->rcount++;
	}
	return *this;
}

void FileTest::ReleaseData() {
	assert(data);
	assert(data->rcount >= 1);

	data->rcount--;

	if(data->rcount == 0) {
		// delete object

		ReleaseExtData();

		delete data;
	}
}

bool FileTest::Open() {
	bool bResult;

	if(!data->extdata) {
		data->extdata = new extFileTestdata;

		bResult = data->extdata->FileTest.Open(data->name);
	}
	else {
		bResult = data->extdata->FileTest.IsOpened();
	}
	return bResult;
}


bool FileTest::Read(char *buffer, int &ncount) {
	assert(data->extdata);

	assert(ncount == FileTest::BUFSIZE);

	if(!data->extdata->FileTest.IsOpened()) {
		return false;
	}

	ncount = data->extdata->FileTest.Read(buffer, ncount);

	return ncount != wxInvalidOffset;
}

void FileTest::Close() {
	ReleaseExtData();
	init();
}

void FileTest::ReleaseExtData()
{
	if(data->extdata) {
		if(data->extdata->FileTest.IsOpened()) {
			data->extdata->FileTest.Close();
		}
		delete data->extdata;
	}
}






