CFLAGS=		-g -Wall -std=c99 -I ../include
LDFLAGS=	-L ../libconfparse

all: do_stuff

do_stuff: do_stuff.o ../libconfparse/libconfparse.so
	${CC} ${LDFLAGS} do_stuff.o -l confparse -l bsd -o do_stuff

../libconfparse/libconfparse.so: ../libconfparse/libconfparse.c
	${MAKE} -C ../libconfparse

run: do_stuff
	@rm -f p0wn*
	ktrace -di ./do_stuff ../conf `mktemp -d scratch.XXXXXX` \
		|| kdump | grep 'operation requires'

clean:
	rm -rf do_stuff ktrace.out *.o p0wn* scratch.*
