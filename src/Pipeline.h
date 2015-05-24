#pragma once


template<typename ObjectType, typename Lock, std::size_t Size>
class PipelineTask {
public:
    PipelineTask(const char *name, PipelineTask *nextStage = nullptr) :
        name(name),
        nextStage(nextStage){}

    void doJob();
    void addJob(ObjectType data);

protected:
    const char *name;
    PipelineTask *nextStage;
    CyclicBuffer<ObjectType, Lock, Size> fifo;
};

template<typename ObjectType, typename Lock, std::size_t Size>
void PipelineTask<ObjectType, Lock, Size>::doJob() {
    while (!fifo.isEmpty()) {
        ObjectType data;
        fifo.remove(&data);
        cout << "Stage:" << name << ", data=" << data << endl;
        nextStage->addJob(data);
    }
}

template<typename ObjectType, typename Lock, std::size_t Size>
void PipelineTask<ObjectType, Lock, Size>::addJob(ObjectType data) {
    fifo.add(data);
}
