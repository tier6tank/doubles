
THE PROGRAM: 
------------

The goal of the program is shortly described:
It is a command-line program which searches for duplicate files in the given path(s). 

INSTALLATION:
-------------

Windows: simply copy the executable (dbl.exe) and the cygwin1.dll file (if present) 
         for example to C:\windows, so that you can call it from the commandline from every directory

Unix:    copy the executable (dbl) to /bin, so that you can call it from every directory -- this is old information 
         due to wxWidgets overhead --

USAGE:
-------

From the commandline, simply type dbl <path1> <path2> <...> to search the dirtectories and subdirectories for duplicates. 
The output can be redirected to a file. 

TODO/IDEAS:
-----
- Create a nice (os- and make-utitlity-independent?! if possible) makefile
+ See wxWidget's makefile-collection, this is very nice, for every compiler a special makefile! 
- putting every compiler's files in a special directory, even different for debug/nodebug unicode/no-unicode build
+ Adding license text as header in source files. done
+ converting all integers to longlong (__i64) integers, where overflow could occur (e.g. the number of double files) done. 
+ unicode support for windows... done!
- (delete OpenFile, ReadFile, CloseFile and replace them by fopen, fread, fclose)
- adding option -max (max size of files to compare with each other) or -maxbytes (compare max. n bytes of a file)
- adding IsSameFileFunction
- adding error messages (when finding files, comparing files...)
+ supporting files over 4 GB always?! yes, i do hope so. 
+ adding option -f <filename> for output to file? (because unicode chars aren't displayed in windows consoles)
  unicode output. done
+ adding option which determines if the small sized files or the large sized files are treated and displayed firstd. done
- adding option for not going in subdirectories
- adding unicode support for linux/unix
+ removing clock_t from source, replacing it with time_t: that wasn't an error, it was my fault!
+ (correct cygwin-build lf/cr errors) related to "adding option -f <filename> " seems not to be possible
- merging searching files and sorting files by size algorithms ?
+ somehow fixing the problem with the sprintf_s routines, which are currently supported only by microsoft v14 (or deleting
  them?? but they are to nice to delete them, so something other, but what!?!?, has to be done...) done! with v-printf-routines. 
  Nevertheless a call to sscaf_s (if occuring) with an "%s" format argument would very propably leed into trouble, so i must not
  use this!
- playing a bit with some compiling options (especcially optimization for gcc!)
- internationalization
+ replacing the f<*> c functions with the PSDK-functions for true unicode support (difficult, because of stdout?...) done
- add to statistcs sum of bytes read
+ need a faster container than list for testing for double scanning of directories. done (using set). 
- linux makefile mkdir if

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
Now the compiling is very simple: for each compiler, there is a special makefile. 
Note that you have to have wxWidgets on your system (in directory C:\wx (for windows), if not, 
edit makefiles (variable wxdir) )

For building the program with visual c++ - compiler, just type the following: 

	nmake -f makefile.vc [debug=0/1] [unicode=0/1]

The compiled files are stored under vc[u][d]. 


For building the program with borland c++, type the following:

	make -f makefile.bcc [-Ddebug=0/1] [-Dunicode=0/1]

The compiled files are stored under bcc[u][d]. 


For building the program with mingw, type the following:

	mingw32-make -f makefile.gcc [debug=0/1] [unicode=0/1]

The compiled files are stored under gcc[u][d]. 

If you don't have mingw but only cygwin, you can build the program that way:

	make cygwin=1 [debug=0/1]

The compiled files are stored under unix[d]>. 

For building the program in unix with gcc, type the following:

	make [debug=0/1]

The compiled files are stored under unix[d]. 

Other compilers are perhaps also supported, but i did not try other compilers yet. 


LICENSE:
-------
See license.txt

