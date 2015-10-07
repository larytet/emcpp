/**
 * Pool for allocation of small (4 bytes) blocks
 * This is an interview question
 */

void fastPoolInitialize();
uint32_t *fastPoolAllocate();
void fastPoolFree(uint32_t *block);
void fastPoolPrint();
