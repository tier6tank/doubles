
THE PROGRAM: 
------------

The goal of the program is shortly described:
It is a command-line program which searches for duplicate files in the given path(s). 


TODO:
-----
Create a nice (os- and make-utitlity-independent?! if possible) makefile
Adding license text as header
converting all integers to longlong (__i64) integers, where overflow could occur (e.g. the number of double files)
unicode support for windows...
delete OpenFile, ReadFile, CloseFile and replace them by fopen, fread, fclose


DESCRIPTION:
------------
description of the scanning process:

1. all files in the given directories and subdirectories are stored in a list, listing twice or more often scanned
   files only once
2. a second list, which contains itself lists of files of equal size is created, for each size an entry which contains
   a list of equal-sized files
3. in that list, the files having same size are compared with each other, to determine whether they are equal or not
   equal files are stored in other lists, for each size one, which at the end of that process are printed to screen



COMPILING:
----------
the makefile is propably no use for anybody, so i provide here some instructions to compile
the program (under different compilers):

1. Microsoft compiler (i've tested with version 12-14, that's the environment under which i 
   developped the program initially, you have to have the Platform SDK installed, else you will get
   error messages when linking or already when compiling!):

   cl.exe -Yu"stdinc.h" -Yc"stdinc.h" <debugoptions> -W4 -EHsc -Fe"dbl.exe" dbl.cpp os_cc_specific.cpp shlwapi.lib

   if you're debugging, replace <debugoptions> by "-Zi -D_DEBUG -DDEBUG -MTd -GS -RTCs -RTCu -RTCc"
   else (release) by "-Ox -DNDEBUG"


2. Borland C++ (e.g. the free commandline version 5.5, here aren't that many options though it is surely not bad to add some, 
   for example for debug/release, but i'm not very familiar with the borland compiler, perhaps i'll add some, perhaps i wont...):
   bcc32 -e"dbl.exe"  <debugoptions> dbl.cpp os_cc_specific.cpp PSDK\shlwapi.lib

   debug build: replace <debugoptions> by -DDEBUG -D_DEBUG
   release build: replace <debugoptions> by -DNDEBUG

3. Cygwin gnu/gnu gcc compiler under linux (as well as before: i'm not that familiar with that compiler, it's just the minimal 
   commandline provided here): 
   g++ -o dbl dbl.cpp os_cc_specific.cpp
