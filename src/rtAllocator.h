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
	static block* findBlockInArray(void* p);

	static void addBlockToArray(block* b);
	static void removeBlockFromArray(block* b);

	static block* mFreeBlocks[bdCount];
	static long mBlockCount;
	static long mBlockArraySize;
	static block** mBlockArray; 
	static block** mBlockArrayEnd;
	static bool mInited;
	static block* mLastFoundBlock;
	static char mBDIndexLookup[1025];
};

