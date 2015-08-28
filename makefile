CC = gcc
CFLAGS = -g -c
INCDIR=./inc
BINDIR=.
LIBPATH=c:\Users\Yanqing\AppData\Local\Code\bin
OBJDIR=./obj
IINC=-I${INCDIR}
LLIB=-L${LIBPATH} 
LIBFLAG=-lupkg

all: utiltest

.c.o:
	${CC} ${CFLAGS} $*.c ${IINC}
	

utiltest:test.o
	${CC} -g $^ -o ${BINDIR}/$@ ${LLIB} ${LIBFLAG}
	mv $^ ${OBJDIR}

libupkg: util.o list.o upkg.o
	${CC} -g -shared $^ -o ${LIBPATH}/$@.dll
	${CC} -g -shared $^ -o ${LIBPATH}/$@.so
	mv $^ ${OBJDIR}
	
clean:
	rm -f *.o