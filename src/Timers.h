#pragma once

typedef uint32_t TimerID;

/**
 * SystemTime can be any type which supports operations <,>,-,+
 * For example, system tick
 */
typedef size_t SystemTime;

/**
 * Timeout can be any type which supports Timeout operator+(SystemTime& rhs)
 */
typedef size_t Timeout;

/**
 * Deals with system tick wrap around.
 * Implementation of the method depends on the definition of what System time and Timeout are.
 */
static inline bool isTimerExpired(SystemTime startTime, Timeout timeout,
        SystemTime currentTime) {
    bool timerExpired = false;

    SystemTime timerExpiartionTime = startTime + timeout;
    timerExpired = timerExpired
            || ((timerExpiartionTime >= currentTime)
                    && (startTime < currentTime));
    timerExpired = timerExpired
            || ((timerExpiartionTime >= currentTime)
                    && (startTime > currentTime));
    timerExpired = timerExpired
            || ((timerExpiartionTime <= currentTime)
                    && (startTime > currentTime)
                    && (timerExpiartionTime < startTime));

    return timerExpired;
}

enum TimerError {
    Ok, Expired, Stppped, Illegal, NoFreeTimer, NoRunningTimers
};

class Timer {
public:

    Timer() :
        running(false) {
    }

    /**
     * Returns timer identifier - a unique on the system level ID of the timer
     * Time ID is set by startTimer()
     *
     * This field cab be used to solve the race condition between stopTimer and timerExpired -
     * application keeping a trace of the IDs of all started timers can make sure that
     * the expired timer has not been stopped a moment before it's expiration
     */
    TimerID getId() const {
        return id;
    }

    SystemTime getStartTime() const {
        return startTime;
    }

    bool isRunning() const {
        return running;
    }

    inline void stop() {
        running = false;
    }

    inline void start() {
        running = true;
    }

    inline void setApplicationData(uintptr_t applicationData) {
        this->applicationData = applicationData;
    }

    uintptr_t getApplicationData() const {
        return applicationData;
    }

    void setId(TimerID id) {
        this->id = id;
    }

    void setStartTime(SystemTime systemTime) {
        startTime = systemTime;
    }

protected:

    TimerID id;
    uintptr_t applicationData;
    bool running;

    SystemTime startTime;
};

typedef void (*TimerExpirationHandler)(const Timer& timer);

class TimerListBase {

protected:
    TimerListBase(Timeout timeout, TimerExpirationHandler expirationHandler,
            bool callExpiredForStoppedTimers) :
            timeout(timeout), expirationHandler(expirationHandler), callExpiredForStoppedTimers(
                    callExpiredForStoppedTimers), noRunningTimers(true) {

    }

    inline bool isEmpty() {
        return noRunningTimers;
    }

    /**
     * Remove stopped timers from the list, call application callback
     * for expired timers
     *
     * @result returns TimerError::Ok if there is at least one running timer on the list
     */
    virtual TimerError processExpiredTimers(SystemTime) = 0;

    SystemTime getNearestExpirationTime() {
        return nearestExpirationTime;
    }

    void setNearestExpirationTime(SystemTime time) {
        nearestExpirationTime = time;
    }

    TimerExpirationHandler& getExpirationHandler() {
        return expirationHandler;
    }

    /**
     * Generates unique system ID for the timer
     * This method is not thread safe and can require synchronization of access
     */
    inline static TimerID getNextId() {
        static TimerID id = 0;
        id++;
        return id;
    }

    /**
     * All timers in the list have the same timeout
     * The expiration time of a timer depends on the start timer
     */
    Timeout timeout;
    TimerExpirationHandler expirationHandler;
    bool callExpiredForStoppedTimers;
    SystemTime nearestExpirationTime;
    bool noRunningTimers;

};

/**
 * A timer list is a queue of the started timers with the SAME timeout.
 * For example list of 1 s timers
 *
 * @param Size maximum number of pending timers. Performance of the list does not
 * @param Lock is a class synchronizing access to the list API
 * depend on the number of timers.
 */
template<std::size_t Size, typename Lock> class TimerList: public TimerListBase {

public:
    /**
     * Code bloat here - I duplicate non-trivial initialization routine
     */
    TimerList(Timeout timeout, TimerExpirationHandler expirationHandler,
            bool callExpiredForStoppedTimers = false);

    /**
     * Move a timer from the list of free timers to the tail of the list of started timers.
     * Return time of the next timer expiration
     * Application shall update the expiration time of the nearest timer and schedule
     * a call to Set::processExpiredTimers() accordingly
     *
     * @param timer will be set to reference the started timer
     * @param applicationData is provided as an argument in the application callback
     * @param currentTime is a current value for the system tick
     * @param nearestExpirationTime is set to the expiration time of the nearest timer
     * @result TimerError:Ok if success
     */
    inline enum TimerError startTimer(SystemTime currentTime,
            SystemTime& nearestExpirationTime, uintptr_t applicationData = 0,
            const Timer** _timer = nullptr);

    inline enum TimerError stopTimer(Timer& timer) {
        timer.stop();
        return TimerError::Ok;
    }

    bool getHead(Timer& timer) {
        bool res = runningTimers.getHead(timer);
        return res;
    }


    virtual TimerError processExpiredTimers(SystemTime currentTime);

protected:

    /**
     * This method is not thread safe and can require synchronization of access
     */
    bool removeHead(Timer& timer) {
        bool res = runningTimers.remove(timer);
        return res;
    }

    CyclicBuffer<Timer*, LockDummy, Size> runningTimers;
    CyclicBuffer<Timer*, LockDummy, Size> freeTimers;

    // static allocation of all timers
    array<Timer, Size> timers;

};

template<std::size_t Size, typename Lock> TimerList<Size, Lock>::TimerList(Timeout timeout, TimerExpirationHandler expirationHandler,
        bool callExpiredForStoppedTimers) :
        TimerListBase(timeout, expirationHandler,
                callExpiredForStoppedTimers) {

    // fill up the list of free timers
    for (size_t i = 0; i < Size; i++) {
        freeTimers.add(&timers[i]);
    }
}

template<std::size_t Size, typename Lock> inline enum TimerError TimerList<Size, Lock>::startTimer(SystemTime currentTime,
        SystemTime& nearestExpirationTime, uintptr_t applicationData, const Timer** _timer) {

    Timer *newTimer;

    Lock();

    if (!freeTimers.isEmpty())
        return TimerError::NoFreeTimer;

    freeTimers.remove(newTimer);
    newTimer->setStartTime(currentTime);
    newTimer->setApplicationData(applicationData);
    newTimer->setId(getNextId());
    newTimer->start();
    runningTimers.add(newTimer);
    Timer* headTimer;
    runningTimers.getHead(headTimer);
    nearestExpirationTime = headTimer->getStartTime() + timeout;
    this->nearestExpirationTime = nearestExpirationTime;
    noRunningTimers = false;
    if (_timer != nullptr)
        *_timer = newTimer;

    return TimerError::Ok;
}

template<std::size_t Size, typename Lock> TimerError TimerList<Size, Lock>::processExpiredTimers(SystemTime currentTime) {
    Timer* timer;

    Lock();
    while (!isEmpty()) {

        if (!runningTimers.getHead(timer))
            break;

        bool timerExpired = isTimerExpired(timer->getStartTime(), timeout,
                currentTime);
        bool timerIsRunning = timer->isRunning();

        bool callExpirationHandler = timerExpired;
        callExpirationHandler = callExpirationHandler || (!timerIsRunning && callExpiredForStoppedTimers);

        if (callExpirationHandler) {
            (getExpirationHandler())(*timer);
        }

        if (timerExpired || !timerIsRunning) {
            runningTimers.remove(timer);
        }

        if (!timerExpired && timerIsRunning) {
            setNearestExpirationTime(timer->getStartTime() + timeout);
        }
    }

    if (!isEmpty())
        return TimerError::Ok;
    else
        return TimerError::NoRunningTimers;
}


/**
 * A timer set is one or more lists of pending timers
 * For example set SlowTimers containing 1s timers, 2s timers and 5s timers
 * and set HighPriorityTimers containing 100 ms timers, 50 ms timers and 200 ms timers
 *
 *
 * @param Size is small and usually is a single digit number. Performance of the set API
 * is linear function of the number of lists in the set.
 */
template<size_t Size> class TimerSet {

    /**
     * @param name is a name of the set, useful for debug
     */
    TimerSet(const char* name, int size) :
            name(name), listCount(0) {
    }

    const char *getName() {
        return name;
    }

    /**
     * Process all expired timers and remove stopped timers from the list of
     * running timers
     * Application will call this method to calculate time before the next timer expiration.
     * The return value can be used in, for example, call to a semaphore with timeout
     * Performance of this method depends on the number of lists in the set.
     *
     * All expiration handlers are called from this method.
     *
     * This function creates some code bloat - it is duplicated for every TimerSet object in the
     * system, unless the linker takes care to remove duplicate code.
     *
     * @param currentTime is current value of the system tick
     * @param currentTime is current value of the system tick
     * @result true if new expiration time is available, false if no timers are running
     */
    TimerError processExpiredTimers(SystemTime currentTime,
            SystemTime& expirationTime);

    inline bool addList(TimerListBase* list);

protected:
    const char *name;
    array<TimerListBase*, Size> timerLists;
    size_t listCount;
};

template<size_t Size> TimerError TimerSet<Size>::processExpiredTimers(SystemTime currentTime,
        SystemTime& expirationTime) {
    TimerListBase* timerList;
    size_t i;
    SystemTime nearestExpirationTime;
    bool res = false;
    for (i = 0; i < listCount; i++) {
        timerList = timerLists[i];

        TimerError timerRes = timerList->processExpiredTimers(currentTime);
        res = res || (timerRes == TimerError::Ok);

        if (timerRes == TimerError::Ok) {
            SystemTime listExpirationTime =
                    timerList->getNearestExpirationTime();
            if (!res) {
                nearestExpirationTime = listExpirationTime;
            } else {
                if (nearestExpirationTime > listExpirationTime) {
                    nearestExpirationTime = listExpirationTime;
                }
            }
        }
    }
    if (res)
        return TimerError::Ok;
    else
        return TimerError::NoRunningTimers;
}

template<size_t Size> inline bool TimerSet<Size>::addList(TimerListBase* list) {
    if (listCount < Size) {
        timerLists[listCount] = list;
        listCount++;
        return true;
    } else {
        return false;
    }
}
