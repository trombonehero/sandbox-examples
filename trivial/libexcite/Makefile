CFLAGS=	-Weverything -fPIC -I ../include

all: libexcite.so

libexcite.o: ../include/libexcite.h

libexcite.so: libexcite.o
	${CC} -shared ${CFLAGS} libexcite.o -o libexcite.so

clean:
	rm -f *.o *.so
