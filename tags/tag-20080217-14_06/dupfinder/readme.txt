
THE PROGRAM: 
------------

The program searches for duplicate files. Because it does not use any hashing algorithm, 
it is much faster than other programs. There are both a gui and command line version available. 


INSTALLATION:
-------------

As currently there is no installation program, you have to extract the files of the program in 
a folder you want. From there, you can either execute the command line version (dupf) in a console
or the gui version (dupfgui) by double clicking at the symbol in the file explorer. 


USAGE:
-------

Commandline version:
====================

Type dupf for getting hints for the command line. 
Here a summary: Simply type dupf <path1> <path2> <...> on the command line, for searching 
in the paths path1 and path2 and ... for duplicates. 
You can specify one or more options additionally. 

Graphical User interface version:
=================================

First add the paths in which you want to search in to the list by clicking on Add, 
and specify options as you want. 
Then click on Go! and wait, until the results page is displayed. 

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


For building the program with mingw, type the following (on the windows 
command line, not on msys or another unix emulation shell!):

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

