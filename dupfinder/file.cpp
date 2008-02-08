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

#include "stdinc.h"
#include "file.h"
#include "dbl.h"

const int File::MAXFIRSTBYTES = BASEBUFSIZE << 7;
const int File::BUFSIZE = BASEBUFSIZE << 7;

File::File(){
	init();
}

File::File(const wxString &_name) {
	init();
	data->name = _name;
}

void File::init() {
	data = new filedata;
	data->extdata = NULL;
	data->rcount = 1;
}

File::~File() {
	ReleaseData();
}

File::File(const File & rhs) {
	data = rhs.data;
	data->rcount++;
}

File & File::operator = (const File &rhs) {
	if(&rhs != this) {
		ReleaseData();
		data = rhs.data;
		data->rcount++;
	}
	return *this;
}

void File::ReleaseData() {
	assert(data);
	assert(data->rcount >= 1);

	data->rcount--;

	if(data->rcount == 0) {
		// delete object

		ReleaseExtData();

		delete data;
	}
}

bool File::Open() {
	bool bResult;

	if(!data->extdata) {
		data->extdata = new extfiledata;

		data->extdata->firstbytes = NULL;
		data->extdata->nFirstBytes = 0;
		data->extdata->nMaxFirstBytes = 0;
		
		bResult = data->extdata->file.Open(data->name);
	}
	else {
		bResult = data->extdata->file.IsOpened();
	}
	return bResult;
}


bool File::Read(char *buffer, int &ncount) {
	if(!Open()) {
		return false;
	}

	assert(ncount == File::BUFSIZE);

	ncount = data->extdata->file.Read(buffer, ncount);

	return ncount != wxInvalidOffset;
}

bool File::Seek(const wxULongLong &pos) {
	if(!Open()) {
		return false;
	}

	wxFileOffset new_pos = data->extdata->file.Seek((wxFileOffset)pos.GetValue());

	return new_pos != wxInvalidOffset;
}

void File::Close() {
	ReleaseExtData();
	init();
}

void File::ReleaseExtData()
{
	if(data->extdata) {
		if(data->extdata->file.IsOpened()) {
			data->extdata->file.Close();
		}
		delete [] data->extdata->firstbytes;
		delete data->extdata;
	}
}






