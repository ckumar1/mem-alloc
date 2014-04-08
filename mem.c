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
size_t align(size_t size, size_t alignment) {
	// Round up the requested size to the next highest multiple of alignment

	// Note: if partialSpace==0, function still rounds up to the next page by adding 4096
	size_t partialSpace = size % alignment;
	size_t addedSpace = alignment - partialSpace;

	size_t alignedSize = size + addedSpace;
	return alignedSize;
}

/* initHeader(header_t *, size_t)
 *
 * initialize header fields
 * returns the size of header
 */
void * initHeader(header_t * header, size_t allocSize) {
	// Initialize Header fields
	header->size = allocSize;
	return sizeof(header_t);
}

// TODO Search for free space
void * findBestfitChunk(size_t size) {
	void * bfChunk = NULL;

	return bfChunk;
}

/* bestfitChunk(size_t size)
 *
 * Searches for a free chunk in the free list that can fit size
 * returns address to requested space on success
 * returns NULL on failure
 */
void * bestfitChunk(size_t size) {
	void * ptr = findBestfitChunk(size);// returns a pointer to the head of the split free chunk if found; else NULL
	if (ptr)
		ptr += initHeader(ptr, size);// ptr should now point to the beginning of the allocated memory
	return ptr;
}

/*
 *  Check if there is enough free space to satisfy requested size
 * if found, sets header and splits free chunk
 * On Success, returns void* ptr to the start of the requested spaced
 * On Failure, returns NULL
 */
void * bestfitFor(size_t size) {

	void* ptr = bestfitChunk(size);

	// Set m_error inside bestfitFor
	if (ptr == NULL) {
		m_error = E_NO_SPACE;
	}
	return (ptr);
}

int Mem_Init(int sizeOfRegion) {

	puts("Mem_Init starts...\n");
	printf("Requested Size: %d\n", sizeOfRegion);

	// check for invalid args and attempts to run multiple times
	if (m_init_flag != 0 || sizeOfRegion <= 0) {
		m_error = E_BAD_ARGS;
		return (-1);
	}

	// set init flag to prevent Mem_Init from being run again
	m_init_flag = 1;

	// Make sure there is enough memory for the free list
	size_t heapSize = sizeOfRegion + sizeof(node_t);

	printf("Free list Node Size: %u\n", (unsigned int) sizeof(node_t)); // TEST output

	// Align the requested heap size to the nearest page size
	size_t alignedSize = align(heapSize, getpagesize());
	printf("Aligned Size: %u\n", (unsigned int) alignedSize); // TEST output

	// open the /dev/zero device
	int fd = open("/dev/zero", O_RDWR);

	// roundedSize (in bytes) is evenly divisible by the page size
	head = mmap(NULL, alignedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

	if (head == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	// close the device (don't worry, mapping should be unaffected)
	close(fd);

	// Initialize fields of head free list
	head->size = alignedSize;
	head->next = NULL;

	printf("Free Space: %i\n", head->size); // TEST output

	puts("Mem_Init Ending."); //TEST output

	// return 0 on success
	return (0);
}

void *Mem_Alloc(int size) {

	// Total allocated size is requestedSize + sizeof(header)
	size_t allocSize = size + sizeof(header_t);
	// Align size to 8 byte chunks
	size_t alignedAllocSize = align(allocSize, 8);

	return bestfitFor(alignedAllocSize);
}

int Mem_Free(void *ptr) {

	return (0);
}

void Mem_Dump() {

}
