
THE PROGRAM: 
------------

The goal of the program is shortly described:
It is a command-line program which searches for duplicate files in the given path(s). 

INSTALLATION:
-------------

Windows: simply copy the executable (dbl.exe) and the cygwin1.dll or mingwm10.dll file (if present) 
         for example to C:\windows, so that you can call it from the commandline from every directory

Unix:    copy the executable (dbl) to /usr/local/bin (if that is not in your path, choose /bin), 
         so that you can call it from every directory

USAGE:
-------

From the commandline, simply type dbl <path1> <path2> <...> to search the dirtectories and subdirectories for duplicates. 
The output can be redirected to a file. 

TODO/IDEAS:
-----------
+ Create a nice (os- and make-utitlity-independent?! if possible) makefile (done, different makefiles)
+ See wxWidget's makefile-collection, this is very nice, for every compiler a special makefile! 
+ putting every compiler's files in a special directory, even different for debug/nodebug unicode/no-unicode build (done)
+ Adding license text as header in source files. done
+ converting all integers to longlong (__i64) integers, where overflow could occur (e.g. the number of double files) done. 
+ unicode support for windows... done!
+ (delete OpenFile, ReadFile, CloseFile and replace them by fopen, fread, fclose) (old)
- adding option -max (max size of files to compare with each other) or -maxbytes (compare max. n bytes of a file)
+ adding IsSameFileFunction (done -> wxWidgets)
- adding error messages (when finding files, comparing files...)
+ supporting files over 4 GB always?! yes, i do hope so. 
+ adding option -f <filename> for output to file? (because unicode chars aren't displayed in windows consoles)
  unicode output. done
+ adding option which determines if the small sized files or the large sized files are treated and displayed firstd. done
+ adding option for not going in subdirectories (done)
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
- removing the current logging construct (removing wxUSE_STD_IOSTREAM, because it needs a recompile of wxWidgets), 
  perhaps using wxLogNull and my own error reporting routines?
- suppress logging completely
- delete unneccessary unix includes
- perhaps setting error flag if any error on that file is encountered
- add progress display for sorting files by size
- display speed in kbytes/second

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

1) At first you have to compile the wxWidgets library:

a) Unix:

Follow the following instructions: 
download wxWidgets, then run the configure shell script in an newly created directory
(e.g. buildgtk-dbl or buildmotif-dbl) with the following options: 
--with-gtk|--with-motif --disable-shared --enable-std_iostreams [--enable-debug]
here is an example of how to do this:

# this is the path where you unpacked wxWidgets
cd <wx-library-path>
# change this to buildgtkd-dbl for debug, replace gtk by motif etc. 
# if you want to use a different graphical interface
mkdir buildgtk-dbl
# use either --with-gtk or --with-motif as you want, if you want to create a debug
# library, use --enable-debug
../configure --with-gtk|--with-motif --disable-shared --enable-std_iostreams [--enable-debug]
make
su <enter root password>
make install
exit

b) Windows: 

In windows follow the following steps (see also technical note 0021):

Edit your setup.h file (in lib/*_lib/msw*/wx); if it does not exist, 
you have to run make once, but you need not wait until the compilation 
exits, because the setup.h file is created at the beginning (press Ctrl+c).
Choose the right options (UNICODE=[0/1], BUILD=[release|debug]). 

Then you have to edit all the lib/*_lib/msw*/setup.h files before you 
compile the wxWidgets library (again): 
Change in all files the line

#define wxUSE_STD_IOSTREAM 0

to 

#define wxUSE_STD_IOSTREAM 1

Then (re)compile wxWidgets (for some reasons be sure to use the 
option -A with nmake if you use visual c++). 


2) The rest is pretty simple: 

For each compiler, there is a special makefile. 


For building the program with visual c++ - compiler, just type the following: 

	nmake -f makefile.vc [debug=0/1] [unicode=0/1]

The compiled files are stored under vc[u][d]. 


For building the program with borland c++, type the following:

	make -f makefile.bcc [-Ddebug=0/1] [-Dunicode=0/1]

The compiled files are stored under bcc[u][d]. 


For building the program with mingw, type the following:

	mingw32-make -f makefile.gcc [debug=0/1] [unicode=0/1]

The compiled files are stored under gcc[u][d]. 


If you don't have mingw but only cygwin, you can build the program that way
(note that you have to follow the steps for unix in 1a first using --with-msw instead
of --with-gtk or --with-motif):

	make cygwin=1 [debug=0/1]

The compiled files are stored under unix[d]. 


For building the program in unix with gcc, type the following:

	make [debug=0/1]

The compiled files are stored under unix[d]. 

Other compilers are perhaps also supported, but i did not try other compilers yet. 


LICENSE:
-------
See license.txt

