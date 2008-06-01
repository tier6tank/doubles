
bin_PROGRAMS = dupf dupfgui
dupf_SOURCES = dupf.cpp dbl.cpp os_cc_specific.cpp file.cpp
dupf_CXXFLAGS = `wx-config --static --debug --cxxflags`
dupf_LDADD = @WX_LIBS@
#dupf_DEPENDENCIES = stdinc.h.gch
dupfgui_CXXFLAGS = `wx-config --static --debug --cxxflags`
dupfgui_SOURCES = dupfgui.cpp dbl.cpp os_cc_specific.cpp dialog1.cpp dialog2.cpp dialog3.cpp file.cpp
dupfgui_LDADD = @WX_LIBS@
#dupfgui_DEPENDENCIES = stdinc.h.gch

#stdinc.h.gch: stdinc.cpp stdinc.h
#	`wx-config --cxx` `wx-config --cxxflags` -g -c -Wall -x c++ stdinc.h