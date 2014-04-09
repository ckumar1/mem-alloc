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
	size_t size;
	struct node_t* next;
	struct node_t* prev;

} node_t;

// Header for an allocated block
typedef struct header_t {
	int size;
} header_t;

size_t maxHeapSize;
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
size_t initHeader(header_t * header, size_t allocSize) {
	// Initialize Header fields
	header->size = allocSize;
	return sizeof(header_t);
}

void split_free_node(size_t requestedSize, node_t* freeSplitNode,
		node_t* bestfit) {
	
	// TODO store bestfit fields in local vars
	// then set freeSplitNode accordingly.
	size_t bfChunkSize = bestfit->size;
	node_t * bfNextNode = bestfit->next;
	node_t * bfPrevNode = bestfit->prev;
	
	// Split node with 8bit alignment if bestfit exists
	// FIXME bestfit is getting corrupted before we are done reading from it
	freeSplitNode = (void*) bestfit + requestedSize;

	// Calculate the new size of the split free node
	freeSplitNode->size = bfChunkSize - requestedSize;
	// set pointers as appropriate
	freeSplitNode->next = bfNextNode;
	freeSplitNode->prev = bfPrevNode;


	if (freeSplitNode->prev) {
		// Set previous node's next ptr to the newly split node's address
		// FIXME throwing a segfault
		freeSplitNode->prev->next = (node_t*) freeSplitNode;
	} else {
		// no previous node means bestfit was the head ptr
		// update head ptr to new split address
		head = (node_t*) freeSplitNode;
	}
}

// Search for free space
/* findBestfitChunk(size_t)
 * 
 * Use the Bestfit strategy to find the min(|n| n->size > size) 
 * Return the best node on success
 * Returns NULL on failure (no node is big enough to alloc size)
 */
void * findBestfitChunk(size_t requestedSize) {

	// Store a pointer to the head of the list
	node_t * freeNode = head;
	node_t * bestfit = NULL;
	node_t* freeSplitNode = NULL;

	for (; freeNode != NULL; freeNode = freeNode->next) {

		if (bestfit) {
			// node greater than requested size and smaller (better) than current bestfit
			if (freeNode->size >= requestedSize
					&& (freeNode->size < bestfit->size))
				bestfit = freeNode;
		} else if (freeNode->size >= requestedSize)
			// first node bigger than size is bestfit initially
			bestfit = freeNode;

	} // end_FOR

	if (bestfit) { // Split node with 8bit alignment if bestfit exists
		// FIXME error from head->prev pointer becoming corrupted
		split_free_node(requestedSize, freeSplitNode, bestfit);
	}
	// return the address of the new block
	return (void*) bestfit;

}

/* bestfitChunk(size_t size)
 * Searches for a free chunk in the free list that can fit size
 * returns a pointer to the allocated space if found;
 * returns NULL on failure
 */
void * bestfitChunk(size_t size) {
	header_t* bestfitHeader = (header_t*) findBestfitChunk(size);
	if (bestfitHeader)
		// init Header returns size of header to convert header addr to the start address of allocated memory
		bestfitHeader = (void *) (bestfitHeader
				+ initHeader(bestfitHeader, size - sizeof(header_t)));

	return (bestfitHeader);
}

/*
 *  Check if there is enough free space to satisfy requested size
 * if found, sets header and splits free chunk
 * On Success, returns void* ptr to the start of the requested spaced
 * On Failure, returns NULL
 */
void * bestfitFor(size_t size) {

	void* ptr = bestfitChunk(size);

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
	maxHeapSize = sizeOfRegion;		// + sizeof(node_t);

	printf("Free list Node Size: %u\n", (unsigned int) sizeof(node_t)); // TEST output

	// Align the requested heap size to the nearest page size
	size_t alignedSize = align(maxHeapSize, getpagesize());
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

	// Initialize fields of head freelist node
	head->size = alignedSize;
	head->next = NULL;
	head->prev = NULL;

	printf("Free Space: %zu\n", head->size); // TEST output

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
	// TODO finish implementing mem_free

	// Calculate ptr to Header of allocated block
	header_t* alloc_header = (void *) ptr - sizeof(header_t);

	// size of allocated space being freed (not counting the header)
	size_t freed_size = alloc_header->size;

	// Convert allocated block into a free block
	node_t *freed_blk = (node_t*) alloc_header;
	freed_blk->size = freed_size + sizeof(header_t);
	freed_blk->next = NULL;
	freed_blk->prev = NULL;

	// Reassign pointers to add freed_blk back to the free list

	// Link freed block to Head of free list
	freed_blk->next = head;
	// Link Head of Free list to freed_blk
	head->prev = freed_blk;
	// Set the Head of the free list to the newly freed block
	head = freed_blk;

	// FIXME: coalesce!
	return (0);
}

void Mem_Dump() {

	// TODO implement mem_dump
}
