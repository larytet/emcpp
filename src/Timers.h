#pragma once

namespace Timer {
typedef uint32_t TimerID;

/**
 * SystemTime can be any type which supports operations <,>,-,+
 * For example, system tick
 */
typedef uint32_t TimerSystemTime;

/**
 * Timeout can be any type which supports Timeout operator+(SystemTime& rhs)
 *
 */
typedef uint32_t Timeout;

enum TimerError {
    Ok, Expired, Stppped, Illegal, NoFreeTimer, ILLEGAL_ID,
};

class Timer {
public:

    Timer() :
            isRunning(false) {
    }

    TimerID getId() {
        return id;
    }

protected:

    friend class TimerListBase;

    void setId(TimerID id) {
        this->id = id;
    }

    void setExpirationTime(SystemTime systemTime) {
        expirationTime = systemTime;
    }

    SystemTime getExpirationTime() {
        return expirationTime;
    }

    inline void stop() {
        isRunning = false;
    }

    inline void start() {
        isRunning = true;
    }

    inline void setApplicationData(uintptr_t applicationData) {
        this->applicationData = applicationData;
    }

    uintptr_t getApplicationData() const {
        return applicationData;
    }

    /**
     * Unique 32-bits ID of the timer
     * This field initialized by startTimer()
     * This field cab be used to solve the race condition between stopTimer and timerExpired -
     * application keeping a trace of the IDs of all started timers can make sure that
     * the expired timer has not been stopped a moment before it's expiration
     */
    TimerID id;
    uintptr_t applicationData;
    bool isRunning;

    /**
     * Start time + timer timeout
     */
    SystemTime expirationTime;
};

typedef void (*TimerExpirationHandler)(uintptr_t);

class TimerListBase {

    TimerListBase(SystemTime timeout, TimerExpirationHandler expirationHandler,
            bool callExpiredForStoppedTimers = false) :
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
     * @result returns true if there is still running timers on the list
     */
    virtual bool processExpiredTimers() = 0;

protected:

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
     * All timers in the list have the same timeout
     * The expiration time of a timer depends on the start timer
     */
    SystemTime timeout;
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

    inline TimerList(SystemTime timeout,
            TimerExpirationHandler expirationHandler,
            bool callExpiredForStoppedTimers = false) :
            TimerListBase(timeout, expirationHandler,
                    callExpiredForStoppedTimers) {

        // fill up the list of free timers
        for (size_t i = 0; i < Size; i++) {
            freeTimers.add(timers[i]);
        }
    }

    /**
     * Move a timer from the list of free timers and
     * add the timer to the tail of the list of started timers.
     *
     * Application shall update the expiration time of the nearest timer and schedule
     * a call to Set::processExpiredTimers() accordingly
     *
     * @param timer will be set to reference the started timer
     * @param applicationData is provided as an argument in the application callback
     * @param currentTime is a current value for the system tick
     * @param nearestExpirationTime is set to the expiration time of the nearest timer
     * @result TimerError:Ok if success
     */
    inline enum TimerError startTimer(Timer& timer, uintptr_t applicationData,
            SystemTime currentTime, SystemTime& nearestExpirationTime) {
        SystemTime expirationTime = currentTime + timeout;

        Lock();
        if (!freeTimers.isEmpty) {
            freeTimers.remove(timer);
            timer.setExpirationTime(expirationTime);
            timer.setApplicationData(applicationData);
            timer.setId(getNextId());
            timer.start();
            runningTimers.add(timer);
            Timer& headTimer;
            runningTimers.getHead(headTimer);
            nearestExpirationTime = headTimer.getExpirationTime();
            this->nearestExpirationTime = nearestExpirationTime;
            noRunningTimers = false;
            return TimerError::Ok;
        } else {
            return TimerError::NoFreeTimer;
        }
    }

    inline enum TimerError stopTimer(Timer& timer) {
        timer.stop();
        return TimerError::Ok;
    }

    bool getHead(Timer& timer) {
        bool res = runningTimers.getHead(timer);
        return res;
    }

    bool removeHead(Timer& timer) {
        bool res = runningTimers.remove(timer);
        return res;
    }

    bool processExpiredTimers(SystemTime currentTime) {
        Lock();

        while (!isEmpty()) {
            Timer& timer;
            bool res = runningTimers->getHead(timer);
            if (!res)
                break;
            if (timer.expirationTime >= currentTime) {
                (getExpirationHandler())(timer.applicationData);
                runningTimers->remove();
            }
            else if (!timer.isRunning) {
                runningTimers->remove();
            }
            else {
                setNearestExpirationTime(timer.expirationTime);
                break;
            }
        }

        bool res = !isEmpty();
        return res;
    }

protected:


    /**
     * Generates unique ID for the timer
     */
    inline static TimerID getNextId() {
        static TimerID id = 0;
        id++;
        return id;
    }

    friend class TimerSet;

    CyclicBuffer<Timer*, LockDummy, Size> runningTimers;
    CyclicBuffer<Timer*, LockDummy, Size> freeTimers;

    // static allocation of all timers
    array<Timer, Size> timers;
};

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
    bool processExpiredTimers(SystemTime currentTime, SystemTime& expirationTime) {
        TimerListBase* timerList;
        size_t i;
        SystemTime nearestExpirationTime;
        bool res = false;
        for (i = 0; i < listCount; i++) {
            timerList = timerLists[i];

            bool timerRes = timerList->processExpiredTimers();
            if (timerRes) {
                res = true;
                SystemTime listExpirationTime = timerList->getNearestExpirationTime();
                if (!res) {
                    nearestExpirationTime = listExpirationTime;
                }
                else {
                    if (nearestExpirationTime > listExpirationTime) {
                        nearestExpirationTime = listExpirationTime;
                    }
                }
            }
        }

        expirationTime = nearestExpirationTime;
        return res;
    }

    inline bool addList(TimerListBase* list) {
        if (listCount < Size) {
            timerLists[listCount] = list;
            listCount++;
            return true;
        }
        else {
            return false;
        }

    }

protected:
    const char *name;
    array<TimerListBase*, Size> timerLists;
    size_t listCount;
};
