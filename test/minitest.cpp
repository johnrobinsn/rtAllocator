#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rtAllocator.h"

#define NUMALLOCS 1000000

int   sizes[NUMALLOCS];
void* allocs[NUMALLOCS];

void*   blockalloc(long size) {
	return malloc(size);
}
void*   blockrealloc(void* fp, long newsize) {
	return realloc(fp, newsize);
} 
void    blockfree(void* p) {
	free(p);
}

typedef void* (MallocFunc)(size_t size);
typedef void  (FreeFunc)(void* p);

void init() {
  for (int i = 0; i < NUMALLOCS; i++) {
		sizes[i] = (((double)rand()/(double)RAND_MAX) * 128);
	}
}

void test(MallocFunc afunc, FreeFunc ffunc) {

	for (int i = 0; i < 10; i++) {
		// Alloc all
		for (int j = 0; j < NUMALLOCS; j++) {
			allocs[j] = afunc(sizes[j]);
		}
#if 1
		// Free first half
		for (int j = 0; j < NUMALLOCS/2; j++) {
			ffunc(allocs[j]);
		}
		// Alloc first half
		for (int j = 0; j < NUMALLOCS/2; j++) {
			allocs[j] = afunc(sizes[j]);
		}
		// Free second half
		for (int j = NUMALLOCS/2; j < NUMALLOCS; j++) {
			ffunc(allocs[j]);
		}
		// Alloc second half
		for (int j = NUMALLOCS/2; j < NUMALLOCS; j++) {
			allocs[j] = afunc(sizes[j]);
		}
#endif
		// Free all
		for (int j = 0; j < NUMALLOCS; j++) {
			ffunc(allocs[j]);
		}

	}

}

int  main() {

#include <stdint.h>
#if UINTPTR_MAX == 0xffffffff
	/* 32-bit */
	printf("32bit\n");
#elif UINTPTR_MAX == 0xffffffffffffffff
	/* 64-bit */
	printf("64bit\n");
#else
	/* wtf */
	printf("wtf\n");
#endif

	void* p;
	printf("ptr size %lu\n", sizeof(p));
	init();

#if 1
	for (int i=0; i < 5; i++) {
	clock_t start1 = clock();
	test(rtAllocator::alloc, rtAllocator::free);
	clock_t end1 = clock();
	double delta = (end1-start1)/(double)CLOCKS_PER_SEC;
	printf("rtAllocator time: %f\n", delta);
	}
#endif
	{
	clock_t start2 = clock();
	test(malloc, free);
	clock_t end2 = clock();
	double delta2 = (end2-start2)/(double)CLOCKS_PER_SEC;
	printf("malloc/free time: %f\n", delta2);
	}
	return 0;
}
