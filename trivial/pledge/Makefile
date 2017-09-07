CFLAGS=		-g -Wall -I ../include
LDFLAGS=	-L ../libexcite

all: excite

excite: excite.o ../libexcite/libexcite.so
	${CC} ${LDFLAGS} excite.o -l excite -o excite

clean:
	rm -rf excite *.o
