#pragma once


template<typename ObjectType, typename Lock, std::size_t Size> class Stack: public Container {
public:

    Stack() :
        Container(Size) {
    }

    ~Stack() {
    }

    bool push(ObjectType* object) {
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

    bool pop(ObjectType** object) {
        Lock();
        if (!isEmpty()) {
            *object = (data[this->head]);
            this->head = this->increment(this->head);
            return true;
        } else {
            errorUnderflow();
            return false;
        }
    }

private:

    ObjectType* data[Size + 1];
};// class Stack
