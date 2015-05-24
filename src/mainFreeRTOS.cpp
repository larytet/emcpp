
/* System headers. */

extern "C" {

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "partest.h"
#include "semphr.h"

/* Demo file headers. */
#include "BlockQ.h"
#include "PollQ.h"
#include "death.h"
#include "crflash.h"
#include "flop.h"
#include "print.h"
#include "fileIO.h"
#include "semtest.h"
#include "integer.h"
#include "dynamic.h"
#include "mevents.h"
#include "crhook.h"
#include "blocktim.h"
#include "GenQTest.h"
#include "QPeek.h"
#include "countsem.h"
#include "recmutex.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSocket.h"
#include "AsyncIO/PosixMessageQueueIPC.h"
#include "AsyncIO/AsyncIOSerial.h"


void vApplicationIdleHook( void );
}

#include <string>
#include <cstring>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <array>
#include <limits>
#include <atomic>
#include <cstdint>


using namespace std;


static unsigned long uxQueueSendPassedCount = 0;
void vMainQueueSendPassed( void )
{
    /* This is just an example implementation of the "queue send" trace hook. */
    uxQueueSendPassedCount++;
}


void vApplicationIdleHook( void )
{
    vCoRoutineSchedule();   /* Comment this out if not using Co-routines. */
}


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

/**
 * Instantiate a new type - lock which does nothing
 */
typedef Lock<SynchroObjectDummy> LockDummy;

class StackBase {

public:
    inline bool isEmpty() {
        bool res = (this->top == 0);
        return res;
    }

    inline bool isFull() {
        bool res = (this->top == size);
        return res;
    }

protected:
    StackBase(size_t size) {
        this->size = size;
        this->top = 0;
    }

    void errorOverflow() {
    }

    void errorUnderflow() {
    }

    size_t top;
    size_t size;

};// StackBase

template<typename ObjectType, typename Lock, std::size_t Size>
class Stack: public StackBase {
public:

    Stack() :
        StackBase(Size) {
    }

    ~Stack() {
    }

    inline bool push(ObjectType* object);
    inline bool pop(ObjectType** object);

private:

    ObjectType* data[Size + 1];
};// class Stack

template<typename ObjectType, typename Lock, std::size_t Size>
inline bool Stack<ObjectType, Lock, Size>::push(ObjectType* object) {
    Lock lock;
    if (!isFull()) {
        data[this->top] = object;
        this->top++;
        return true;
    } else {
        errorOverflow();
        return false;
    }
}

template<typename ObjectType, typename Lock, std::size_t Size>
inline bool Stack<ObjectType, Lock, Size>::pop(ObjectType** object) {
    Lock lock;
    if (!isEmpty()) {
        this->top--;
        *object = (data[this->top]);
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}

template<typename Lock, typename ObjectType, size_t Size> class MemoryPool {

public:

    MemoryPool() {
        for (int i = 0;i < Size;i++) {
            pool.push(&objects[i]);
        }
    }

    ~MemoryPool() {}

    inline bool allocate(ObjectType **obj) {
        bool res;
        Lock lock;
        res = pool.pop(obj);
        return res;
    }

    inline bool free(ObjectType *obj) {
        bool res;
        Lock lock;
        res = pool.push(obj);
        return res;
    }

protected:
    Stack<ObjectType, LockDummy,  Size> pool;
    ObjectType objects[Size];
};


template <typename JobType> class JobThread {
public:

    JobThread();

    void start(JobType *job);

protected:

    JobType *job;
    xTaskHandle pxCreatedTask;
    xQueueHandle signal;

    void mainLoop();
};

template<typename JobType>
JobThread<JobType>::JobThread() : job(nullptr) {
    static const char *name = "a job";
    vSemaphoreCreateBinary(this->signal);
    void *pMainLoop = (void*)&JobThread<JobType>::mainLoop;
    portBASE_TYPE res = xTaskCreate((pdTASK_CODE)pMainLoop, (const signed char *)name, 300, this, 1, &this->pxCreatedTask);
    if (res != pdPASS) {
        cout << "Failed to create a task" << endl;
    }
}

template<typename JobType>
void JobThread<JobType>::start(JobType *job) {
    this->job = job;
    xSemaphoreGive(this->signal);
}

template<typename JobType>
void JobThread<JobType>::mainLoop() {
    while (true) {
        xSemaphoreTake(signal, portMAX_DELAY);
        if (job != nullptr) {
            job->run();
        }
        job = nullptr;
    }
}



template<typename ObjectType, typename Lock> class CyclicBufferDynamic {
public:

    inline CyclicBufferDynamic(size_t size, void *address=nullptr);

    ~CyclicBufferDynamic() {
    }

    inline bool isEmpty();
    inline bool isFull();
    inline bool add(ObjectType object);
    inline bool remove(ObjectType *object);
    inline bool getHead(ObjectType *object);

private:
    void errorOverflow() {
    }

    void errorUnderflow() {
    }

    size_t increment(size_t index);

    ObjectType *data;
    size_t head;
    size_t tail;
    size_t size;
};

template<typename ObjectType, typename Lock> inline CyclicBufferDynamic<
        ObjectType, Lock>::CyclicBufferDynamic(size_t size, void *address) {

    this->head = 0;
    this->tail = 0;
    this->size = size;
    if (address != nullptr) {
        this->data = new (address) ObjectType[size];
    }
    else {
        this->data = new ObjectType[size];
    }

    static_assert(sizeof(ObjectType) <= sizeof(uintptr_t), "CyclicBuffer is intended to work only with integer types or pointers");
}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::isEmpty() {
    bool res = (this->head == this->tail);
    return res;
}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::isFull() {
    size_t tail = increment(this->tail);
    bool res = (this->head == tail);
    return res;
}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::add(ObjectType object) {
    Lock lock;
    if (!isFull()) {
        data[this->tail] = object;
        this->tail = increment(this->tail);
        return true;
    } else {
        errorOverflow();
        return false;
    }
}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::remove(ObjectType *object) {
    Lock lock;
    if (!isEmpty()) {
        *object = data[this->head];
        this->head = this->increment(this->head);
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::getHead(ObjectType *object) {
    Lock lock;
    if (!isEmpty()) {
        *object = data[this->head];
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}

template<typename ObjectType, typename Lock> size_t CyclicBufferDynamic<
ObjectType, Lock>::increment(size_t index) {
    if (index < this->size) {
        return (index + 1);
    } else {
        return 0;
    }
}

template<typename ObjectType, typename Lock>
class Mailbox {
public:
    Mailbox(const char *name, size_t size);
    const char *getName() {return name;}

    inline bool send(ObjectType msg);

    enum TIMEOUT {NORMAL, NONE, FOREVER};
    inline bool wait(ObjectType *msg,
        TIMEOUT waitType, size_t timeout);

protected:
    const char *name;
    xQueueHandle semaphore;
    CyclicBufferDynamic<ObjectType, Lock> fifo;
};


template<typename ObjectType, typename Lock>
Mailbox<ObjectType, Lock>::Mailbox(const char *name, size_t size) :
    fifo(size),
    name(name) {
    semaphore = xSemaphoreCreateCounting(size+1, 0);
}

template<typename ObjectType, typename Lock>
bool Mailbox<ObjectType, Lock>::send(ObjectType msg) {
    bool res;
    res = fifo.add(msg);
    xSemaphoreGive(semaphore);
    return res;
}

template<typename ObjectType, typename Lock>
bool Mailbox<ObjectType, Lock>::wait(ObjectType *msg,
        TIMEOUT waitType, size_t timeout) {
    bool res = false;
    portBASE_TYPE semaphoreRes = pdFALSE;
    while (semaphoreRes == pdFALSE) {
        if (waitType == TIMEOUT::FOREVER) {
            semaphoreRes = xSemaphoreTake(semaphore, portMAX_DELAY);
        }
        else if (waitType == TIMEOUT::NORMAL) {
            semaphoreRes = xSemaphoreTake(semaphore, timeout);
            break;
        }
        else {
            semaphoreRes = xSemaphoreTake(semaphore, 0);
            break;
        }
    }

    if ((semaphoreRes == pdTRUE) && !fifo.isEmpty()) {
        res = fifo.remove(msg);
    }

    return res;
}

enum EVENT {UART0, UART1};
typedef struct {
    enum EVENT event;
    size_t data[32];
    int dataSize;
} Message;


struct PrintJob {
    void run(void) {
        cout << "Print job is running" << endl;
    }
};

int main( void )
{

#if 0
    MemoryPool<LockDummy, JobThread<PrintJob>, 3> jobThreads;
    PrintJob printJob;
    JobThread<PrintJob> *jobThread;
    jobThreads.allocate(&jobThread);
    jobThread->start(&printJob);
#endif

    MemoryPool<LockDummy, Message, 3> pool;
    Mailbox<Message*, LockDummy> myMailbox("mbx", 3);

    Message *message;
    pool.allocate(&message);
    message->data[0] = 0x30;
    message->dataSize = 1;
    message->event = EVENT::UART0;

    myMailbox.send(message);
    myMailbox.wait(&message, myMailbox.TIMEOUT::FOREVER, 0);
    cout << "data=" << message->data << ", event=" << message->event << endl;
    pool.free(message);

    vTaskStartScheduler();


	return 1;
}
