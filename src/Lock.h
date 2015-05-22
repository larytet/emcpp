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
    static inline void get() {
        SynchroObjectOmpLock *instance = getInstance();
        omp_set_lock(&instance->lock);
    }

    static inline void release() {
        SynchroObjectOmpLock *instance = getInstance();
        omp_unset_lock(&instance->lock);
    }

    static void createInstance() {
        SynchroObjectOmpLock *instance = getInstance();
        if (!instance->initialized) {
            omp_init_lock(&instance->lock);
        }
    }

    ~SynchroObjectOmpLock() {
        SynchroObjectOmpLock *instance = getInstance();
        omp_destroy_lock(&instance->lock);
    }

protected:
    omp_lock_t lock;
    bool initialized = false;

    inline SynchroObjectOmpLock() {}

    static SynchroObjectOmpLock *getInstance() {
        static SynchroObjectOmpLock instance;
        return &instance;
    }
};


/**
 * Instantiate a new type - lock which does nothing
 */
typedef Lock<SynchroObjectOmpLock> LockOmp;
