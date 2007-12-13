cd /cygdrive/c/programs/cpp/dbl_cvs/dbl
g++ -o dblunix.exe dbl.cpp os_cc_specific.cpp
g++ -o dblmingw.exe -DUNICODE -D_UNICODE -mwin32 -mno-cygwin dbl.cpp os_cc_specific.cpp -lshlwapi
g++ -o dblmingw_nouni.exe -mwin32 -mno-cygwin dbl.cpp os_cc_specific.cpp -lshlwapi

#g++ -o dblunix0.exe -DBENCHMARK -DTESTFUNC0 dbl.cpp
#g++ -o dblunix4.exe -DBENCHMARK -DTESTFUNC0 -DTESTCNT0 dbl.cpp
