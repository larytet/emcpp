#pragma once


template<typename ObjectType, typename Lock> class Mailbox : protected CyclicBufferDynamic<ObjectType, Lock> {
public:
    Mailbox(const char name, size_t size) :
        CyclicBufferDynamic(size),
        name(name) {
    }

protected:
    const char *name;
};
