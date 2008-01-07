/******************************************************************************
    dbl - search duplicate files
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


#ifdef PROFILE
LARGE_INTEGER __frequency;
#endif

#if defined(_WIN32) && defined(PROFILE)

#define INITPROFILE QueryPerformanceFrequency(&__frequency);

#define STARTTIME(a) { LARGE_INTEGER __b1, __b2; QueryPerformanceCounter(&__b1); 

#define STOPTIME(a) QueryPerformanceCounter(&__b2);  (a).QuadPart += __b2.QuadPart - __b1.QuadPart; }

#define SECONDS(a)  (wxULongLong((a).QuadPart).GetValue() /__frequency.QuadPart)


#else /* defined(_WIN32) && defined(PROFILE) */

#define INITPROFILE 

#define STARTTIME(a) {

#define STOPTIME(a) }

#define SECONDS(a) 1

#endif /* defined(_WIN32) && defined(PROFILE) */
