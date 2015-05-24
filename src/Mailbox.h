#pragma once


template<typename ObjectType, typename Lock> class Mailbox :
        protected CyclicBufferDynamic<ObjectType, Lock> {
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
};


template<typename ObjectType, typename Lock>
Mailbox<ObjectType, Lock>::Mailbox(const char *name, size_t size) :
    CyclicBufferDynamic<ObjectType, Lock>(size),
    name(name) {
    semaphore = xSemaphoreCreateCounting(size+1, 0);
}

template<typename ObjectType, typename Lock>
bool Mailbox<ObjectType, Lock>::send(ObjectType msg) {
    bool res;
    res = this->add(msg);
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

    if ((semaphoreRes == pdTRUE) && !this->isEmpty()) {
        res = this->remove(msg);
    }

    return res;
}
