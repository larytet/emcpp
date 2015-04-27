#pragma once

template<typename ObjectType, typename Lock, std::size_t Size> class CyclicBuffer: public Container {
public:

    CyclicBuffer() :
            Container(Size) {
        static_assert(std::numeric_limits<ObjectType>::is_integer, "CyclicBuffer is intended to work only with integer types");
    }

    ~CyclicBuffer() {
    }

    bool add(const ObjectType object) {
        Lock();
        if (!isFull()) {
            data[this->tail] = object;
            this->tail = increment(this->tail);
            return true;
        } else {
            errorOverflow();
            return false;
        }

    }

    bool remove(ObjectType &object) {
        Lock();
        if (!isEmpty()) {
            object = data[this->head];
            this->head = this->increment(this->head);
            return true;
        } else {
            errorUnderflow();
            return false;
        }
    }

private:

    ObjectType data[Size + 1];
};// class CyclicBuffer
