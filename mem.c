#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>

int m_error;

static int m_init_flag = 0;


typedef struct page_t { 
	void* start;
	int size; 
	void* next;
	void* prev;
	bool used;
} page_t;

page_t* head;

int alignPage(int sizeOfRegion) {
	// Round up the requested size of region to the nearest page size
	int pageSize = getpagesize();
	int partialSpace = sizeOfRegion % pageSize;
	int addedSpace = pageSize - partialSpace;
	int roundedSize = sizeOfRegion + addedSpace;
	return roundedSize;
}

int Mem_Init(int sizeOfRegion) 
{
	// TODO make sure there is enough memory for the free list and other data structs


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
	int roundedSize = alignPage(sizeOfRegion);
	printf("Rounded Size: %d\n", roundedSize );	
	
	// open the /dev/zero device
	int fd = open("/dev/zero", O_RDWR);
	
	// roundedSize (in bytes) is evenly divisible by the page size
	head = mmap(NULL, roundedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	
	if (head == MAP_FAILED) { perror("mmap"); exit(1); }
	// close the device (don't worry, mapping should be unaffected)
	close(fd);


	// Initialize fields of head free list
	head->size = roundedSize;
	head->start = &head; // could be incorrect and need to be removed
	head->next = NULL;
	head->prev = NULL;
	head->used = false;

	printf("Free Space: %i\n", head->size);
	
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
