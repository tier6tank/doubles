#
#   dbl - search duplicate files
#   Copyright (C) 2007-2008 Matthias Böhm
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#


# this makefile isn't very nice, and propably only works on my computer

debug = 1
unicode = 1

minopt = -W4 -EHsc -Wp64
opt = -Yu"stdinc.h" $(minopt)
!if $(debug)==1
dbg = -Zi -D_DEBUG -DDEBUG -MTd -GS -RTCs -RTCu -RTCc
!else
dbg = -Ox -DNDEBUG
!endif
!if $(unicode)==1
unicode = -D_UNICODE -DUNICODE
!else
unicode = 
!endif

opt = $(opt) $(dbg) $(unicode)
minopt = $(minopt) $(dbg) $(unicode)
libs = shlwapi.lib
nostdincheaders = dbl.h largeint.h ulargeint.h os_cc_specific.h os_cc_specific_includes.h stdinc.h
headers = $(nostdincheaders) stdinc.pch

#all: dbl.exe dblunix.exe dblbl.exe

all: dbl.exe dblunix.exe dblmingw.exe dblmingw_nouni.exe

dbl.exe:  dbl.obj os_cc_specific.obj 
	cl $(opt) -Fe"dbl.exe" dbl.obj os_cc_specific.obj $(libs)

dbl.obj: dbl.cpp $(headers)
	cl $(opt) -c dbl.cpp

os_cc_specific.obj: os_cc_specific.cpp $(headers)
	cl $(opt) -c os_cc_specific.cpp

stdinc.pch: stdinc.cpp $(nostdincheaders)
	cl -c $(minopt) -Yc"stdinc.h" stdinc.cpp

dblunix.exe dblmingw.exe dblmingw_nouni.exe: dbl.cpp os_cc_specific.cpp $(headers)
	C:\cygwin\bin\bash --login -i /cygdrive/c/programs/cpp/dbl_cvs/dbl/buildunix.sh

dblbl.exe: dbl.cpp os_cc_specific.cpp $(headers)
	bcc32 -e"dblbl.exe" -WU dbl.cpp os_cc_specific.cpp PSDK\shlwapi.lib