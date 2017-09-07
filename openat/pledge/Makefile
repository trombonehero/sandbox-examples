CFLAGS=		-Wall -I ../include
LDFLAGS=	-L ../libconfparse

all: do_stuff

do_stuff: do_stuff.o ../libconfparse/libconfparse.so
	${CC} ${LDFLAGS} do_stuff.o -l confparse -o do_stuff

clean:
	rm -rf do_stuff *.o
