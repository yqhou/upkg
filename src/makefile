CC = gcc
INCDIR=../inc
BINDIR=../bin
LIBPATH=d:\program\lib
OBJDIR=../obj
IINC=-I${INCDIR}
LLIB=-L${LIBPATH} 
LIBFLAG=-lupkg

ifeq ($(OS),Windows_NT)
SUFFIX=.dll
CFLAGS = -g -c -DWIN
else
SUFFIX=.so
CFLAGS= -g -c -DUNIX
endif

all: utiltest

.c.o:
	${CC} ${CFLAGS} $*.c ${IINC}
	

utiltest:test.o
	${CC} -g $^ -o ${BINDIR}/$@ ${LLIB} ${LIBFLAG}
	rm -f $^

upkg:main.o #util.o list.o upkg.o pkgfile.o pkg.o
	${CC} -g $^ -o ${BINDIR}/$@ ${LLIB} ${LIBFLAG}
	rm -f $^
 
 

pkg:pkgMain.o #util.o list.o upkg.o pkgfile.o pkg.o
	${CC} -g $^ -o ${BINDIR}/$@ ${LLIB} ${LIBFLAG}
	rm -f $^
    
Sep2Fixed:fconv.o util.o 
	${CC} -g $^ -o ${BINDIR}/$@ 
	rm -f $^
           
libupkg: util.o list.o upkg.o pkgfile.o pkg.o
	${CC} -g -shared $^ -o ${LIBPATH}/$@$(SUFFIX)
	rm -f $^
	
clean:
	rm -f *.o