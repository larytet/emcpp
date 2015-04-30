#pragma once

typedef uint32_t TimerID;

enum TimerError {
      Ok           ,
      Expired      ,
      Stppped      ,
      Illegal      ,
      NoFreeTimer  ,
      ILLEGAL_ID   ,
};

class Timer {
public:

    Timer() : isRunning(false) {
    }

protected:

    friend class TimerListBase;

    void setId(TimerID id) {
        this->id = id;
    }

    void setIsrunning(bool flag) {
        isRunning = flag;
    }

    void setApplicationData(uintptr_t applicationData) {
        this->applicationData = applicationData;
    }

    uintptr_t getApplicationData() const {
        return applicationData;
    }

    TimerID id;
    uintptr_t applicationData;
    bool isRunning;
};

typedef void (*TimerExpirationHandler)(void *);

class TimerListBase {

    TimerListBase(uint32_t timeout, TimerExpirationHandler expirationHandler,
            bool callExpiredForStoppedTimers = false) :
            timeout(timeout), expirationHandler(expirationHandler), callExpiredForStoppedTimers(
                    callExpiredForStoppedTimers) {

    }


protected:
    uint32_t timeout;
    TimerExpirationHandler expirationHandler;
    bool callExpiredForStoppedTimers;

};

/**
 * A timer list is a queue of the started timers with the SAME timeout.
 * For example list of 1 s timers
 *
 * @param Size maximum number of pending timers. Performance of the list does not
 * @param Lock is a class synchronizing access to the list API
 * depend on the number of timers.
 */
template<std::size_t Size, Lock> class TimerList : public TimerListBase {

    TimerList(uint32_t timeout, TimerExpirationHandler expirationHandler,
            bool callExpiredForStoppedTimers = false) : TimerListBase(timeout, expirationHandler, callExpiredForStoppedTimers) {

    }

    /**
     * Move a timer from list of of free timers and
     * add the timer to the tail of the list of the started timers
     *
     * @param timer - will be set to reference the started timer
     * @result TimerError:Ok if success
     */
    inline enum TimerError startTimer(Timer& timer) {
        Lock();
        if (!freeTimers.isEmpty) {
            freeTimers.remove(timer);
            runningTimers.add(timer);
            return TimerError::Ok;
        }
        else {
            return TimerError::NoFreeTimer;
        }
    }

    inline enum TimerError stopTimer(Timer& timer) {
        timer.setIsrunning(false);
        return TimerError::Ok;
    }

protected:

    friend class TimerSet;

    CyclicBuffer<Timer*, LockDummy, Size> runningTimers;
    CyclicBuffer<Timer*, LockDummy, Size> freeTimers;
};


/**
 * A timer set is one or more lists of pending timers
 * For example set SlowTimers containing 1s timers, 2s timers and 5s timers
 * and set HighPriorityTimers containing 100 ms timers, 50 ms timers and 200 ms timers
 *
 * @param Size is small and usually is a single digit number. Performance of the set API
 * is linear function of the number of lists in the set.
 */
template<std::size_t Size> class TimerSet {

    TimerSet(const char* name) : name(name) {
    }

    const char *getName() {return name;}

protected:
    const char *name;
    array<TimerListBase*, Size> timerLists;
};
