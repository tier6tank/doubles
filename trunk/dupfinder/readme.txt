
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
- (correct cygwin-build lf/cr errors) related to "adding option -f <filename> "
- merging searching files and sorting files by size algorithms ?
+ somehow fixing the problem with the sprintf_s routines, which are currently supported only by microsoft v14 (or deleting
  them?? but they are to nice to delete them, so something other, but what!?!?, has to be done...) done! with v-printf-routines. 
  Nevertheless a call to sscaf_s (if occuring) with an "%s" format argument would very propably leed into trouble, so i must not
  use this!
- playing a bit with some compiling options (especcially optimization for gcc!)
- internationalization

THE FAR, FAR, FUTURE:
-----------
- instead of a command line version program implementing (via other libraries) an easy to use graphical user inferface

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
the program (under different compilers). Nevertheless i will try somewhen to provide a makefile!
Note that for all command lines the following is true: 
If you want to compile your program with unicode support, specify the following additional options on 
the commandline (this is the same with all compilers except to borland): "-D_UNICODE -DUNICODE" (only for 
borland, you need only to specify -WU). Note that the output of unicode characters on the console is only 
good with the microsoft compiler, all other compilers that support unicode break output immidiately when 
they find an unicode character which is not displayable. Hope that the -f option will come soon. )
With each compiler, there is a note whether unicode is supported or not. 

1. Microsoft compiler (unicode supported):
   I've tested with version 12-14, that's the environment under which i 
   developped the program initially, you have to have the Platform SDK installed, else you will get
   error messages when linking or already when compiling!:

   non-unicode:
     cl.exe -Yu"stdinc.h" -Yc"stdinc.h" <debugoptions> -W4 -EHsc -Fe"dbl.exe" dbl.cpp os_cc_specific.cpp shlwapi.lib
   unicode:
     cl.exe -Yu"stdinc.h" -Yc"stdinc.h" -DUNICODE -D_UNICODE <debugoptions> -W4 -EHsc -Fe"dbl.exe" dbl.cpp os_cc_specific.cpp shlwapi.lib
 
   debug build: replace <debugoptions> by -Zi -D_DEBUG -DDEBUG -MTd -GS -RTCs -RTCu -RTCc. 
   release build: replace <debugoptions> by -Ox -DNDEBUG. 


2. Borland C++ (unicode supported):
   e.g. the free commandline version 5.5. I'm not very familiar with that compiler, so here only the 
   minimal commandline (for unicode support specify the option -WU as well):

   non-unicode:
     bcc32 -e"dbl.exe"  <debugoptions> dbl.cpp os_cc_specific.cpp PSDK\shlwapi.lib
   unicode:
     bcc32 -e"dbl.exe" -WU <debugoptions> dbl.cpp os_cc_specific.cpp PSDK\shlwapi.lib

   debug build: replace <debugoptions> by -DDEBUG -D_DEBUG
   release build: replace <debugoptions> by -DNDEBUG -O2

3. Gnu gcc compiler under linux (unicode not supported):
   That's again only the minimalistic minimal commandline, because i have no idea of what options 
   there are. 

     g++ -o dbl dbl.cpp os_cc_specific.cpp

4. Cygwin build alternative 1 (unicode not supported):
   In windows under cygwin, you can build the program nearly as described in 3. Simply call: 

     g++ -o dbl.exe dbl.cpp os_cc_specific.cpp

   Note that compiled thus, the program does NOT support unicode and uses the unix 
   build-in functions. Every call of an unix function is emulated by cygwin. It is therefore very slow!


5. Cygwin build alternative 2/MingW build, the preferable way (unicode supported):
   If you want to have unicode support in your program, you can build the program with mingw 
   under cygwin, which supports unicode. MingW for sure must be installed. Besides, the executables
   which are created with that commandline, are native windows and do not need the cygwin1.dll. 
   They are therefore much more faster than the cygwin-build described before. 
   Again only a minimalistic commandline: 

   non-unicode:
     g++ -o dbl.exe -mwin32 -mno-cygwin dbl.cpp os_cc_specific.cpp -lshlwapi
   unicode:
     g++ -o dbl.exe -DUNICODE -D_UNICODE -mwin32 -mno-cygwin dbl.cpp os_cc_specific.cpp -lshlwapi

   note that unfortunately, the current implementation of mingw does not allow to specify 
   a unicode commandline, but all the rest of the program is in unicode (there are unfortunately
   also some display problems, but what shall i do? Perhaps a future version of mingw will solve 
   these issues).



LICENSE:
-------
See license.txt

