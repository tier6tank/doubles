
# this makefile isn't very nice, and propably only works on my computer

debug = 1
unicode = 1

minopt = -W4 -EHsc -Wp64
opt = -Yu"stdinc.h" $(minopt)
!if $(debug)==1
dbg = -Zi -D_DEBUG -DDEBUG -MTd -GS -RTCs -RTCu -RTCc
!else
dbg = -Ox -DNDEBUG
!endif
!if $(unicode)==1
unicode = -D_UNICODE -DUNICODE
!else
unicode = 
!endif

opt = $(opt) $(dbg) $(unicode)
minopt = $(minopt) $(dbg) $(unicode)
libs = shlwapi.lib
nostdincheaders = dbl.h largeint.h ulargeint.h os_cc_specific.h os_cc_specific_includes.h stdinc.h
headers = $(nostdincheaders) stdinc.pch

#all: dbl.exe dblunix.exe dblbl.exe

all: dbl.exe dblunix.exe dblmingw.exe

dbl.exe:  dbl.obj os_cc_specific.obj 
	cl $(opt) -Fe"dbl.exe" dbl.obj os_cc_specific.obj $(libs)

dbl.obj: dbl.cpp $(headers)
	cl $(opt) -c dbl.cpp

os_cc_specific.obj: os_cc_specific.cpp $(headers)
	cl $(opt) -c os_cc_specific.cpp

stdinc.pch: stdinc.cpp $(nostdincheaders)
	cl -c $(minopt) -Yc"stdinc.h" stdinc.cpp

dblunix.exe dblmingw: dbl.cpp os_cc_specific.cpp $(headers)
	C:\cygwin\bin\bash --login -i /cygdrive/c/programs/cpp/dbl_cvs/dbl/buildunix.sh

dblbl.exe: dbl.cpp os_cc_specific.cpp $(headers)
	bcc32 -e"dblbl.exe" -WU dbl.cpp os_cc_specific.cpp PSDK\shlwapi.lib