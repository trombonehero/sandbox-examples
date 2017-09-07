CFLAGS=		-g -Wall -I ../include
LDFLAGS=	-L ../libconfparse

all: do_stuff

do_stuff: do_stuff.o ../libconfparse/libconfparse.so
	${CC} ${LDFLAGS} do_stuff.o -l confparse -o do_stuff

../libconfparse/libconfparse.so: ../libconfparse/libconfparse.c
	${MAKE} -C ../libconfparse

run: do_stuff
	@rm -f p0wn*
	./do_stuff ../conf `mktemp -d scratch.XXXXXX`

clean:
	rm -rf do_stuff ktrace.out *.o p0wn* scratch.*
