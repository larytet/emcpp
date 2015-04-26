//============================================================================
// Name        : main.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <string>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <array>
#include <limits>

using namespace std;


#define PERFORMANCE
#define PERFORMANCE_LOOPS (1000*1000*1000)
#define EXAMPLE 5


#if EXAMPLE == 1

/**
 * Dummy lock
 */
class LockDummy {
public:

    LockDummy() {
        cout << "Locked context" << endl;
    }

    ~LockDummy() {
        cout << "Lock is freed" << endl;
    }

protected:

private:

};// class LockDummy

/*
 * Output of this code is going to be
 * Locked context
 * Lock is freed
 */
int main() {
#if (__cplusplus >= 201103) // use "auto" if C++11 or better
    auto myDummyLock = LockDummy();
#else
    LockDummy myDummyLock = LockDummy();
#endif
    return 0;
}

#endif // EXAMPLE == 1

#if EXAMPLE == 2

/**
 * Dummy lock
 */
class LockDummy {
public:

    LockDummy() {
        cout << "Locked context" << endl;
    }

    ~LockDummy() {
        cout << "Lock is freed" << endl;
    }

protected:

private:

};// class LockDummy

/*
 * Output of this code is going to be
 * Locked context
 * Lock is freed
 * End of main
 */
int main() {
    {
        auto myDummyLock = LockDummy();
    }

    cout << "End of main" << endl;
    return 0;
}

#endif // EXAMPLE == 2

#if EXAMPLE == 3

static inline void interruptDisable(void) {
    cout << "Disable" << endl;
}

static inline void interruptEnable(void) {
    cout << "Enable" << endl;
}

class SynchroObject {
public:
    void inline get() const {
        interruptDisable();
    }

    void inline release() const {
        interruptEnable();
    }
};// class SynchroObject




template<typename _TMutex> class Lock {

    // A private member of the class
    const _TMutex* mutex;

public:
    inline Lock(const _TMutex& mutex) {
        this->mutex = &mutex;
        this->mutex->get();
    }

    inline ~Lock() {
        this->mutex->release();
    }
};// class Lock

static SynchroObject myInterruptDisable;

/*
 * Output of this code is going to be
 * Disable
 * Enable
 * End of main
 */
int main() {
    {
        auto myInterruptLock = Lock<SynchroObject>(
                myInterruptDisable);

    }
    return 0;
}
#endif // EXAMPLE == 3


#if EXAMPLE == 4

static inline void interruptDisable(void) {
    cout << "Disable" << endl;
}

static inline void interruptEnable(void) {
    cout << "Enable" << endl;
}

class SynchroObject {

    /**
     * Default constructor is private - I do not want any objects of this type
     */
    SynchroObject() {};

public:

    static inline void get() {
        interruptDisable();
    }

    static inline void release() {
        interruptEnable();
    }

};// class SynchroObject

template<typename Mutex> class Lock {

public:
    inline Lock() {
        Mutex::get();
    }

    inline ~Lock() {
        Mutex::release();
    }
};// class Lock

/**
 * Declare a new type Lock which uses SynchroObject to
 * disable/enable interrupts
 */
typedef Lock<SynchroObject> Lock_t;

/*
 * Output of this code is going to be
 * Disable
 * Enable
 */
int main() {
    {
        /*
         * Just call a constructor, I skip declaring of a variable here
         */
        Lock_t();
    }
    return 0;
}

#endif // EXAMPLE === 4


#if EXAMPLE == 5

class SynchroObjectDummy {

    SynchroObjectDummy() {};

public:

    static inline void get() {
    }

    static inline void release() {
    }

};// class SynchroObjectDummy

template<typename Mutex> class Lock {

public:
    inline Lock() {
        Mutex::get();
    }

    inline ~Lock() {
        Mutex::release();
    }
};// class Lock

template<typename ObjectType, typename Lock, std::size_t Size> class CyclicBuffer {
public:

    CyclicBuffer() {

        /**
         * I want to fail compilation if the ObjectType not an integer
         * I check the type traits
         * In C++11 I have 'static_assert'
         */
#if (__cplusplus >= 201103)
        static_assert(std::numeric_limits<ObjectType>::is_integer, "CyclicBuffer is intended to work only with integer types");
#elif defined(__GNUC____)
        __attribute__((unused)) ObjectType val1 = 1;
#else
        volatile ObjectType val1;
        *(&val1) = 1;
#endif
        this->head = 0;
        this->tail = 0;
    }

    ~CyclicBuffer() {
    }

    /**
     * add element to the tail of the buffer
     */
    inline void add(const ObjectType object) {
        Lock();
        if (!isFull()) {
            data[this->tail] = object;
            this->tail = increment(this->tail);
        } else {
            errorOverflow();
        }

    }

    /**
     * Remove element from the head of the buffer
     */
    inline void remove(ObjectType &object) {
        Lock();
        if (!isEmpty()) {
            object = data[this->head];
            this->head = this->increment(this->head);
        } else {
            errorUnderflow();
        }
    }

    inline bool isEmpty() {
        bool res = (this->head == this->tail);
        return res;
    }

    inline bool isFull() {
        size_t tail = increment(this->tail);
        bool res = (this->head == tail);
        return res;
    }

private:

    inline size_t increment(size_t index) {
        if (index < Size) {
            return (index + 1);
        } else {
            return 0;
        }

    }

    inline void errorOverflow() {
    }
    inline void errorUnderflow() {
    }

    ObjectType data[Size + 1];
    size_t head;
    size_t tail;
};// class CyclicBuffer

/**
 * Instantiate a new type - lock which does nothing
 */
typedef Lock<SynchroObjectDummy> LockDummy_t;

/**
 * Function returns number of elements in the cyclic buffer.
 * Compiler will fail if the value can not be calculated in compilation time.
 */
constexpr size_t calculateCyclicBufferSize() {
    return 10;
}

static CyclicBuffer<uint8_t, LockDummy_t, calculateCyclicBufferSize()> myCyclicBuffer;

int main() {
#ifdef PERFORMANCE
    unsigned int count = PERFORMANCE_LOOPS;
    while (--count)
    {
        uint8_t val;
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
        myCyclicBuffer.add((uint8_t)count);
        myCyclicBuffer.remove(val);
    }
#else
    // I want to inspect assembly code generated by the C++ compiler
    myCyclicBuffer.add(0);
    // I demonstrate here a range based loop from C++11
    for (int i : { 1, 2, 3 }) {
        myCyclicBuffer.add(i);
    }

    while (!myCyclicBuffer.isEmpty()) {
        uint8_t val;
        myCyclicBuffer.remove(val);
        cout << (int) val << endl;
    }
#endif
    return 0;
}

#if 0
xor %esi,%esi
  inline void add(const ObjectType object) {
mov 0x2009c0(%rip),%rax
          return (index + 1);
mov %rsi,%r8
...............................................
  inline void add(const ObjectType object) {
mov %rcx,%rdx
cmovbe %rdi,%r8
      if (!isFull()) {
cmp %r8,%rax
...............................................
          data[this->tail] = object;
movb $0x0,0x6011a0(%rcx)
          return (index + 1);
mov %r8,%rdx
          this->tail = increment(this->tail);
mov %r8,0x2009a0(%rip)
#endif

#endif // EXAMPLE === 5


#if EXAMPLE == 6

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

static inline void CyclicBufferAdd(CyclicBuffer* cyclicBuffer, const CYCLIC_BUFFER_OBJECT_TYPE object) {
    if (!CyclicBufferIsFull(cyclicBuffer)) {
        cyclicBuffer->data[cyclicBuffer->tail] = object;
        cyclicBuffer->tail = CyclicBufferIncrement(cyclicBuffer->tail, CYCLIC_BUFFRE_SIZE);
    } else {
        errorOverflow();
    }

}

static inline void CyclicBufferRemove(CyclicBuffer* cyclicBuffer, CYCLIC_BUFFER_OBJECT_TYPE* object) {
    if (!CyclicBufferIsEmpty(cyclicBuffer)) {
        *object = cyclicBuffer->data[cyclicBuffer->head];
        cyclicBuffer->head = CyclicBufferIncrement(cyclicBuffer->head, CYCLIC_BUFFRE_SIZE);
    } else {
        errorUnderflow();
    }
}

int main() {
#ifdef PERFORMANCE
    unsigned int count = PERFORMANCE_LOOPS;
    while (--count)
    {
        uint8_t val;
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
        CyclicBufferAdd(&myCyclicBuffer, (uint8_t)count);
        CyclicBufferRemove(&myCyclicBuffer, &val);
    }
#else
    CyclicBufferAdd(&myCyclicBuffer, 0);

    for (int i = 1;i < 4;i++) {
        CyclicBufferAdd(&myCyclicBuffer, i);
    }

    while (!CyclicBufferIsEmpty(&myCyclicBuffer)) {
        uint8_t val;
        CyclicBufferRemove(&myCyclicBuffer, &val);
        cout << (int) val << endl;
    }
#endif
    return 0;
}

#if 0
        return (index + 1);
  xor %esi,%esi
static inline void CyclicBufferAdd(CyclicBuffer* cyclicBuffer, const CYCLIC_BUFFER_OBJECT_TYPE object) {
  mov 0x2009be(%rip),%rax        # 0x6011b0 <myCyclicBuffer+16>
        return (index + 1);
  mov %rsi,%r8
int main() {
  push %rbp
        return (index + 1);
  lea 0x1(%rcx),%rdi
...............................................
static inline void CyclicBufferAdd(CyclicBuffer* cyclicBuffer, const CYCLIC_BUFFER_OBJECT_TYPE object) {
  mov %rcx,%rdx
  push %rbx
        return (index + 1);
  cmovbe %rdi,%r8
...............................................
    if (!CyclicBufferIsFull(cyclicBuffer)) {
  cmp %r8,%rax
...............................................
        cyclicBuffer->data[cyclicBuffer->tail] = object;
  movb $0x0,0x6011a0(%rcx)

        push   %r12
        xor    %esi,%esi
        mov    0x2009be(%rip),%rax
        mov    %rsi,%r8
        push   %rbp
        lea    0x1(%rdx),%rdi
        cmp    $0x9,%rdx
        mov    %rdx,%rcx
        push   %rbx
        cmovbe %rdi,%r8
        cmp    %r8,%rax
        je     40081c <main+0x3c>
        movb   $0x0,0x6011a0(%rdx)
        mov    %r8,%rcx
        mov    %r8,0x20099c(%rip)
        xor    %r12d,%r12d

#endif

#endif // EXAMPLE === 6
