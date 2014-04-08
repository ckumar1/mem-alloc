all: mtest mem.so mem.o

mem.o: mem.c
	gcc -c -g -O0 -fpic mem.c -Wall -Werror

mem.so: mem.o
	gcc -shared -o libmem.so mem.o

mtest: mem.so mem_test.c
	gcc -lmem -L. -g -O0 -o mtest mem_test.c -Wall -Werror

clean:
	rm -f mtest libmem.so mem.o
