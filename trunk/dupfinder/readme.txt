
THE PROGRAM: 
------------

The goal of the program is shortly described:
It is a command-line program which searches for duplicate files in the given path(s). 

INSTALLATION:
-------------

Windows: simply copy the executable (dbl.exe) and the cygwin1.dll file (if there is that file) 
         for example to C:\windows, so that you can call it from the commandline from every directory

Unix:    copy the executable (dbl) to /bin, so that you can call it from every directory

USAGE:
-------

From the commandline, simply type dbl <path1> <path2> <...> to search the dirtectories and subdirectories for duplicates. 
The output can be redirected to a file. 
Please avoid such things like "dbl /path1 /path1" or "dbl /path1/path2 /path1". If you specify directly or 
indirectly directories twice or more on the commandline, the files in these directories are compared with 
themselves. Up to now, I did'nt fix this problem, but it will probably later be fixed. 

TODO:
-----
- Create a nice (os- and make-utitlity-independent?! if possible) makefile
- Adding license text as header in source files
+ converting all integers to longlong (__i64) integers, where overflow could occur (e.g. the number of double files) done. 
+ unicode support for windows... done!
- (delete OpenFile, ReadFile, CloseFile and replace them by fopen, fread, fclose)
- adding option -max (max size of files to compare with each other) or -maxbytes (compare max. n bytes of a file)
- adding IsSameFileFunction
- adding error messages (when finding files, comparing files...)
+ supporting files over 4 GB always?! yes, i do hope so. 
- adding option -f <filename> for output to file? (because unicode chars aren't displayed in windows consoles)
  unicode output
- adding option which determines if the small sized files or the large sized files are treated and displayed firstd
- adding option for not going in subdirectories
- adding unicode support for linux/unix
- removing clock_t from source, replacing it with time_t
- (correct cygwin-build lf/cr errors)
- merging searching files and sorting files by size algorithms ?
+ somehow fixing the problem with the sprintf_s routines, which are currently supported only by microsoft v14 (or deleting
  them?? but they are to nice to delete them, so something other, but what!?!?, has to be done...) done! with v-printf-routines. 
  Nevertheless a call to sscaf_s (if occuring) with an "%s" format argument would very propably leed into trouble, so i must not
  use this!
- playing a bit with some compiling options (especcially optimization for gcc!)

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


LICENSE:
-------
See license.txt

