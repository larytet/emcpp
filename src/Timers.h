#pragma once

typedef uint32_t TimerID;

class Timer {
public:

    Timer() {

    }

protected:

    /**
     * this function will be called by the timer task if the timer expires
     * can be 0
     */
    void (*timerExpired)(void *data);

    /**
     * unique Id of the timer
     * this field initialized by timerStart()
     * can be used to solve the race condition between stopTimer and timerExpired -
     * application can make sure that expired timer has not been stopped a moment before
     * it's expiration
     */
    TimerID Id;

    /**
     * Engine forwards this field as an argument in the timer expiration handler
     */
    void *applicationData;

    /**
     * pointer used to stop the timer
     * this field initialized by aosTimerStart()
     */
    unsigned int timerHandle;
};
