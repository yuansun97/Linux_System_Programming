/**
 * Malloc
 * CS 241 - Spring 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

/* Define macros */
#define WSIZE       4           // Size of word           -- 4 bytes
#define DSIZE       8           // Size of double         -- 8 bytes
#define CHUNKSIZE   (1 << 12)   // Default extending size -- 4k bytes

#define MAX(x, y)   ( (x) > (y) ? (x) : (y) )
#define MIN(x, y)   ( (x) < (y) ? (x) : (y) )

/* OR mask to pack size and allocated flag */
/* alloc: 1--used; 0--free */
#define PACK(size, alloc)   ( (size) | (alloc) )

/* Read and set a int value at pointer p */
#define GET(p)      ( *(unsigned int *)(p) )
#define SET(p, val) ( *(unsigned int *)(p) = (val) )

/* Get the block size and allocated flag from header pointer p */
#define GET_SIZE(hp) ( GET(hp) & ~0x7 )
#define IS_ALLOC(hp) ( GET(hp) & 0x1 )

/* Get the header and footer of a given block pointer */
#define HEAD(bp)    ( (char *)(bp) - WSIZE )
#define FOOT(bp)    ( (char *)(bp) + GET_SIZE( HEAD(bp) ) - DSIZE )

/* Compute the next and the previous block pointer of a given block pointer */
#define NEXT_BLKP(bp)   ( (char *)(bp) + GET_SIZE( HEAD(bp) ) )
#define PREV_BLKP(bp)   ( (char *)(bp) - GET_SIZE( ((char *)(bp) - DSIZE) ) )

#define TOTAL_CLASS     20

static size_t total_heap = 0;   // Size of memory allocated by sbrk (bytes)
static size_t total_used = 0;   // Size of requested memory by user (bytes)

static void *mem_head = NULL;   // The start of heap memory which shall never change
static void *mem_iter = NULL;   // The iterator for memory blocks
static void *mem_brk = NULL;    // The end of allocated heap memory

typedef struct meta {
    // void *prev;
    void *next;
} meta;   

static void *available_blks[TOTAL_CLASS];

void init_heap();
void *extend_heap(size_t nbytes);
void *merge(void *bp);
void *find_fit(size_t nbytes);
void *assign(void *bp, size_t nbytes);
int size_class_num(size_t nbytes);
void insert_node(void *bp, int nclass);
void delete_node(void *dp, int nclass);


/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    if (num == 0 || size == 0) 
        return NULL;

    size_t request_size = num * size;
    long *bp = (long *) malloc(request_size);
    if (bp == NULL) {
        perror("calloc failed!");
        return NULL;
    }

    /* memset */
    for (size_t i = 0; i < request_size / DSIZE; i++) {
        bp[i] = 0;
    }
    return (void *) bp;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!
    if (total_heap == 0) 
        init_heap();

    if (size == 0) 
        return NULL;

    /* Minimal size is 16 / 24 bytes. Padding the size if not divisable by 8. 
        Adding 2 words for header and footer. */
    size_t alloc_size;
    if (size <= 8) {
        alloc_size = 16;
    } else {
        alloc_size = DSIZE * ( (size + DSIZE + (DSIZE - 1)) / DSIZE );
        // size_t rem = size % DSIZE;
        // alloc_size = (rem == 0) ? size : size + (DSIZE - rem);
        // alloc_size += 2 * WSIZE;
    }

    /* Search for fit block */
    void *fit_bp = find_fit(alloc_size);
    if (fit_bp != NULL) {
        fit_bp = assign(fit_bp, alloc_size);
        mem_iter = fit_bp;
        return fit_bp;
    }

    // esle
    /* No fit block in current heap. Extend the heap. */
    size_t extend_size = MAX(alloc_size, CHUNKSIZE);
    fit_bp = extend_heap(extend_size);
    if (fit_bp != NULL) {
        fit_bp = assign(fit_bp, alloc_size);
        mem_iter = fit_bp;      // Update the heap iterator
    }
    
    return fit_bp;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    size_t size = GET_SIZE( HEAD(ptr) );
    
    SET( HEAD(ptr), PACK(size, 0) );
    SET( FOOT(ptr), PACK(size, 0) );
    
    // if ( ptr == mem_iter) {
    //     mem_iter = merge(ptr);
    // } else {
    //     merge(ptr);
    // }

    if (size > 24) {
        if (ptr == mem_iter) 
            mem_iter = merge(ptr);
        else 
            merge(ptr);
    }
    else {
        ;
    }
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if (total_heap == 0) 
        init_heap();

    if (ptr == NULL)
        return malloc(size);

    size_t old_size = GET_SIZE( HEAD(ptr) );

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    // Do nothing if the size stays the same
    if (size == old_size - DSIZE)
        return ptr;
    
    // Compute the actual size to allocate
    size_t alloc_size;
    if (size <= 8) {
        alloc_size = 16;
    } else {
        alloc_size = DSIZE * ( (size + DSIZE + (DSIZE - 1)) / DSIZE );
    }

    /* Assign the front part of the current block 
        if old_size >= request_size 
    */
    if (old_size >= alloc_size) {
        mem_iter = assign(ptr, alloc_size);
        return mem_iter;
    }

    // --------------------
    /* If the current block is followed by a free block 
        and next_size + old_size >= request_size 
    */
    size_t next_size = GET_SIZE( HEAD( NEXT_BLKP(ptr) ) );
    size_t sum_size = old_size + next_size;
    if ( !IS_ALLOC( HEAD( NEXT_BLKP(ptr) ) ) && sum_size >= alloc_size ) {
        delete_node( NEXT_BLKP(ptr), size_class_num(next_size) );
        // SET( HEAD(ptr), PACK(sum_size, 1) );
        // SET( FOOT(ptr), PACK(sum_size, 1) );

        // SET( HEAD(ptr), PACK(alloc_size, 1) );
        // SET( FOOT(ptr), PACK(alloc_size, 1) );
        // SET( HEAD( NEXT_BLKP(ptr) ), PACK(sum_size - alloc_size, 0) );
        // SET( FOOT( NEXT_BLKP(ptr) ), PACK(sum_size - alloc_size, 0) );
        // insert_node( NEXT_BLKP(ptr), size_class_num(sum_size - alloc_size) );
        
        SET( HEAD(ptr), PACK(sum_size, 0) );
        SET( FOOT(ptr), PACK(sum_size, 0) );
        mem_iter = assign(ptr, alloc_size);
        mem_iter = ptr;
        return mem_iter;
    }
    // --------------------

    // Allocate new heap
    if (total_heap == 0) 
        init_heap();

    /* Search for fit block */
    void *fit_bp = find_fit(alloc_size);
    if (fit_bp != NULL) {
        fit_bp = assign(fit_bp, alloc_size);
        mem_iter = fit_bp;
    }

    else {
        /* No fit block in current heap. Extend the heap. */
        size_t extend_size = MAX(alloc_size, CHUNKSIZE);
        fit_bp = extend_heap(extend_size);
        if (fit_bp != NULL) {
            fit_bp = assign(fit_bp, alloc_size);
            mem_iter = fit_bp;      // Update the heap iterator
        }
    }

    /* memmove */
    size_t presrv_size = MIN( size, GET_SIZE( HEAD(ptr) ) - DSIZE );    // exclude the header and footer
    for (size_t i = 0; i < presrv_size; i++) {
        ((char *)fit_bp)[i] = ((char *) ptr)[i];
    }

    free(ptr);

    return (void *) fit_bp;
}


/* -------------------------- Segerated fit --------------------------- */
void init_heap() {
    /* Create initial heap */
    if ( ( mem_head = sbrk(4 * WSIZE) ) == (void *) -1 ) {
        perror("sbrk failed!");
        return;
    }
    mem_iter = mem_head;
    SET( mem_iter, PACK(0, 1) );  
    SET( (mem_iter += WSIZE), PACK(8, 1) );
    SET( (mem_iter += WSIZE), PACK(8, 1) );
    SET( (mem_iter += WSIZE), PACK(0, 1) );
    
    mem_brk = mem_head + 4 * WSIZE;
    mem_head += 2 * WSIZE;

    /* Initialize the free blocks array */
    for (int i = 0; i < TOTAL_CLASS; i++) {
        available_blks[i] = NULL;
    }

    if ( extend_heap(CHUNKSIZE) == NULL )
        return;

    total_used = 0;
}

void *extend_heap(size_t nbytes) {
    if (nbytes == 0) 
        return NULL;
    
    size_t rem = nbytes % 8;
    size_t size = (rem == 0) ? nbytes : nbytes + (8 - rem);

    void *bp = NULL;
    if ( (bp = sbrk(size)) == (void *) -1 ) {
        perror("sbrk failed!");
        return NULL;
    }

    /* Tag the new blok */
    SET( HEAD(bp), PACK(size, 0) );
    SET( FOOT(bp), PACK(size, 0) );
    SET( FOOT(bp) + WSIZE, PACK(0, 1) );    // Epilogue

    mem_brk += size;
    total_heap += size;
    return merge(bp);
}

void *merge(void *bp) {
    size_t prev_allocated = IS_ALLOC( HEAD( PREV_BLKP(bp) ) );
    size_t next_allocated = IS_ALLOC( HEAD( NEXT_BLKP(bp) ) );
    size_t size = GET_SIZE( HEAD(bp) );

    if ( prev_allocated && next_allocated ) {               /* No free neighbors to merge */
        // Update the size class array
        int n = size_class_num(size);
        insert_node(bp, n);
        return bp;
    }

    else if ( prev_allocated && !next_allocated ) {         /* Next block is free */
        void *next_bp = NEXT_BLKP(bp);
        size_t next_size = GET_SIZE( HEAD( next_bp ) );
        size += next_size;

        SET( HEAD(bp), PACK(size, 0) );
        SET( FOOT(bp), PACK(size, 0) );
        // Update the size class array
        int old_class = size_class_num(next_size);
        int new_class = size_class_num(size);
        delete_node(next_bp, old_class);
        insert_node(bp, new_class);
    }

    else if ( !prev_allocated && next_allocated ) {         /* Previous block is free */
        void *prev_bp = PREV_BLKP(bp);
        size_t prev_size = GET_SIZE( HEAD( prev_bp ) );
        size += prev_size;

        SET( HEAD( prev_bp ), PACK(size, 0) );
        SET( FOOT(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
        // Update the size class array
        int old_class = size_class_num(prev_size);
        int new_class = size_class_num(size);
        if (new_class != old_class) {
            delete_node(prev_bp, old_class);
            insert_node(bp, new_class);
        }
    }

    else {                                                  /* Both blocks are free */
        void *next_bp = NEXT_BLKP(bp);
        void *prev_bp = PREV_BLKP(bp);
        size_t next_size = GET_SIZE( HEAD( next_bp ) );
        size_t prev_size = GET_SIZE( HEAD( prev_bp ) );
        size += next_size;
        size += prev_size;
        SET( HEAD( PREV_BLKP(bp) ), PACK(size, 0) );
        SET( FOOT( NEXT_BLKP(bp) ), PACK(size, 0) );
        bp = PREV_BLKP(bp);
        // Update the size class array
        int old_prev_class = size_class_num(prev_size);
        int old_next_class = size_class_num(next_size);
        int new_class = size_class_num(size);
        delete_node( next_bp, old_next_class );
        // if (old_prev_class != new_class) {
            delete_node( prev_bp, old_prev_class );
            insert_node( bp, new_class );
        // }
    }

    return bp;
}

void *find_fit(size_t nbytes) {
    int nclass = size_class_num(nbytes);
    for (int i = nclass; i < TOTAL_CLASS; i++) {
        void *iter = available_blks[i];
        while (iter != NULL) {
            if ( GET_SIZE( HEAD(iter) ) >= nbytes ) {
                return iter;
            }
            iter = ((meta *) iter)->next;
        }
    }
    return NULL;
}

void *assign(void *bp, size_t nbytes) {
    size_t size = GET_SIZE( HEAD(bp) );

    if (size < nbytes) {
        perror("Not enough space for nbytes! -- assign");
        return NULL;
    }

    if ( (size - nbytes) < 16 ) {
        SET( HEAD(bp), PACK(size, 1) );
        SET( FOOT(bp), PACK(size, 1) );
        delete_node( bp, size_class_num(size) );
        total_used += size;
        return bp;
    }

    else {
        int old_class = size_class_num(size);
        int new_class = size_class_num(size - nbytes);
        SET( HEAD(bp), PACK(nbytes, 1) );
        SET( FOOT(bp), PACK(nbytes, 1) );
        delete_node( bp, old_class );
        bp = NEXT_BLKP(bp);
        SET( HEAD(bp), PACK(size - nbytes, 0) );
        SET( FOOT(bp), PACK(size - nbytes, 0) );
        insert_node( bp, new_class );
        total_used += nbytes;
        return PREV_BLKP(bp);
    }
}

int size_class_num(size_t nbytes) {
    int i = 0;
    while (nbytes / 2 > 0 && i < TOTAL_CLASS - 1) {
        nbytes = nbytes / 2;
        i++;
    }
    return i;
}

void insert_node(void *bp, int nclass) {
    if (nclass < 0 || nclass > TOTAL_CLASS - 1)
        return;
    
    meta *bp_meta = (meta *) bp;

    void *head = available_blks[nclass];
    bp_meta->next = head;
    available_blks[nclass] = bp;
}

void delete_node(void *bp, int nclass) {    
    meta *target = (meta *) bp;
    meta *iter = (meta *) available_blks[nclass];
    meta *prev = NULL;

    while (iter != NULL) {
        if (iter == target) {
            if (prev == NULL) {
                available_blks[nclass] = (void *) iter->next;
            } else {
                prev->next = iter->next;
            }
            iter->next = NULL;
        }
        prev = iter;
        iter = iter->next;
    }
}



/* ------------------------ First fit and Next fit ------------------------ */

/** 
 * Initialize the heap by using sbrk.
 * 
 * The initial heap with be a four-word heap,
 *  containing a blank word, two-word header block, one-word end block.
 * 
 * */
// void init_heap() {
    // /* Create initial heap */
    // if ( ( mem_head = sbrk(4 * WSIZE) ) == (void *) -1 ) {
    //     perror("sbrk failed!");
    //     return;
    // }
    // mem_iter = mem_head;
    // SET( mem_iter, PACK(0, 1) );  
    // SET( (mem_iter += WSIZE), PACK(8, 1) );
    // SET( (mem_iter += WSIZE), PACK(8, 1) );
    // SET( (mem_iter += WSIZE), PACK(0, 1) );
    
    // mem_brk = mem_head + 4 * WSIZE;
    // mem_head += 2 * WSIZE;

    // /* Extend the heap by CHUNKSIZE */
    // if ( ( mem_iter = sbrk(CHUNKSIZE) ) == (void *) -1 ) {
    //     perror("sbrk failed!");
    //     return;
    // }
    // SET( HEAD(mem_iter), PACK(CHUNKSIZE, 0) );
    // SET( FOOT(mem_iter), PACK(CHUNKSIZE, 0) );
    // SET( FOOT(mem_iter) + WSIZE, PACK(0, 1) );  // Epilogue

    // mem_brk += CHUNKSIZE;

    // total_heap = CHUNKSIZE;
    // total_used = 0;
// }

/**
 * Extend the heap by nbytes and merge the previous block if it's free.
 * Note that if nbytes is not divisable by 8 there will be padding bytes in the end of block,
 *  rounding it up so that it is divisable by 8.
 * 
 * */
// void *extend_heap(size_t nbytes) {
    // if (nbytes == 0) 
    //     return NULL;
    
    // size_t rem = nbytes % 8;
    // size_t size = (rem == 0) ? nbytes : nbytes + (8 - rem);

    // void *bp = NULL;
    // if ( (bp = sbrk(size)) == (void *) -1 ) {
    //     perror("sbrk failed!");
    //     return NULL;
    // }

    // /* Tag the new blok */
    // SET( HEAD(bp), PACK(size, 0) );
    // SET( FOOT(bp), PACK(size, 0) );
    // SET( FOOT(bp) + WSIZE, PACK(0, 1) );    // Epilogue

    // mem_brk += size;

    // total_heap += size;

    // return merge(bp);
// }

/**
 * Merge the non-allocated neighbor blocks.
 * 
 * */
// void *merge(void *bp) {
    // size_t prev_allocated = IS_ALLOC( HEAD( PREV_BLKP(bp) ) );
    // size_t next_allocated = IS_ALLOC( HEAD( NEXT_BLKP(bp) ) );
    // size_t size = GET_SIZE( HEAD(bp) );

    // if ( prev_allocated && next_allocated ) {               /* No free neighbors to merge */
    //     return bp;
    // }

    // else if ( prev_allocated && !next_allocated ) {         /* Next block if free */
    //     size += GET_SIZE( HEAD( NEXT_BLKP(bp) ) );
    //     SET( HEAD(bp), PACK(size, 0) );
    //     SET( FOOT(bp), PACK(size, 0) );
    // }

    // else if ( !prev_allocated && next_allocated ) {         /* Previous block is free */
    //     size += GET_SIZE( HEAD( PREV_BLKP(bp) ) );
    //     SET( HEAD( PREV_BLKP(bp) ), PACK(size, 0) );
    //     SET( FOOT(bp), PACK(size, 0));
    //     bp = PREV_BLKP(bp);
    // }

    // else {                                                  /* Both blocks are free */
    //     size += GET_SIZE( HEAD( NEXT_BLKP(bp) ) );
    //     size += GET_SIZE( HEAD( PREV_BLKP(bp) ) );
    //     SET( HEAD( PREV_BLKP(bp) ), PACK(size, 0) );
    //     SET( FOOT( NEXT_BLKP(bp) ), PACK(size, 0) );
    //     bp = PREV_BLKP(bp);
    // }

    // return bp;
// }

/**
 * Find a free block for the requested size.
 * 
 * First-fit search.
 * Next-fit search.
 * 
 * Two-way search.
 * 
 * */
// void *find_fit(size_t nbytes) {
    // void *bp;

    // if (nbytes >= (0x1 << 12)) {
    //     /* First-fit search */
    //     for( bp = mem_head; GET_SIZE( HEAD(bp) ) > 0; bp = NEXT_BLKP(bp) ) {
    //         if ( !IS_ALLOC( HEAD(bp) ) && GET_SIZE( HEAD(bp) ) >= nbytes ) {
    //             return bp;
    //         }
    //     } 
    // }
    
    // else {
    //     /* Next-fit search */
    //     for( bp = mem_iter; GET_SIZE( HEAD(bp) ) > 0; bp = NEXT_BLKP(bp) ) {
    //         if ( !IS_ALLOC( HEAD(bp) ) && GET_SIZE( HEAD(bp) ) >= nbytes ) {
    //             return bp;
    //         }
    //     } 
    // }

    // return NULL;
// }

/**
 * Split the block pointed by bp and assign to requested nbytes.
 * 
 * */
// void assign(void *bp, size_t nbytes) {

    // size_t size = GET_SIZE( HEAD(bp) );

    // if (size < nbytes) {
    //     perror("Not enough space for nbytes!");
    //     return;
    // }

    // if ( (size - nbytes) < DSIZE ) {
    //     SET( HEAD(bp), PACK(size, 1) );
    //     SET( FOOT(bp), PACK(size, 1) );
    // }

    // else {
    //     SET( HEAD(bp), PACK(nbytes, 1) );
    //     SET( FOOT(bp), PACK(nbytes, 1) );
    //     bp = NEXT_BLKP(bp);
    //     SET( HEAD(bp), PACK(size - nbytes, 0) );
    //     SET( FOOT(bp), PACK(size - nbytes, 0) );
    // }

    // total_used += nbytes;
// }