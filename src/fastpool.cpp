/**
 * Pool for allocation of small (4 bytes) blocks
 * This is an interview question
 */

static const int POOL_SIZE = 128;
static uint32_t fastPoolData[POOL_SIZE];
static uint32_t *fastPoolHead;
static const int ALLOCATED_ENTRY = -1;

static inline uint32_t *fastPoolGetNext(uint32_t *node)
{
    return (*node);
}

static inline void fastPoolSetNext(uint32_t *node, uint32_t next)
{
    *node = (uint32_t)next;
}

/**
 * Create a linked list of integers
 */
void fastPoolInitialize()
{
    fastPoolHead = &fastPoolData[0];
    for (int i = 0;i < POOL_SIZE-1;i++)
    {
        fastPoolSetNext(&fastPoolData[i], i+1);
    }
    fastPoolSetNext(&fastPoolData[POOL_SIZE-1], ALLOCATED_ENTRY);
}

uint32_t *fastPoolAllocate()
{
    uint32_t *block = fastPoolHead;
    fastPoolHead = *fastPoolHead;

}


void fastPoolFree()
{

}
