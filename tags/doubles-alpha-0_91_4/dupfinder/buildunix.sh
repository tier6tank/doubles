#   dbl - search duplicate files
#   Copyright (C) 2007-2008 Matthias Bhm
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


cd /cygdrive/c/programs/cpp/dbl_cvs/dbl
g++ -o dblunix.exe dbl.cpp os_cc_specific.cpp
g++ -o dblmingw.exe -DUNICODE -D_UNICODE -mwin32 -mno-cygwin dbl.cpp os_cc_specific.cpp -lshlwapi
g++ -o dblmingw_nouni.exe -mwin32 -mno-cygwin dbl.cpp os_cc_specific.cpp -lshlwapi



