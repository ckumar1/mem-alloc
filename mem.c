#include "mem.h"
#include <stdio.h>
#include <unistd.h>


int m_error;

static int m_init_flag = 0;

int Mem_Init(int sizeOfRegion) 
{
	puts("Mem_Init starts...\n");
	printf("Requested Size: %d\n", sizeOfRegion);

	// check for errors
	if ( m_init_flag != 0 || sizeOfRegion <= 0)
	{
		puts("Bad arguments for Mem_Init!\n\nEither already been run once or sizeOfRegion was not above zero");
		// set m_error for bad args
		m_error = E_BAD_ARGS;
		return (-1);
	}

	// set init flag to prevent Mem_Init from being run again
	m_init_flag = 1;

	
	// Round up the requested size of region to the nearest page size
	int pageSize = getpagesize();
	int partialSpace = sizeOfRegion % pageSize;
	int addedSpace = pageSize  - partialSpace;
	int roundedSize = sizeOfRegion + addedSpace;

	printf("Rounded Size: %d\n", roundedSize );	
	
	
	
	puts("Mem_Init Ending.");

	// return 0 on success
	return (0);
}

void *Mem_Alloc(int size)
{

	return (0);
}

int Mem_Free(void *ptr) {

	return (0);
}

void Mem_Dump() {

}
