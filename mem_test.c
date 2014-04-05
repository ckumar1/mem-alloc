#include "mem.h"
#include <stdio.h>
#include <unistd.h>

int main() {

	printf( "Page size: %i\n", getpagesize() );

	int init_rc;
	init_rc  = Mem_Init(782134);
	printf("Memory Init Return Code: %d \n", init_rc);

	// int init_rc_2;
	// init_rc_2  = Mem_Init(getpagesize());
	// printf("Second Memory Init Return Code: %d", init_rc_2);


	// int init_rc_3;
	// init_rc_3  = Mem_Init(-1);
	// printf("Negative page size Memory Init Return Code: %d \n", init_rc_3);

	// int init_rc_4;
	// init_rc_4  = Mem_Init(0);
	// printf("Zero size Memory Init Return Code: %d", init_rc_4);

	return (0);
}