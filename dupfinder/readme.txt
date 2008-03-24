
THE PROGRAM: 
------------

The program searches for duplicate files. Because it does not use any hashing algorithm, 
it is much faster than other programs. There are both a gui and command line version available. 


INSTALLATION:
-------------

On windows, simpy use the installation program. 
On unix, there is currently no installation program, only a still experimental 
version of a configure system. See "Compiling" below. 

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

Supported are on windows these compilers:
Borland c++, Microsoft c++, Mingw, Cygwin

On unix, use either the provided makefile (makefile.unix, recommended), 
or the configure script (still experimental!). 

Step 1)

  Download the wxWidgets library (if you don't have it installed yet). 

  --- Unix specific: 

  Configure the library with the following command line and then build and install it:

      ./configure --with-gtk --disable-shared [--enable-debug] 
      make 
      make install # as root

  Decide yourself if you want to create a debug version. 

  --- Windows specific:

  In the docs directory you find information about how to install
  the library with your preferred compiler. 

Step 2)

  There are a few options for the makefiles which determine 
  the program build: they are "debug" and "unicode" (windows 
  only). For all options, the valid values are
  0 for off and 1 for on.
  "debug" determines whether to build a debug version or a 
  retail version. Enabled by default. 
  "unicode" determines whether to build a unicode version of 
  the program (windows only). That option mainly exists for
  supporting the old (95, 98, ME) windows versions, which do
  not support unicode. It is enabled by default. 

  --- Windows specific:

  For each compiler, there is a special makefile: 

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
  (note that you have to follow the steps for unix in step 1 first, using --with-msw instead
  of --with-gtk):

	make -f makefile.unix cygwin=1 [debug=0/1]

  The compiled files are stored under unix[d]. 

  --- Unix specific:

  Method 1: For building the program in unix with gcc, type the following:

  	make -f makefile.unix [debug=0/1]

  The compiled files are stored under unix[d].


  Method 2: Take the configure way (but note that this is still experimental, 
  and produces non-optimized executables with debug info, hence the stripping
  below):

	./configure
	make
	strip -S dupfgui
	strip -S dupf
 

LICENSE:
-------
See license.txt

