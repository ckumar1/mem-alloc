#include <assert.h>
#include <stdlib.h>
#include "mem.h"
// Free Test
int main() {
   assert(Mem_Init(4096) == 0);
   void* ptr = Mem_Alloc(8);
   assert(ptr != NULL);
   assert(Mem_Free(ptr) == 0);
   exit(0);
}
