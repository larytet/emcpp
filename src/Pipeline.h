#pragma once


template<typename ObjectType, typename Lock, std::size_t Size>
class PipeLineTask {
public:
    PipeLineTask(const char *name, PipeLineTask *nextStage = nullptr) :
        name(name),
        nextStage(nextStage){}

    void doJob();
    void addJob(ObjectType data);

protected:
    const char *name;
    PipeLineTask *nextStage;
    CyclicBuffer<ObjectType, Lock, Size> fifo;
};

template<typename ObjectType, typename Lock, std::size_t Size>
void PipeLineTask<ObjectType, Lock, Size>::doJob() {
    while (!fifo.isEmpty()) {
        ObjectType data;
        fifo.remove(&data);
        cout << "Stage:" << name << ", data=" << data << endl;
        nextStage->addJob(data);
    }
}


template<typename ObjectType, typename Lock, std::size_t Size>
void PipeLineTask<ObjectType, Lock, Size>::addJob(ObjectType data) {
    fifo.add(data);
}
