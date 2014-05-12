#include "brk.h"
#include <unistd.h>
#include <string.h> 
#include <errno.h> 
#include <sys/mman.h>

#define NALLOC 1024                                     /* minimum #units to request */

/* test körning gcc -o tstmalloc -g -Wall -DSTRATEGY=1 malloc.c tstmalloc.c 
./tstmalloc 
gcc -c -o malloc.o malloc.c
make
./RUN_ALLTESTS
chmod +x RUN_TESTS*/

typedef long Align;                                     /* for alignment to long boundary */

union header {                                          /* block header */
	struct {
		union header *ptr;                                  /* next block if on free list */
		unsigned size;                                      /* size of this block  - what unit? */ 
	} s;
	Align x;                                              /* force alignment of blocks */
};

typedef union header Header;

static Header base;                                     /* empty list to get started */
static Header *freep = NULL;                            /* start of free list */

/* Function declarations */
void * realloc(void *ptr, size_t size);
void free(void * ap);
void * endHeap(void);
static Header *morecore(unsigned nu);
void * malloc(size_t nbytes);
void * firstFit(size_t nbytes);
void * bestFit(size_t nbytes);


/*  TODO: SPECIAL FALL 3st*/
void * realloc(void *ptr, size_t size){
	void *newPtr;
	Header *oldHPtr;
	Header *newHPtr;	
	
	if(size == 0){
		if(NULL != ptr){
			free(ptr);
		}
		/* a new minimu m size object is allocated and the orignal object freed */
		return malloc(1);/* borde vara align och ge minsta storleken.*/ /*TODO prata med robert*/
	}

	if(NULL == ptr){		
		return malloc(size);
	}		

	/*Allocate a new block */
	newPtr = malloc(size);
	if(NULL == newPtr){
	/* Malloc failed thus realloc fails aswell*/
		return NULL;
	}

	/* Fetch the old Header*/
	oldHPtr = (Header *) ptr -1; /* Typecast void* to Header* */
	newHPtr = (Header *) newPtr -1; /* Typecast void* to Header* */

	if((oldHPtr->s.size)>(newHPtr->s.size)){ /* if we reduce the block size, copy data as big as the new block*/
		/* Copy data from old to new, size of the block times size of Align*/
		memcpy(newPtr, ptr, newHPtr->s.size*sizeof(Align));
	}else {
		/* Copy data from old to new, size of the block times size of Align*/
		memcpy(newPtr, ptr, oldHPtr->s.size*sizeof(Align));
	}
	
	free(ptr); /* Free the old block*/
	
	return newPtr; /* Return new Pointer */
}


/* free: put block ap in the free list */

void free(void * ap)
{
	Header *bp, *p;
	if(ap == NULL){ 
		return;                                /* Nothing to do */
	}

	bp = (Header *) ap - 1;                               /* point to block header */
	for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr){
		if(p >= p->s.ptr && (bp > p || bp < p->s.ptr)){
			break;                          
		}                  /* freed block at atrt or end of arena */
	}
	if(bp + bp->s.size == p->s.ptr) {                     /* join to upper nb */
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	}
	else{
		bp->s.ptr = p->s.ptr;
	}
	if(p + p->s.size == bp) {                             /* join to lower nbr */
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	} else{
		p->s.ptr = bp;
	}
	freep = p;
}

/* morecore: ask system for more memory */
/* getpagesize, returns number of bytes in a page */

#ifdef MMAP

static void * __endHeap = 0;

void * endHeap(void)
{
	if(__endHeap == 0) { __endHeap = sbrk(0); }
	return __endHeap;
}
#endif


static Header *morecore(unsigned nu)
{
	void *cp;
	Header *up;
#ifdef MMAP
	unsigned noPages;
	if(__endHeap == 0) { __endHeap = sbrk(0); }
#endif

	if(nu < NALLOC) {
		nu = NALLOC;
	}
#ifdef MMAP
	noPages = ((nu*sizeof(Header))-1)/getpagesize() + 1;
	cp = mmap(__endHeap, noPages*getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	nu = (noPages*getpagesize())/sizeof(Header);
	__endHeap += noPages*getpagesize();
#else
	cp = sbrk(nu*sizeof(Header));
#endif
	if(cp == (void *) -1){                                 /* no space at all */
	/* perror("failed to get more memory"); */
	return NULL;
	}
	up = (Header *) cp;
	up->s.size = nu;
	free((void *)(up+1));
	return freep;
}

/*
Malloc function allocating bytes.
@Param: size_t (16 byte unsigned int), representing how many bytes to be allocated by malloc().
@return: Null if when nbytes is 0 or if morecore fails.
@return: void * to the allocated chunk. 
*/
void * malloc(size_t nbytes) {
	return bestFit(nbytes);
}

void * firstFit(size_t nbytes){
	Header *p;	 	/*Header pointer to the new chunk */
	Header *prevp; 	 	/*Header pointer to the previous chunk */
	Header * morecore(unsigned); 	 	/* Create a new header */
	unsigned nunits; 	 	/* Size of Header + the chunk in bytes*/

	if(nbytes == 0) { return NULL; } 	 	/* if nothing is to be allocated, according to malloc def */

	nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) +1; 	 /* Ceiling since we want to round upwards, chunk size */

	/* Intialize */
	if((prevp = freep) == NULL) { 	 	/* If there exists no freep */
		base.s.ptr = freep = prevp = &base; 	 	/* Set all to start of the list */
		base.s.size = 0; 	 	/* intalize size to  0*/
	}

	/* Naive next fit */
	for(p= prevp->s.ptr;  ; prevp = p, p = p->s.ptr) { 	 	/* Iterate over the list */ /* TODO: 2dut to 2dut */
		if(p->s.size >= nunits) {                           /* loop until big enough */
			if (p->s.size == nunits){                          /* Fits perfectly */
				prevp->s.ptr = p->s.ptr; 	 	/* Point the new Header*/
			}
			else {                                            /* allocate tail end, split chunk */
				p->s.size -= nunits; 	 	/* Remove the diffrence */
				p += p->s.size; 	 		/* Add next size nunits to current header */
				p->s.size = nunits; 	 	/* Set next size to nunits */
			}
			/* Ta bort denna rad så får du en first fit nästan*/
			freep =  base.s.ptr; /*	blir en next-fit nästan Start from the new posistion instead from the beginning */
			return (void *)(p+1); 	 	/* Return the new pointer to data chunk*/
		}
		if(p == freep) {                                      /* wrapped around free list */
				if((p = morecore(nunits)) == NULL){ 	 	/* if morecore returns null */				
					return NULL;                                    /* none free space left */
				}
		}
	}
}

void * bestFit(size_t nbytes){
	Header *p;	 	/*Header pointer to the new chunk */
	Header *prevp; 	 	/*Header pointer to the previous chunk */
	Header * morecore(unsigned); 	 	/* Create a new header */
	unsigned nunits; 	 	/* Size of Header + the chunk in bytes*/

	if(nbytes == 0) { return NULL; } 	 	/* if nothing is to be allocated, according to malloc def */

	nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) +1; 	 /* Ceiling since we want to round upwards, chunk size */

	/* Intialize */
	if((prevp = freep) == NULL) { 	 	/* If there exists no freep */
		base.s.ptr = freep = prevp = &base; 	 	/* Set all to start of the list */
		base.s.size = 0; 	 	/* intalize size to  0*/
	}

	Header *bestp = NULL;
	for(p= prevp->s.ptr;  ; prevp = p, p = p->s.ptr) { 	 	/* Iterate over the list */ /* TODO: 2dut to 2dut */
		if(p->s.size >= nunits) {                           /* loop until big enough */
			if (p->s.size == nunits){                          /* Fits perfectly */
				prevp->s.ptr = p->s.ptr; 	 	/* Point the new Header*/
				freep =  base.s.ptr; /*	blir en next-fit nästan Start from the new posistion instead from the beginning */
				return (void *)(p+1); 	 	/* Return the new pointer to data chunk*/
			}
			if(NULL == bestp){ 
				bestp = p;
			}else{
				if(p->s.size < bestp->s.size){
					bestp = p;
				}
			}
		}
		if(p == freep) {                                      /* wrapped around free list */
			if(NULL == bestp){				
				if((p = morecore(nunits)) == NULL){ 	 	/* if morecore returns null */				
					return NULL;                                    /* none free space left */
				}
			} else{
				bestp->s.size -= nunits; 	 	/* Remove the diffrence */
				bestp += bestp->s.size; 	 		/* Add next size nunits to current header */
				bestp->s.size = nunits; 	 	/* Set next size to nunits */
				freep =  base.s.ptr; /*	blir en next-fit nästan Start from the new posistion instead from the beginning */
				return (void *)(bestp+1); 	 	/* Return the new pointer to data chunk*/				
			}
		}
	}
}
