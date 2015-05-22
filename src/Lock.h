#pragma once


class SynchroObjectDummy {

    SynchroObjectDummy() {};

public:

    static inline void get() {
    }

    static inline void release() {
    }

};

template<typename Mutex> class Lock {

public:
    inline Lock() {
        Mutex::get();
    }

    inline ~Lock() {
        Mutex::release();
    }
};

/**
 * Instantiate a new type - lock which does nothing
 */
typedef Lock<SynchroObjectDummy> LockDummy;



class SynchroObjectOmpLock {
public:
    static inline void get();
    static inline void release();
    ~SynchroObjectOmpLock();

protected:
    omp_lock_t lock;
    bool initialized = false;

    inline SynchroObjectOmpLock() {}
    static SynchroObjectOmpLock *getInstance();
};


void SynchroObjectOmpLock::get() {
    SynchroObjectOmpLock *instance = getInstance();
    omp_set_lock(&instance->lock);
}

void SynchroObjectOmpLock::release() {
    SynchroObjectOmpLock *instance = getInstance();
    omp_unset_lock(&instance->lock);
}

SynchroObjectOmpLock::~SynchroObjectOmpLock() {
    SynchroObjectOmpLock *instance = getInstance();
    omp_destroy_lock(&instance->lock);
}

SynchroObjectOmpLock *SynchroObjectOmpLock::getInstance() {
    static SynchroObjectOmpLock *instance = new SynchroObjectOmpLock();
    if (!instance->initialized) {
        omp_init_lock(&instance->lock);
    }
    return instance;
}


/**
 * Instantiate a new type - lock which does nothing
 */
typedef Lock<SynchroObjectOmpLock> LockOmp;
