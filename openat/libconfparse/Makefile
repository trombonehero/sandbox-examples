CFLAGS=	-g -Wall -std=c99 -fPIC -I ../include

all: libconfparse.so

libconfparse.o: ../include/libconfparse.h

libconfparse.so: libconfparse.o
	${CC} -shared ${CFLAGS} libconfparse.o -o libconfparse.so

clean:
	rm -f *.o *.so
