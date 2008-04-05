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


#ifndef __OS_CC_SPECIFIC_H_123
#define __OS_CC_SPECIFIC_H_123

#if !defined(_WIN32) || ( defined(_WIN32) && defined(__GNUC__) && !defined(__MINGW32__) )

#if defined(_UNICODE) || defined(UNICODE)

#ifndef _T
#define _T(a) L ## a
#endif

#define _TCHAR wchar_t
#define _tmain wmain
#define _ftprintf fwprintf
// secure not needed (only with strings)
#define _stscanf_s swscanf 

#else // !defined(UNICODE)

#ifndef _T
#define _T(a) a
#endif

#define _TCHAR char
#define _tmain main
#define _ftprintf fprintf
// secure not needed (only with strings)
#define _stscanf_s sscanf

#endif // defined (UNICODE)

#endif /* !defined(_WIN32) */

// MingW/unix  does not support unicode wmain

// the following macro does that:

// loading *_TCHAR* argv over to *wxChar*

// note: no need exceptionally to delete argv, because 1) it is only a small piece
// of memory, and 2) there would be a delete [] argv necessarry
// in front of every return instruction
// perhaps if i have the time, i'll change it...

#if ( (defined(__MINGW32_VERSION) && defined(_WIN32)) || defined(__UNIX__) ) && (defined(_UNICODE) || defined(UNICODE) )

#define DECLARE_MAIN int main(int argc, char *_argv[]) { \
	/* xxx yyy;  uncomment this for testing */ \
	_argv++; \
	argc--; \
	 \
	wxString *argv = new wxString[argc]; \
	{ int i; \
		for(i = 0; i < argc; i++) { \
			argv[i] = wxString(_argv[i], wxConvLocal); \
		} \
	}
#else
#define DECLARE_MAIN int _tmain(int argc, _TCHAR * _argv[]) { \
	_argv++; \
	argc--; \
	 \
	wxString *argv = new wxString[argc]; \
	{ int i;  \
		for(i = 0; i < argc; i++) { \
			argv[i] = _argv[i]; \
		} \
	}
#endif /* defined(__MINGW32_VERSION) */

struct FileData
{
	wxString name;
	wxULongLong size;
	// more is not needed
};

class wxExtDirTraverser : public wxDirTraverser
{
public:
	virtual wxDirTraverseResult OnExtFile(const FileData &) {
		return wxDIR_CONTINUE;
	}
	
};

template <class T> T & unconst(const T & x) {
	return const_cast<T &>(x);
}

// prototypes

bool IsSymLink(const wxString &);
bool IsSymLinkSupported();
bool CreateSymLink(const wxString &, const wxString &);

// bool IsHardLink(const wxString &);  // currently not needed/ (supported (?))
bool IsHardLinkSupported();
bool CreateHardLink(const wxString &, const wxString &);

void Traverse(const wxString &, const wxString &, int, wxExtDirTraverser &);



#endif



