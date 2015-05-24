#pragma once

struct PrintJob {
    void run(void) {
        cout << "Print job is running" << endl;
    }
};


template <typename JobType> class JobThread {
public:

    JobThread();

    void start(JobType *job);

protected:

    JobType *job;
    xTaskHandle pxCreatedTask;
    xQueueHandle signal;

    void mainLoop();
};

template<typename JobType>
JobThread<JobType>::JobThread() : job(nullptr) {
    static const char *name = "a job";
    vSemaphoreCreateBinary(this->signal);
    void *pMainLoop = (void*)&JobThread<JobType>::mainLoop;
    portBASE_TYPE res = xTaskCreate((pdTASK_CODE)pMainLoop, (const signed char *)name, 300, this, 1, &this->pxCreatedTask);
    if (res != pdPASS) {
        cout << "Failed to create a task" << endl;
    }
}

template<typename JobType>
void JobThread<JobType>::start(JobType *job) {
    this->job = job;
    xSemaphoreGive(this->signal);
}

template<typename JobType>
void JobThread<JobType>::mainLoop() {
    while (true) {
        xSemaphoreTake(signal, portMAX_DELAY);
        if (job != nullptr) {
            job->run();
        }
        job = nullptr;
    }
}
