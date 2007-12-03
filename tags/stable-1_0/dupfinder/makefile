
# this makefile isn't very nice, and propably only works on my computer

debug = 1

minopt = -W4 -EHsc
opt = -Yu"stdinc.h" $(minopt)
!if $(debug)==1
dbg = -Zi -D_DEBUG -DDEBUG -MTd -GS -RTCs -RTCu -RTCc
!else
dbg = -Ox -DNDEBUG
!endif
opt = $(opt) $(dbg)
minopt = $(minopt) $(dbg)
libs = shlwapi.lib
nostdincheaders = dbl.h largeint.h ulargeint.h os_cc_specific.h os_cc_specific_includes.h stdinc.h
headers = $(nostdincheaders) stdinc.pch

all: dbl.exe #dblunix.exe dblbl.exe

#all: dbl.exe dblunix.exe

dbl.exe:  dbl.obj os_cc_specific.obj 
	cl $(opt) -Fe"dbl.exe" dbl.obj os_cc_specific.obj $(libs)

dbl.obj: dbl.cpp $(headers)
	cl $(opt) -c dbl.cpp

os_cc_specific.obj: os_cc_specific.cpp $(headers)
	cl $(opt) -c os_cc_specific.cpp

#dbl.exe: dbl.cpp stdinc.pch dbl.h os_cc_specific.h profile.h
#	cl $(opt) dbl.cpp $(libs)

dbl0.exe: dbl.cpp stdinc.pch dbl.h os_cc_specific.h
	cl $(opt) -DBENCHMARK -DTESTFUNC=0 -DTESTCNT=1 -Fedbl0.exe dbl.cpp $(libs)

dbl4.exe: dbl.cpp stdinc.pch dbl.h os_cc_specific.h
	cl $(opt) -DBENCHMARK -DBENCHMARKBUFSIZE -DTESTFUNC=4 -Fedbl4.exe dbl.cpp $(libs)

stdinc.pch: stdinc.cpp $(nostdincheaders)
	cl -c $(minopt) -Yc"stdinc.h" stdinc.cpp

dblunix.exe: dbl.cpp dbl.h os_cc_specific.h os_cc_specific_includes.h stdinc.h
	C:\cygwin\bin\bash --login -i /cygdrive/c/programs/cpp/dbl/buildunix.sh

dblbl.exe: dbl.cpp os_cc_specific.cpp $(headers)
	bcc32 -e"dblbl.exe" dbl.cpp os_cc_specific.cpp PSDK\shlwapi.lib