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
 * Usage:
 * void lockTest()
 * {
 *    LockDummy lock();
 * }
 */
typedef Lock<SynchroObjectDummy> LockDummy;



class SynchroObjectOmpLock {
public:
    static inline void get();
    static inline void release();
    ~SynchroObjectOmpLock();

protected:
    omp_lock_t lock;

    static SynchroObjectOmpLock *instance;
    inline SynchroObjectOmpLock();
};
SynchroObjectOmpLock *SynchroObjectOmpLock::instance = new SynchroObjectOmpLock();

void SynchroObjectOmpLock::get() {
    omp_set_lock(&instance->lock);
}

void SynchroObjectOmpLock::release() {
    omp_unset_lock(&instance->lock);
}

SynchroObjectOmpLock::SynchroObjectOmpLock() {
    omp_init_lock(&lock);
}

SynchroObjectOmpLock::~SynchroObjectOmpLock() {
    omp_destroy_lock(&instance->lock);
}



/**
 * Instantiate a new type - lock which does nothing
 */
typedef Lock<SynchroObjectOmpLock> LockOmp;
