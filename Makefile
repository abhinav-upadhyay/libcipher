CFLAGS+="-std=c11"
all: http

http: http.o base64.o
	cc -o http http.o base64.o

http.o: http.c
	cc -c http.c

base64.o: base64.c
	cc -c base64.c

clean:
	rm -f http *.o