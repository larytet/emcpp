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
protected:

    SynchroObjectOmpLock() {};

public:

    static inline void get() {
    }

    static inline void release() {
    }

};
