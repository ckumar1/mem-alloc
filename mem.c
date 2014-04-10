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
node_t* head;  // pointer to the head of the free list

/* Helper Functions */

// TODO maybe needs to be fixed so 4096 doesn't allocate a new page?
//  Should have enough space for our data structures on top of requested data
size_t align(size_t size, size_t alignment)
{
	// Round up the requested size to the next highest multiple of alignment

	// Note: if partialSpace==0, function still rounds up to the next page by adding 4096
	size_t partialSpace = size % alignment;
	size_t addedSpace = alignment - partialSpace;

	size_t alignedSize = size + addedSpace;
	return alignedSize;
}


/*
 * Splits freeBlock into a smaller freeBlock and a new block 
 * of size newBlockSize
 * returns void* to the allocated block of newBlockSize
 */
void* split_free_block(node_t* freeBlock, size_t newBlockSize)
{

	// Store freeBlock fields in local vars
	size_t oldBlockSize = freeBlock->size;
	node_t * fbNext = freeBlock->next;
	node_t * fbPrev = freeBlock->prev;

	// TODO: refactor allocHeader to hold the address of allocated space not allocHeader
	header_t* allocHeader = (void*) freeBlock;
	// Initialize allocated block size to their 8bit aligned request
	allocHeader->size = newBlockSize - sizeof(header_t);

	// Set trimmedFreeBlock by adding  + the size of an allocated header
	node_t* trimmedFreeBlock = (void*) freeBlock + sizeof(header_t) + newBlockSize;
	// Initialize trimmedFreeBlock
	// Calculate the new reduced size of trimmedFreeBlock
	trimmedFreeBlock->size = oldBlockSize - newBlockSize; // TODO: refactor to requestedSize
	// Keep the same pointers
	trimmedFreeBlock->next = fbNext;
	trimmedFreeBlock->prev = fbPrev;



	// Connect the new free node back to free list

	if (fbNext) {  // N
		// Set next nodes previous pointer to the new node
		fbNext->prev = trimmedFreeBlock;
	}

	if (fbPrev) {  // P
		// Set previous node's next ptr to the newly split node's address
		// FIXME throwing a segfault
		fbPrev->next = trimmedFreeBlock;

	} else { // P'
		// no previous node means freeBlock was the head ptr
		// so head must be updated to the new address of the trimmed node
		head = trimmedFreeBlock;
	}

	return allocHeader;
}

// Search for free space
/* findBestfitChunk(size_t)
 * 
 * Use the Bestfit strategy to find the min(|n| n->size > size) 
 * Return the best node on success
 * Returns NULL on failure (no node is big enough to alloc size)
 */
void * findBestfit(size_t requestedSize)
{

	// Store a pointer to the head of the list
	node_t * freeNode = head;
	node_t * bestfit = NULL;

	for (; freeNode != NULL; freeNode = freeNode->next) {

		if (bestfit) {	// if bestfit is set
			// node greater than requested size and smaller (better) than current bestfit
			if (freeNode->size >= requestedSize && (freeNode->size < bestfit->size))
				bestfit = freeNode;
		} else if (freeNode->size >= requestedSize)  // if (bestfit==NULL)
			// initialize bestfit with the first node that can fit the requestedSize
			bestfit = freeNode;

	}  // end_FOR

	// if bestfit is found: return pointer to start of the free block
	// else: return NULL
	return (void*) bestfit;

}

/* bestfitChunk(size_t size)
 * Searches for a free chunk in the free list that can fit size
 * returns a pointer to the allocated space if found;
 * returns NULL on failure
 */
void * bestfitChunk(size_t totalSize)
{

	// Search for an available block in free list
	void * bestfitBlock = findBestfit(totalSize);

	if (bestfitBlock) {  // CASE:  Available node found!

		// Splits free block and returns void ptr to allocated block (TODO +header) of size totalSize
		header_t* bestfitHeader = split_free_block(bestfitBlock, totalSize);

		// Set bestfitBlock to point to the start of allocated memory
		bestfitBlock = (void *) (bestfitHeader + sizeof(header_t));


	}

	// return void* to allocated mem if found,
	// otherwise NULL if no Node is big enough
	return (bestfitBlock);
}

/*
 *  Check if there is enough free space to satisfy requested size
 * if found, sets header and splits free chunk
 * On Success, returns void* ptr to the start of the requested spaced
 * On Failure, returns NULL
 */
void * bestfitFor(size_t size)
{

	void* ptr = bestfitChunk(size);

	if (ptr == NULL) {
		m_error = E_NO_SPACE;
	}
	return (ptr);
}

int Mem_Init(int sizeOfRegion)
{


	// check for invalid args and attempts to run multiple times
	if (m_init_flag != 0 || sizeOfRegion <= 0) {
		m_error = E_BAD_ARGS;
		return (-1);
	}

	// set init flag to prevent Mem_Init from being run again
	m_init_flag = 1;

	// Make sure there is enough memory for the free list
	maxHeapSize = sizeOfRegion;		// + sizeof(node_t);


	// Align the requested heap size to the nearest page size
	size_t alignedSize = align(maxHeapSize, getpagesize());

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
	// return 0 on success
	return (0);
}

void *Mem_Alloc(int size)
{

	// Total allocated size is requestedSize + sizeof(header)
	size_t allocSize = size + sizeof(header_t);
	// Align size to 8 byte chunks
	size_t alignedAllocSize = align(allocSize, 8);

	return bestfitFor(alignedAllocSize);
}

int Mem_Free(void *ptr)
{
	// TODO finish implementing mem_free

	// Calculate ptr to Header of allocated block
	header_t* alloc_header = (void *) ptr - sizeof(header_t);

	// size of allocated space being freed (not counting the header)
	size_t freed_size = alloc_header->size;

	// Convert allocated block into a free block
	node_t *freed_blk = (void *) alloc_header;
	freed_blk->size = freed_size + sizeof(header_t);
	freed_blk->next = NULL;
	freed_blk->prev = NULL;

	// Reassign pointers to add freed_blk back to the free list
	freed_blk->next = head;  // Link freed block to Head of free list
	head->prev = freed_blk;  // Link Head of Free list to freed_blk
	head = freed_blk;  // Set the Head of the free list to the newly freed block

	// FIXME: coalesce!
	node_t * freelistIterator = head;
	while (freelistIterator->next != NULL) {
		//adding size to the pointer address to get next block
		node_t *possibleNextFreeBlock = (node_t *) (freelistIterator + freelistIterator->size);
		// Check if next block is neighboring block
		if (possibleNextFreeBlock == freelistIterator->next) {
//			freelistIterator->size += possibleNextFreeBlock->size;
//			freelistIterator->next = possibleNextFreeBlock->next;
		}

		// if available, check if prev block can be coalesced
		if (freelistIterator->prev) {
			// subtracting size to the pointer address to get previous block
			node_t *possiblePrevFreeBlock = (node_t *) (freelistIterator
			        - freelistIterator->prev->size);
			// Check if prev block is neighboring block
			if (possiblePrevFreeBlock == freelistIterator->prev) {
//				freelistIterator->size += possiblePrevFreeBlock->size;
//				freelistIterator->prev = possiblePrevFreeBlock->prev;
			}
		}

		// move onto next node
		freelistIterator = freelistIterator->next;
	}

	return (0);
}

void Mem_Dump()
{
	// print out the entire free list

	node_t* nextFlNode = head;

	while (nextFlNode != NULL) {
		printf("Node at: %p\n\tPrevious: %p\n\tNext: %p\n\tSize: %zu\n", nextFlNode,
		        nextFlNode->prev, nextFlNode->next, nextFlNode->size);
		nextFlNode = nextFlNode->next;
	}
}
