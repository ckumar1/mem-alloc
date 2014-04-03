#include "mem.h"
#include <stdio.h>
#include <unistd.h>

int main() {
	printf( "%i\n", getpagesize() );
	return (0);
}