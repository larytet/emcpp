#pragma once

/**
 *
 * Usage example
 * struct PrintJob {
 *     void run(void) {
 *         cout << "Print job is running" << endl;
 *     }
 * };
 *
 * MemoryPool<LockDummy, JobThread<PrintJob>, 3> jobThreads;
 * PrintJob printJob;
 * JobThread<PrintJob> *jobThread;
 * jobThreads.allocate(&jobThread);
 * jobThread->start(&printJob);
 */


template <typename JobType> class JobThread {
public:

    JobThread();

    void start(JobType *job);

protected:

    JobType *job;

    void mainLoop();
};

template<typename JobType>
JobThread<JobType>::JobThread() : job(nullptr) {
    static const char *name = "a job";
}

template<typename JobType>
void JobThread<JobType>::start(JobType *job) {
    this->job = job;
}

template<typename JobType>
void JobThread<JobType>::mainLoop() {
    while (true) {
    }
}
