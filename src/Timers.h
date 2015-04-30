#pragma once

typedef uint32_t TimerID;

class Timer {
public:

    Timer() {

    }

protected:

    friend class TimerList;

    void setId(TimerID id) {
        this->id = id;
    }

    void setApplicationData(uintptr_t applicationData) {
        this->applicationData = applicationData;
    }

    uintptr_t getApplicationData() const {
        return applicationData;
    }

    TimerID id;

    uintptr_t applicationData;

    /**
     * pointer used to stop the timer
     * this field initialized by aosTimerStart()
     */
    unsigned int timerHandle;
};

typedef void (*TimerExpirationHandler)(void *);
