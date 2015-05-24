#pragma once

/**
 * Usage example:
 *
 * enum EVENT {FIRST, LAST};
 * typedef struct {
 *     enum EVENT event;
 *     size_t data;
 * } Message;
 *
 * MemoryPool<LockDummy, Message, 3> pool;
 * Mailbox<Message*, LockDummy> myMailbox("mbx", 3);
 *
 * Message *message;
 * pool.allocate(&message);
 * message->data = 1;
 * message->event = EVENT::FIRST;
 *
 * myMailbox.send(message);
 * myMailbox.wait(&message, myMailbox.TIMEOUT::FOREVER, 0);
 * cout << "data=" << message->data << ", event=" << message->event << endl;
 * pool.free(message);
 *
 */


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
