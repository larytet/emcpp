#pragma once


#undef CYCLIC_BUFFRE_SIZE
#define CYCLIC_BUFFRE_SIZE 10

#undef CYCLIC_BUFFER_OBJECT_TYPE
#define CYCLIC_BUFFER_OBJECT_TYPE uint8_t

#define CYCLIC_BUFFRE_DECLARE(ObjectType, Size) \
    typedef struct { \
        ObjectType data[Size+1]; \
        size_t head; \
        size_t tail; \
    } CyclicBuffer;\

CYCLIC_BUFFRE_DECLARE(CYCLIC_BUFFER_OBJECT_TYPE, CYCLIC_BUFFRE_SIZE);
CyclicBuffer myCyclicBuffer;

static inline size_t CyclicBufferIncrement(size_t index, size_t size) {
    if (index < size) {
        return (index + 1);
    }
    else {
        return 0;
    }
}


static inline bool CyclicBufferIsEmpty(CyclicBuffer* cyclicBuffer) {
    bool res = (cyclicBuffer->head == cyclicBuffer->tail);
    return res;
}

static inline bool CyclicBufferIsFull(CyclicBuffer* cyclicBuffer) {
    size_t tail = CyclicBufferIncrement(cyclicBuffer->tail, CYCLIC_BUFFRE_SIZE);
    bool res = (cyclicBuffer->head == tail);
    return res;
}


static inline void errorOverflow() {
}

static inline void errorUnderflow() {
}

static inline bool CyclicBufferAdd(CyclicBuffer* cyclicBuffer, const CYCLIC_BUFFER_OBJECT_TYPE object) {
    if (!CyclicBufferIsFull(cyclicBuffer)) {
        cyclicBuffer->data[cyclicBuffer->tail] = object;
        cyclicBuffer->tail = CyclicBufferIncrement(cyclicBuffer->tail, CYCLIC_BUFFRE_SIZE);
        return true;
    } else {
        errorOverflow();
        return false;
    }

}

static inline bool CyclicBufferRemove(CyclicBuffer* cyclicBuffer, CYCLIC_BUFFER_OBJECT_TYPE* object) {
    if (!CyclicBufferIsEmpty(cyclicBuffer)) {
        *object = cyclicBuffer->data[cyclicBuffer->head];
        cyclicBuffer->head = CyclicBufferIncrement(cyclicBuffer->head, CYCLIC_BUFFRE_SIZE);
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}
