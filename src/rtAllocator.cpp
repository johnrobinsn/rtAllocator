#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "rtAllocator.h"

#if 1 // Hide my sins
#pragma warning (disable: 4311)
#pragma warning (disable: 4312)
#endif

#define ALLOCJR_ALIGN8 1
//#define ALLOCJR_COUNTERS 1

#ifdef ALLOCJR_COUNTERS
#include "math.h"
#endif

#define ALLOCJR_FULLBLOCK (block*)0xffffffff

#if 0
#include "alloctest.h" // blockalloc and blockfree
#define ALLOCJR_ALLOC blockalloc
#define ALLOCJR_REALLOC blockrealloc
#define ALLOCJR_FREE blockfree
#else
#define ALLOCJR_ALLOC malloc
#define ALLOCJR_REALLOC realloc
#define ALLOCJR_FREE free
#endif

typedef struct 
{
	int fixedAllocSize;
	int chunks;
} blockDescriptor;

blockDescriptor bd[rtAllocator::bdCount];

class block
{
public:

	bool init(int fixedAllocSize, int bdIndex, int chunks)
	{
		mPrevFreeBlock = NULL;
		mNextFreeBlock = NULL;
		mAllocCount = 0;
		mFixedAllocSize = fixedAllocSize;
		mBDIndex = bdIndex;
		mChunks = chunks;

		mLastByte = (unsigned char*)this + getAllocSize(this->mFixedAllocSize, this->mChunks)-1;
		unsigned char* mDataStart = (unsigned char*)this + getHeaderSize(mChunks);
#ifdef ALLOCJR_ALIGN8
		mDataStart = (unsigned char*)(((unsigned long)mDataStart+8)&~7); // 8 byte align
#endif

		mFreeChunk = NULL;
		mInitCursor = mDataStart;

		mTotalCount = mChunks * 16;

		return true;
	}

	inline bool isEmpty()
	{
		return (mAllocCount == 0);
	}

	inline bool isFull()
	{
		return mAllocCount == mTotalCount;
	}

	inline void* alloc()
	{
		void* result;

		if (mFreeChunk)
		{
			result = mFreeChunk;
			mFreeChunk = (void**)*mFreeChunk;
		}
		else
		{
			result = mInitCursor;
			mInitCursor += mFixedAllocSize;
		}

		mAllocCount++;
		return result;
	}

	inline void free(void* p)
	{
		void **pp = (void**)p;
		*pp = mFreeChunk;
		mFreeChunk = (void**)p;
		mAllocCount--;
	}

	static inline unsigned long getHeaderSize(int /*chunks*/)
	{
		return sizeof(block);
	}

	static unsigned long getAllocSize(int fixedAllocSize, int chunks)
	{
#ifdef ALLOCJR_ALIGN8
#define PAD8 +8 // tack on 8 extra bytes to ensure enough room for alignment
#else
#define PAD8
#endif
		return getHeaderSize(chunks) + (fixedAllocSize * 16 * chunks) PAD8;
#undef PAD8
	}

	inline unsigned char* getLastByte()
	{
		return mLastByte;
	}

public:
	block* mNextFreeBlock;
	block* mPrevFreeBlock;
	void** mFreeChunk;
	unsigned long mTotalCount;
	unsigned long mAllocCount;
	unsigned char *mInitCursor;
	unsigned short mFixedAllocSize;
	unsigned short mChunks;
	unsigned char mBDIndex;
	unsigned char* mLastByte;
};

#ifdef ALLOCJR_COUNTERS
// extra space(+1) is used for > max band allocations
long gAllocCounter[rtAllocator::bdCount+1];
long gFreeCounter[rtAllocator::bdCount+1];

long gBlockAllocCount = 0;
long gBlockFreeCount = 0;
long gMaxBlockCount = 0;

void resetCounters()
{
	for (int i = 0; i <= rtAllocator::bdCount; i++)
	{
		gAllocCounter[i] = gFreeCounter[i] = 0;
	}
	gBlockAllocCount = 0;
	gBlockFreeCount = 0;
	gMaxBlockCount = 0;
}
void dumpCounters()
{
	printf("Statistics\n==========\n\n");
	printf("Bytes\t   Allocs\t    Frees\t   Left\t Chunks\t waste\n");
	printf("-----\t   ------\t    -----\t   ----\t ------\t -----\n");
	for (int i = 0; i < rtAllocator::bdCount; i++)
	{
		printf("%d\t%9ld\t%9ld\t%7ld\t%7g%7g\n", bd[i].fixedAllocSize, gAllocCounter[i], gFreeCounter[i], gAllocCounter[i] - gFreeCounter[i],
			(double)(gAllocCounter[i] - gFreeCounter[i])/16, (((double)(gAllocCounter[i] - gFreeCounter[i])/16)-floor((double)(gAllocCounter[i] - gFreeCounter[i])/16)) * (16*bd[i].fixedAllocSize));
	}
	printf(">%d\t%9ld\t%9ld\t%7ld\t     NA\n", bd[rtAllocator::bdCount-1].fixedAllocSize, gAllocCounter[rtAllocator::bdCount], 
		gFreeCounter[rtAllocator::bdCount], 
		gAllocCounter[rtAllocator::bdCount] - gFreeCounter[rtAllocator::bdCount]);

#if 1
	printf("block descriptors\n");
	for (int i = 0; i < rtAllocator::bdCount; i++)
	{
		printf("{%d,0,%7ld},\n",bd[i].fixedAllocSize, 
			(((gAllocCounter[i] - gFreeCounter[i]+15)/16)+15)/16);
	}

#endif
}

void dumpBlockCounters()
{
	printf("Total Blocks Allocated = %ld\n", gBlockAllocCount);
	printf("Total Blocks Freed     = %ld\n", gBlockFreeCount);
	printf("Maximum Number Blocks  = %ld\n", gMaxBlockCount);
}

#define INCALLOCCOUNTER(i) gAllocCounter[i]++
#define INCFREECOUNTER(i) gFreeCounter[i]++

#define INCBLOCKCOUNTER() gBlockAllocCount++; if ((gBlockAllocCount-gBlockFreeCount) > gMaxBlockCount) gMaxBlockCount = (gBlockAllocCount-gBlockFreeCount);
#define DECBLOCKCOUNTER() gBlockFreeCount++;
#else
#define INCALLOCCOUNTER(i)
#define INCFREECOUNTER(i)

#define INCBLOCKCOUNTER()
#define DECBLOCKCOUNTER()
#endif

void rtAllocator::init() 
{
	if (mInited) return;

	// initialize the block descriptors for each heap.
	bd[0].fixedAllocSize = 4;
	bd[0].chunks = 3;
	// initialize for multiple of 8
	for (int i = 1; i < bdCount; i++)
	{
		int allocSize = i*8;
		int chunks = 11;

		bd[i].fixedAllocSize = allocSize;

		if (allocSize == 8) chunks = 3;
		else if (allocSize == 1008) chunks = 1;
		else if (allocSize == 1016) chunks = 1;
		else if (allocSize == 1024) chunks = 1;

		bd[i].chunks = chunks;
	}

	// initialize the freelists for each heap
	for (int i = 0; i < bdCount; i++)
	{
		mFreeBlocks[i] = NULL;
	}

	// lookup table used to quickly find out which heap the alloc request
	// should go to.
	for (int i = 0; i < 1024; i++)
	{
		mBDIndexLookup[i] = -1;
		for (int j = 0; j < bdCount; j++)
		{
			if (i <= bd[j].fixedAllocSize)
			{
				mBDIndexLookup[i] = j;
				break;
			}
		}
	}

	mBlockArray = NULL;
	mInited = true;
}

void* rtAllocator::alloc(size_t ls)
{
	if (ls == 0) ls = 1;
	int bdIndex = -1;
	if (ls <= 1024) bdIndex = mBDIndexLookup[ls];

	if (bdIndex < 0)
	{
		// Not handling blocks of this size throw to blockalloc
		INCALLOCCOUNTER(bdCount);
		return ALLOCJR_ALLOC(ls);
	}
	else
	{
		void* result = NULL;

		if (!mFreeBlocks[bdIndex])
		{
			INCBLOCKCOUNTER();
#if 1
			bd[bdIndex].chunks*=2;
			if (bd[bdIndex].chunks > 20) bd[bdIndex].chunks = 1;
#endif
			block* b = (block*)ALLOCJR_ALLOC(block::getAllocSize(bd[bdIndex].fixedAllocSize, bd[bdIndex].chunks));
			if (b)
			{
				b->init(bd[bdIndex].fixedAllocSize, bdIndex, bd[bdIndex].chunks);

				addBlockToArray(b);
				mFreeBlocks[bdIndex] = b;
			}
		}
		if (mFreeBlocks[bdIndex])
		{
			result = mFreeBlocks[bdIndex]->alloc();
			INCALLOCCOUNTER(bdIndex);
			block *b = mFreeBlocks[bdIndex];

			if (b->mNextFreeBlock != ALLOCJR_FULLBLOCK && b->isFull())
			{
				// Unlink from freelist
				if (b->mNextFreeBlock)
				{
					b->mNextFreeBlock->mPrevFreeBlock = b->mPrevFreeBlock;
				}
				if (b->mPrevFreeBlock)
				{
					b->mPrevFreeBlock->mNextFreeBlock = b->mNextFreeBlock;
				}
				mFreeBlocks[bdIndex] = b->mNextFreeBlock;
				b->mNextFreeBlock = ALLOCJR_FULLBLOCK; // special value means removed from free list
				b->mPrevFreeBlock = ALLOCJR_FULLBLOCK;
			}
		}

		return result;
	}
}

void rtAllocator::free(void* p)
{
	if (!p) return;

	block* b = findBlockInArray(p);
	if (b)
	{
		b->free(p);
		INCFREECOUNTER(block->mMarker);
#if 0
		if (b->isEmpty())
		{
			// Unlink from freelist and return to the system
			if (b->mNextFreeBlock)
			{
				b->mNextFreeBlock->mPrevFreeBlock = b->mPrevFreeBlock;
			}
			if (b->mPrevFreeBlock)
			{
				b->mPrevFreeBlock->mNextFreeBlock = b->mNextFreeBlock;
			}
			if (mFreeBlocks[b->mBDIndex] == b) mFreeBlocks[b->mBDIndex] = b->mNextFreeBlock;

			removeBlockFromArray(b);

			DECBLOCKCOUNTER();
			ALLOCJR_FREE(b);

		}
		else
#endif
		{
			// need to see if block is not in free list if not add it back
			if (b->mNextFreeBlock == ALLOCJR_FULLBLOCK)
			{
				b->mPrevFreeBlock = NULL;
				b->mNextFreeBlock = mFreeBlocks[b->mBDIndex];
				if (mFreeBlocks[b->mBDIndex]) mFreeBlocks[b->mBDIndex]->mPrevFreeBlock = b;
				mFreeBlocks[b->mBDIndex] = b;
			}
		}
	}
	else
	{
		// must not be ours pass to blockfree
		ALLOCJR_FREE(p);
		INCFREECOUNTER(bdCount);
	}
}

void rtAllocator::addBlockToArray(block* b)
{
	if (!mBlockArray)
	{
		mBlockArray = (block**)ALLOCJR_ALLOC(sizeof(block**)*mBlockArraySize);
		mBlockArrayEnd = mBlockArray+mBlockCount-1;
	}
	if (mBlockArraySize < mBlockCount+1)
	{
		mBlockArraySize += 10000;
		mBlockArray= (block**)ALLOCJR_REALLOC(mBlockArray, sizeof(block**)*mBlockArraySize);
		mBlockArrayEnd = mBlockArray+mBlockCount-1;
	}
	bool done = false;
	long s = 0; 
	long e = mBlockCount-1;
	long m = s + ((e-s+1)/2);
	while (s < e)
	{
		if (b < mBlockArray[m])
		{
			e = m-1;
		}
		else if ((unsigned char*)b > mBlockArray[m]->getLastByte())
		{
			s = m+1;
		}
		m = s+ ((e-s+1)/2);
	}
	if (s<mBlockCount && (unsigned char*)b > (unsigned char*)mBlockArray[s]->getLastByte())
	{
		s++;
	}

	// We want to insert as s
	for (long i = mBlockCount-1; i >= s; i--)
	{
		mBlockArray[i+1] = mBlockArray[i];
	}

	mBlockArray[s] = b;
	mBlockCount++;
	mBlockArrayEnd = mBlockArray+mBlockCount-1;
}	

int rtAllocator::findBlockIndex(void* b)
{
	int result = -1;

	if (mBlockCount > 0)
	{
		block** s = mBlockArray;
		block** e = mBlockArrayEnd;
		block** m = s + ((e-s+1)>>1);

		while (s < m) // Given this condition alone we know that m-1 exists
		{
			if ((unsigned char*)b <  (unsigned char*)*m) e = m-1;
			else if ((unsigned char*)b >= (unsigned char*)*m) s = m;
			m = s + ((e-s+1)>>1);
		}

		if (m >= mBlockArray && m <= mBlockArrayEnd) // valid block
			if ((unsigned char*)b >= (unsigned char*)*m && (unsigned char*)b <= (*m)->getLastByte())
				result = m-mBlockArray;

	}

	return result;
}

void rtAllocator::removeBlockFromArray(block* b)
{
	int m = findBlockIndex(b);

	if (m >=0)
	{
		if (mLastFoundBlock == mBlockArray[m]) mLastFoundBlock = NULL;
		for (long i = m+1; i < mBlockCount; i++)
		{
			mBlockArray[i-1] = mBlockArray[i];
		}
		mBlockCount--;
		mBlockArrayEnd = mBlockArray+mBlockCount-1;

		if (mBlockCount == 0)
		{
			ALLOCJR_FREE(mBlockArray);
			mBlockArray = NULL;
			mBlockArrayEnd = NULL;
		}	
	}
}

block* rtAllocator::findBlockInArray(void* p)
{
	if (mLastFoundBlock)
	{
		if ((unsigned char*)mLastFoundBlock <= p && p <= mLastFoundBlock->getLastByte())
		{
			return mLastFoundBlock;
		}
	}

	// binary search code duped here to avoid function call penalty
	block* result = NULL;
	if (mBlockCount > 0)
	{

		block** s = mBlockArray;
		block** e = mBlockArrayEnd;
		block** m = s + ((e-s+1)>>1);
		while (s < m) // Given this condition alone we know that m-1 exists
		{
			if ((unsigned char*)p <  (unsigned char*)*m) e = m-1;
			else if ((unsigned char*)p >= (unsigned char*)*m) s = m;
			m = s + ((e-s+1)>>1);
		}
		if (m >= mBlockArray && m <= mBlockArrayEnd) // valid block
			if ((unsigned char*)p >= (unsigned char*)*m && (unsigned char*)p <= (*m)->getLastByte())
				result = mLastFoundBlock = *m;
	}
	return result;
}

block* rtAllocator::mFreeBlocks[bdCount];
long rtAllocator::mBlockCount = 0;
block* rtAllocator::mLastFoundBlock = NULL;
long rtAllocator::mBlockArraySize = 10000;
block** rtAllocator::mBlockArray = NULL;
block** rtAllocator::mBlockArrayEnd = NULL;
bool rtAllocator::mInited = false;
char rtAllocator::mBDIndexLookup[1025];

// helper class to initialize my allocator
class initializer
{
public:
	initializer()
	{
		rtAllocator::init();
	}
};

initializer gInitializer;
