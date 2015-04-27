#pragma once


template<typename ObjectType, typename Lock, std::size_t Size> class Stack: public Container {
public:

    Stack() :
        Container(Size) {
    }

    ~Stack() {
    }

    void push(const ObjectType* object) {
        Lock();
        if (!isFull()) {
            data[this->tail] = object;
            this->tail = increment(this->tail);
        } else {
            errorOverflow();
        }

    }

    void pop(const ObjectType** object) {
        Lock();
        if (!isEmpty()) {
            *object = data[this->head];
            this->head = this->increment(this->head);
        } else {
            errorUnderflow();
        }
    }

private:

    const ObjectType* data[Size + 1];
};// class Stack
