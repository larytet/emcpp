 /**
 *                      -----------   Terminology   ---------------
 *
 *   Timer list - queue of the running timers with the SAME timeout. For example list of 1 s timers
 *   Set - one or more lists of timers
 *       For example set A containing 1s timers, 2s timers and 5s timers
 *       and set B containing 100 ms timers, 50 ms timers and 200 ms timers
 *
 *                      -----------   Design   ---------------
 *
 *   In the system run one or more timers tasks handling different timer sets. Every timer
 *   set contains zero or more timer lists.
 *   Function start timer allocates a free entry from the a pool of free timers and places
 *   the timer in the end of the list of running timers. Complexity of operation is ~O(1)
 *
 *   Timer task waits for the expiration of the nearest timer, calls API processExpiredTimers
 *   The API returns the next timer to be expired using sequential search in the
 *   set. The next timer to expire is always at the head of the timer list. Complexity of the
 *   API is ~O(size of set)
 *   Function stop timer marks a running timer as stopped. Comlexity ~O(1)
 *
 *                      -----------   Reasoning  ---------------
 *
 *   1. It is possible that every subsystem will have it's own timer tasks running in
 *      different priorities.
 *   2. Set of long timers and set of short timers can be created and handled by tasks with
 *      different priorities.
 *   3. "Timer expired"  application handlers can be called from different tasks. For high
 *      priority short timers such handler should be fast - release a semaphore for example,
 *      for low priority long timers the handler can make long processing like audit in a data-base.
 *   4. In the system can coexist 1 or 2 short timers - 50 ms - used in a VoIP state machine
 *      and 10 long timers  - 10 s, 1 min, 1 h, etc. - used in the application
 *      sanity checking or management.
 *   5. In the system can coexist short - 10 ms - timer that always expires and 10 long
 *      DHCP timers that usually get stopped by the application before expiration
 *
 *                      -----------   Example of usage  ---------------
 *
 *    void myTimerTask(TimerSet MySet)
 *    {
 *      int timeOut = FOREVER;
 *      while (1)
 *      {
 *        semGet(MySemaphore, timeOut);
 *        SystemTime nearestExpirationTime;
 *        TimerError err = MySet.processExpiredTimers(currentSystemTick, nearestExpirationTime);
 *        if (err == TimerError::Ok)
 *          timeoOut = nearestExpirationTime - currentSystemTick;
 *        else
 *          timeOut = FOREVER
 *      }
 *    }
 *
 *    startMyTimers()
 *    {
 *      SystemTime nearestExpirationTime;
 *      TimerError err = timerList.startTimer(currentTime, nearestExpirationTime, 0);
 *      if (err == TimerError::Ok)
 *        timeoOut = nearestExpirationTime - currentSystemTick;
 *      else
 *        timeOut = FOREVER
 *       semSend(MySemaphore); // wake up myTimerTask
 *    }
 */
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

enum class TimerError {
    Ok, Expired, Stppped, Illegal, NoFreeTimer, NoRunningTimers
};

class Timer {
public:

    Timer();

    /**
     * Returns timer identifier - a unique on the system level ID of the timer
     * Time ID is set by startTimer()
     *
     * This field cab be used to solve the race condition between stopTimer and timerExpired -
     * application keeping a trace of the IDs of all started timers can make sure that
     * the expired timer has not been stopped a moment before it's expiration
     */
    TimerID getId() const;

    SystemTime getStartTime() const;

    bool isRunning() const;

    inline void stop();

    inline void start();

    inline void setApplicationData(uintptr_t applicationData);

    inline uintptr_t getApplicationData() const;

    void setId(TimerID id);

    void setStartTime(SystemTime systemTime);

protected:

    TimerID id;
    uintptr_t applicationData;
    bool running;
    SystemTime startTime;
};

Timer::Timer() {
    stop();
}

TimerID Timer::getId() const {
    return id;
}

SystemTime Timer::getStartTime() const {
    return startTime;
}

bool Timer::isRunning() const {
    return running;
}

void Timer::stop() {
    running = false;
}

void Timer::start() {
    running = true;
}

void Timer::setApplicationData(uintptr_t applicationData) {
    this->applicationData = applicationData;
}

uintptr_t Timer::getApplicationData() const {
    return applicationData;
}

void Timer::setId(TimerID id) {
    this->id = id;
}

void Timer::setStartTime(SystemTime systemTime) {
    this->startTime = systemTime;
}

template<typename ApplicatonDataType> class TimerApp {

protected:

    TimerID id;
    ApplicatonDataType applicationData;
};

typedef void (*TimerExpirationHandler)(const Timer& timer);

class TimerLock {

public:
    virtual void get() = 0;
    virtual void release() = 0;

protected:
    virtual ~TimerLock() {}

};

class TimerLockDummy : public TimerLock {

public:
    virtual void get() {}
    virtual void release() {}

protected:
};

typedef CyclicBufferDynamic<Timer*, LockDummy> TimerCyclicBuffer;

class TimerList {

public:

    TimerList(size_t size, Timeout timeout, TimerExpirationHandler expirationHandler,
            TimerLock& timerLock,
            bool callExpiredForStoppedTimers=false) :
            timeout(timeout), expirationHandler(expirationHandler), callExpiredForStoppedTimers(
                    callExpiredForStoppedTimers), freeTimers(size), runningTimers(size),
                    timerLock(timerLock) {

        Timer *timers = new Timer[size];
        for (size_t i = 0;i < size;i++) {
            freeTimers.add(&timers[i]);
        }
    }

    /**
     * Remove stopped timers from the list, call application callback
     * for expired timers
     *
     * @result returns TimerError::Ok if there is at least one running timer on the list
     */
    TimerError processExpiredTimers(SystemTime);

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
    enum TimerError startTimer(SystemTime currentTime,
            SystemTime& nearestExpirationTime, uintptr_t applicationData = 0,
            const Timer** timer = nullptr);


    inline enum TimerError stopTimer(Timer& timer) {
        timer.stop();
        return TimerError::Ok;
    }

    SystemTime getNearestExpirationTime() {
        return nearestExpirationTime;
    }


protected:

    /**
     * Generates unique system ID for the timer
     * This method is not thread safe and can require synchronization of access
     */
    inline static TimerID getNextId();

    enum TimerError _startTimer(SystemTime currentTime,
            SystemTime& nearestExpirationTime, uintptr_t applicationData = 0,
            const Timer** timer = nullptr);

    TimerError _processExpiredTimers(SystemTime);

    /**
     * All timers in the list have the same timeout
     * The expiration time of a timer depends on the start timer
     */
    Timeout timeout;

    TimerExpirationHandler expirationHandler;
    bool callExpiredForStoppedTimers;
    SystemTime nearestExpirationTime;

    CyclicBufferDynamic<Timer*, LockDummy> freeTimers;
    CyclicBufferDynamic<Timer*, LockDummy> runningTimers;

    TimerLock& timerLock;
};

TimerID TimerList::getNextId() {
    static TimerID id = 0;
    id++;
    return id;
}

enum TimerError TimerList::startTimer(SystemTime currentTime,
        SystemTime& nearestExpirationTime, uintptr_t applicationData,
        const Timer** timer) {

    timerLock.get();

    TimerError res = TimerList::_startTimer(currentTime,
            nearestExpirationTime, applicationData,
            timer);

    timerLock.release();


    return res;
}

enum TimerError TimerList::_startTimer(SystemTime currentTime,
        SystemTime& nearestExpirationTime, uintptr_t applicationData,
        const Timer** timer) {

    Timer* newTimer;

    if (freeTimers.isEmpty())
        return TimerError::NoFreeTimer;

    freeTimers.remove(&newTimer);
    newTimer->setStartTime(currentTime);
    newTimer->setApplicationData(applicationData);
    newTimer->setId(getNextId());
    newTimer->start();
    runningTimers.add(newTimer);
    Timer* headTimer;
    runningTimers.getHead(&headTimer);
    nearestExpirationTime = headTimer->getStartTime() + timeout;
    this->nearestExpirationTime = nearestExpirationTime;
    if (timer != nullptr)
        *timer = newTimer;

    return TimerError::Ok;
}

TimerError TimerList::processExpiredTimers(
        SystemTime currentTime) {

    timerLock.get();

    TimerError res = TimerList::_processExpiredTimers(currentTime);

    timerLock.release();

    return res;
}

TimerError TimerList::_processExpiredTimers(
        SystemTime currentTime) {
    Timer* timer;

    while (!runningTimers.isEmpty()) {

        if (!runningTimers.getHead(&timer))
            break;

        bool timerExpired = isTimerExpired(timer->getStartTime(), timeout,
                currentTime);
        bool timerIsRunning = timer->isRunning();

        bool callExpirationHandler = timerExpired;
        callExpirationHandler = callExpirationHandler
                || (!timerIsRunning && callExpiredForStoppedTimers);

        if (callExpirationHandler) {
            (expirationHandler)(*timer);
        }

        if (timerExpired || !timerIsRunning) {
            runningTimers.remove(&timer);
            freeTimers.add(timer);
        }

        if (!timerExpired && timerIsRunning) {
            nearestExpirationTime = timer->getStartTime() + timeout;
        }
    }

    if (!runningTimers.isEmpty())
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
class TimerSet {

    /**
     * @param name is a name of the set, useful for debug
     */
    TimerSet(const char* name, int size) :
            name(name), listCount(size) {
        this->size = size;
        timerLists = new TimerList*[size];
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

    bool addList(TimerList* list);

protected:
    const char *name;
    TimerList **timerLists;
    size_t listCount;
    size_t size;
};

TimerError TimerSet::processExpiredTimers(
        SystemTime currentTime, SystemTime& expirationTime) {
    TimerList* timerList;
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

bool TimerSet::addList(TimerList* list) {
    if (listCount < size) {
        timerLists[listCount] = list;
        listCount++;
        return true;
    } else {
        return false;
    }
}

class HardwareTimer : HardwareModule {
public:
    HardwareTimer() {}

protected:
    HardwareRegister32RW CONF;
    HardwareRegister32RW CURR;
    HardwareRegister32RW RST;

    inline void start() {
        CONF = 0x01;
    }

    inline uint32_t read() {
        uint32_t val = CURR;
        return val;
    }

    inline void restart() {
        RST = 0x01;
    }

    inline void stop() {
        CONF = 0x00;
    }
};
