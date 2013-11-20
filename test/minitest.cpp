// Copyright 2006-2013 John Robinson
// MIT License

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rtAllocator.h"

#define NUMALLOCS 10000

int   sizes[NUMALLOCS];
void* allocs[NUMALLOCS];

typedef void* (MallocFunc)(size_t size);
typedef void  (FreeFunc)(void* p);

void init() {
  for (int i = 0; i < NUMALLOCS; i++) {
		sizes[i] = (((double)rand()/(double)RAND_MAX) * ((double)rand()/(double)RAND_MAX) * ((double)rand()/(double)RAND_MAX) * 
		((double)rand()/(double)RAND_MAX) * ((double)rand()/(double)RAND_MAX) * ((double)rand()/(double)RAND_MAX) * ((double)rand()/(double)RAND_MAX) * 1024);
	}
}

void test(MallocFunc afunc, FreeFunc ffunc) {

	for (int i = 0; i < 1000; i++) {

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

			for (int j = 0; j < NUMALLOCS; j++) {
				ffunc(allocs[j]);
			}

	}

}

int  main() {

	init();

	for (int i=0; i < 3; i++) {
		clock_t start1 = clock();
		test(rtAllocator::alloc, rtAllocator::free);
		clock_t end1 = clock();
		double delta = (end1-start1)/(double)CLOCKS_PER_SEC;
		printf("rtAllocator time: %f\n", delta);
	}

	for (int i=0; i < 3; i++)
	{
		clock_t start2 = clock();
		test(malloc, free);
		clock_t end2 = clock();
		double delta2 = (end2-start2)/(double)CLOCKS_PER_SEC;
		printf("malloc/free time: %f\n", delta2);
	}

	return 0;
}
