
make -f makefile.unix shared=0 debug=0 unicode=0 $1
make -f makefile.unix shared=0 debug=0 unicode=1 $1
make -f makefile.unix shared=0 debug=1 unicode=0 $1
make -f makefile.unix shared=0 debug=1 unicode=1 $1

make -f makefile.unix shared=1 debug=0 unicode=0 $1
make -f makefile.unix shared=1 debug=0 unicode=1 $1
make -f makefile.unix shared=1 debug=1 unicode=0 $1
make -f makefile.unix shared=1 debug=1 unicode=1 $1

