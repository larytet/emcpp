#pragma once


class FreeRTOSTask {
public:
    FreeRTOSTask();
    ~FreeRTOSTask();

    void join();
protected:
    static void mainLoop();
    void run();
};
