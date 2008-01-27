
THE PROGRAM: 
------------

The goal of the program is shortly described:
It is a command-line program which searches for duplicate files in the given path(s). 

INSTALLATION:
-------------

Windows: simply copy the executable (dupf.exe) and the cygwin1.dll or mingwm10.dll file (if present) 
         for example to C:\windows or somewhere else in your path, so that you can call it from the 
         commandline from every directory. Alternatively, you can leave out that step and call the program
         from the commandline in only the directory in which you extracted the files. 

Unix:    copy the executable (dupf) to /usr/local/bin (if that is not in your path, choose /bin), 
         so that you can call it from every directory

USAGE:
-------

From the commandline, simply type dupf <path1> <path2> <...> to search the dirtectories and subdirectories for duplicates. 
The output can be redirected to a file. 


THE FAR, FAR, FUTURE:
---------------------
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

1) You have to have the wxWidgets library installed. 
   To get there, you have to download wxWidgets from 
   http://sourceforge.net/projects/wxwindows

a) In Unix configure the whole thing with options as you like 
   (but --disable-shared is required, you could but also change 
    the makefiles to be able to link with shared libraries), 
   for example
   configure --with-gtk --disable-shared [--enable-debug]

b) In Windows, follow the instructions you find in the docs directory


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

