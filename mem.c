#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>


/* Global Variables */

int m_error;

static int m_init_flag = 0;

// Node of the free list
typedef struct node_t {
	int size;
	void* next;
} node_t;

// Header for an allocated block
typedef struct header_t {
	int size; 
} header_t;

node_t* head; // pointer to the head of the free list


/* Helper Functions */

// TODO maybe needs to be fixed so 4096 doesn't allocate a new page?
//  Should have enough space for our data structures on top of requested data
int align(size_t size, unsigned int alignment ) {
	// Round up the requested size to the next highest multiple of alignment

	// Note: if partialSpace==0, function still rounds up to the next page by adding 4096
    int partialSpace = size % alignment;
	int addedSpace = alignment - partialSpace;

	int alignedSize = size + addedSpace;
	return alignedSize;
}

int Mem_Init(int sizeOfRegion) 
{


	puts("Mem_Init starts...\n");
	printf("Requested Size: %d\n", sizeOfRegion);

	// check for errors
	if ( m_init_flag != 0 || sizeOfRegion <= 0)
	{
		// TEST output
		puts("Bad arguments for Mem_Init!\n\n\
		Either already been run once or sizeOfRegion was not above zero");

		m_error = E_BAD_ARGS;
		return (-1);
	}

	// set init flag to prevent Mem_Init from being run again
	m_init_flag = 1;

	// Make sure there is enough memory for the free list
	size_t heapSize = sizeOfRegion + sizeof (node_t);
	
	printf("Free list Node Size: %u\n", (unsigned int) sizeof(node_t)); // TEST output

	// Align the requested heap size to the nearest page size
	size_t alignedSize = align(heapSize, getpagesize());
	printf("Aligned Size: %u\n", (unsigned int) alignedSize ); // TEST output
	


	// open the /dev/zero device
	int fd = open("/dev/zero", O_RDWR);
	
	// roundedSize (in bytes) is evenly divisible by the page size
	head = mmap(NULL, alignedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	
	if (head == MAP_FAILED) { perror("mmap"); exit(1); }
	// close the device (don't worry, mapping should be unaffected)
	close(fd);


	// Initialize fields of head free list
	head->size = alignedSize; // TODO need to take into account size of header
	head->next = NULL;

	printf("Free Space: %i\n", head->size); // TEST output
	
	puts("Mem_Init Ending."); //TEST output

	// return 0 on success
	return (0);
}

void *Mem_Alloc(int size)
{
	// TODO Align size to 8 byte chunks
	// TODO Check if bestfit() can find space for requested size
	// TODO On success, returns void* ptr to the start of the requested space 
	// 		(&alloc_block + sizeof(header))
	void* ptr = head; 
	return (ptr);
}

int Mem_Free(void *ptr) {

	return (0);
}

void Mem_Dump() {

}
