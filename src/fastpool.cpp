/**
 * Pool for allocation of small (4 bytes) blocks
 * This is an interview question
 */

#include <string>
#include <iostream>

using namespace std;

static const int POOL_SIZE = 127;
static uint32_t fastPoolData[POOL_SIZE];
static uint32_t fastPoolHead;
static const int ALLOCATED_ENTRY = (POOL_SIZE+1);

static inline uint32_t fastPoolGetNext(uint32_t node)
{
    return fastPoolData[node];
}

static inline void fastPoolSetNext(uint32_t node, uint32_t next)
{
    fastPoolData[node] = next;
}

/**
 * Create a linked list of integers
 */
void fastPoolInitialize()
{
    fastPoolHead = 0;
    for (int i = 0;i < (POOL_SIZE-1);i++)
    {
        fastPoolSetNext(i, i+1);
    }
    fastPoolSetNext(POOL_SIZE-1, ALLOCATED_ENTRY);
}

uint32_t *fastPoolAllocate()
{
    uint32_t blockOffset = fastPoolHead;
    if (blockOffset != ALLOCATED_ENTRY)
    {
        fastPoolHead = fastPoolGetNext(blockOffset);
        return &fastPoolData[blockOffset];
    }

    return nullptr;
}

void fastPoolFree(uint32_t *block)
{
    uint32_t blockOffset = block-&fastPoolData[0];
    fastPoolSetNext(blockOffset, fastPoolGetNext(fastPoolHead));
    fastPoolSetNext(fastPoolHead, blockOffset);
}

void fastPoolPrint()
{
    for (int i = 0;i < POOL_SIZE;i++)
    {
        cout << fastPoolData[i] << " ";
    }
    cout << endl;

}
