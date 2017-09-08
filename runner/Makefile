CFLAGS=	-g -Weverything -Wno-vla -std=c11

all: run

run: run.o
	${CC} run.o -o run

clean:
	rm -rf *.o run
