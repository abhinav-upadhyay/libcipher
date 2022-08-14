all: http

http: http.o
	cc -o http http.o

http.o: http.c
	cc -c http.c

clean:
	rm -f http *.o