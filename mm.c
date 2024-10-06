/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include <stdint.h>

#include "mm.h"



/* Proposed data structure elements */

typedef struct header {
  struct header * next;     // Bit 0 is used to indicate free block 
  uint64_t user_block[0];   // Standard trick: Empty array to make sure start of user block is aligned
} BlockHeader;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
#define GET_NEXT(p)    (void *) ((uintptr_t) (p->next) & ~0x1)    /* (DONE - Marcus): Mask out free flag */
#define GET_FREE(p)    (uint8_t) ( (uintptr_t) (p->next) & 0x1 )   /* OK -- do not change */
#define SET_NEXT(p,n)  p->next = (void *) ((uintptr_t) n + GET_FREE(p))  /* (DONE - Marcus): Preserve free flag */
#define SET_FREE(p,f)  p->next = (void *) ((uintptr_t) GET_NEXT(p) | (f)) /* (DONE - Marcus): Set free bit of p->next to f */
#define SIZE(p)        (size_t) ( 0 ) /* TODO: Caluculate size of block from p and p->next */ 

#define MIN_SIZE     (8)   // A block should have at least 8 bytes available for the user


static BlockHeader * first = NULL;
static BlockHeader * current = NULL;

/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 *
 */
void simple_init() {
    /* TODO: Alignment */
    uintptr_t start_diff = memory_start & 0x7;
    uintptr_t aligned_memory_start;
  if(start_diff != 0) {
     aligned_memory_start = memory_start + (8 - start_diff);
  } else {
      aligned_memory_start = memory_start;
  }
  uintptr_t aligned_memory_end   = memory_end & ~0x8;
  BlockHeader * last;

  /* Already initalized ? */
  if (first == NULL) {
    /* Check that we have room for at least one free block and an end header */
    if (aligned_memory_start + 2*sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
      /* (DONE - Marcus): Place first and last blocks and set links and free flags properly */
      // Placing the first block on first address of aligned memory
      first = (BlockHeader *) aligned_memory_start;

      // Placing the last (dummy) block (with user space 0 bytes) on the last 8 bytes of aligned memory space
      last = (BlockHeader *) aligned_memory_end - sizeof(BlockHeader);

      // Setting the free flag of the first (free) and last block (allocated)
      SET_FREE(first,1);
      SET_FREE(last,0);

      /*
       * Setting the next pointer of the first and last block.
       * First blocks points to the last block, and the last block
       * points to the first block, creating a circular linked list.
       * First block will have user_block of size = (aligned memory - 2*sizeof(BlockHeader))
       */
      SET_NEXT(first, last);
      SET_NEXT(last, first);
    }
    current = first;     
  } 
}


/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory and return a pointer to the first byte.
 *
 * This function should behave similar to a normal malloc implementation. 
 *
 * @param size_t size Number of bytes to allocate.
 * @retval Pointer to the start of the allocated memory or NULL if not possible.
 *
 */
void* simple_malloc(size_t size) {
  
  if (first == NULL) {
    simple_init();
    if (first == NULL) return NULL;
  }

    /* (DONE - Marcus): Alignment */
  size_t aligned_size;
  if(size % 8 != 0) {
    aligned_size = size + (8 - (size % 8));
  } else {
    aligned_size = size;
  }

  /* Search for a free block */
  BlockHeader * search_start = current;
  do {
 
    if (GET_FREE(current)) {

      /* Possibly coalesce consecutive free blocks here */

      /* Check if free block is large enough */
      if (SIZE(current) >= aligned_size) {
        /* Will the remainder be large enough for a new block? */
        if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
          /* (DONE - Marcus): Use block as is, marking it non-free*/
          //current->next = (void *) ((uintptr_t) current->next | 0x1);
            SET_FREE(current, 0);
        } else {
            // Create new block at the end of the allocated user_block
            BlockHeader * new_block = (BlockHeader *) ((uintptr_t) current + aligned_size);
            // Insert new block into the linked list
            new_block->next = current->next;
            current->next = new_block;
            // Set the free flag of current block to 0
            SET_FREE(current, 0);
          /* (DONE - Marcus): Carve aligned_size from block and allocate new free block for the rest */
        }
        void * user_block = (void *) current->user_block;
        current = GET_NEXT(current);
        return (void *) user_block; /* (DONE - Marcus): Return address of current's user_block and advance current */
      }
    }

    current = GET_NEXT(current);
  } while (current != search_start);

 /* None found */
  return NULL;
}


/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 *
 * This function should behave similar to a normal free implementation. 
 *
 * @param void *ptr Pointer to the memory to free.
 *
 */
void simple_free(void * ptr) {
    // Check if ptr is not 8-byte aligned or if ptr is within the memory area
  if(!((uintptr_t) ptr & 0x7)
  || ((uintptr_t) ptr > memory_end || (uintptr_t) ptr <= memory_start)){
    return;
  }

  BlockHeader * block = ptr - 8; /*(DONE - Marcus): Find block corresponding to ptr */
  if (GET_FREE(block)) {
    /* Block is not in use -- probably an error */
    return;
  }
  /* (DONE - Marcus): Free block */
  SET_FREE(block, 0);
  /* Possibly coalesce consecutive free blocks here */
  if(GET_FREE(block->next)){
    // Coalesce
    BlockHeader * next = GET_NEXT(block);
    block->next = next->next;
  }
}


