all: mtest mem.so mem.o

mem.o: mem.c
	gcc -c -fpic mem.c -Wall -Werror

mem.so: mem.o
	gcc -shared -o libmem.so mem.o

mtest: mem.so mem_test.c
	gcc -lmem -L. -o mtest mem_test.c -Wall -Werror

clean:
	rm -f mtest mem.so mem.o
