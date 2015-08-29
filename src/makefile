CC = gcc
INCDIR=../inc
BINDIR=../bin
LIBPATH=c:\Users\Yanqing\AppData\Local\Code\bin
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

libupkg: util.o list.o upkg.o
	${CC} -g -shared $^ -o ${LIBPATH}/$@$(SUFFIX)
	rm -f $^
	
clean:
	rm -f *.o