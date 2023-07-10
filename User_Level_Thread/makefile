# Makefile for library

t=4

all: my_scheduler.o threads.o libult.a libs project2

my_scheduler.o: threads.h my_scheduler.c
	gcc -c -std=gnu99 -Wall -Wextra -Wno-unused-parameter threads.h my_scheduler.c -lrt

threads.o: threads.c threads.h my_scheduler.c
	gcc -c -std=gnu99 -Wall -Wextra -Wno-unused-parameter threads.c threads.h my_scheduler.c -lrt

libult.a: threads.o my_scheduler.o
	ar rcs libult.a threads.o my_scheduler.o

libs: libult.a

project2: libult.a TestFile1.c
	gcc -g -std=gnu99 -o project2 TestFile1.c -L . -lult -lrt
clean:
	rm -rf *.o libult libult.a project2 *.gch

