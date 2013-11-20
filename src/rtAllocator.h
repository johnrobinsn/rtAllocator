// memtest.cpp : Defines the entry point for the console application.
//

class block;
class initializer;

class rtAllocator
{
public:
	static void* alloc(size_t ls);
	static void free(void* p);

	static const int bdCount = 129;
private:
	friend class initializer;
	// must be called before first alloc
	static void init();

	static int findBlockIndex(void* b);
	static block* findBlockInArray(void* p, int& i);

	static void addBlockToArray(block* b);
	static void removeBlockFromArray(block* b);
	static void removeBlockIndexFromArray(int i);

	static block* mFreeBlocks[bdCount];
	static long mBlockCount;
	static long mBlockArraySize;
	static block** mBlockArray; 
	static block** mBlockArrayEnd;
	static bool mInited;
	static block* mLastFoundBlock;
	static int mLastFoundBlockIndex;
	static unsigned char mBDIndexLookup[1025];
};

void*   blockalloc(long size);
void*   blockrealloc(void* fp, long newsize);
void    blockfree(void* p);