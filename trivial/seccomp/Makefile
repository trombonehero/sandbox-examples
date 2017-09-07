CFLAGS=		-Wall -I ../include
LDFLAGS=	-L ../libexcite

all: excite

excite: excite.o ../libexcite/libexcite.so
	${CC} ${LDFLAGS} excite.o -l excite -o excite

../libexcite/libexcite.so: ../libexcite/libexcite.c
	${MAKE} -C ../libexcite

clean:
	rm -rf excite *.o
