CFLAGS+=-std=c11
CFLAGS+=-g
all: http httpserver

http: http.o base64.o
	cc ${CFLAGS} -o http http.o base64.o

httpserver: httpserver.o
	cc ${CFLAGS} -o httpserver httpserver.o

http.o: http.c
	cc ${CFLAGS} -c http.c

httpserver.o: httpserver.c
	cc ${CFLAGS} -c httpserver.c

base64.o: base64.c
	cc ${CFLAGS} -c base64.c

clean:
	rm -f http httpserver *.o