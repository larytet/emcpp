#pragma once


class StackBase {

public:
    inline bool isEmpty() {
        bool res = (this->top == 0);
        return res;
    }

    inline bool isFull() {
        bool res = (this->top == size);
        return res;
    }

protected:
    StackBase(size_t size) {
        this->size = size;
        this->top = 0;
    }

    void errorOverflow() {
    }

    void errorUnderflow() {
    }

    size_t top;
    size_t size;

};// StackBase

template<typename ObjectType, typename Lock, std::size_t Size>
class Stack: public StackBase {
public:

    Stack() :
        StackBase(Size) {
    }

    ~Stack() {
    }

    inline bool push(ObjectType* object);
    inline bool pop(ObjectType** object);

private:

    ObjectType* data[Size + 1];
};// class Stack

template<typename ObjectType, typename Lock, std::size_t Size>
inline bool Stack<ObjectType, Lock, Size>::push(ObjectType* object) {
    Lock lock;
    if (!isFull()) {
        data[this->top] = object;
        this->top++;
        return true;
    } else {
        errorOverflow();
        return false;
    }

}

template<typename ObjectType, typename Lock, std::size_t Size>
inline bool Stack<ObjectType, Lock, Size>::pop(ObjectType** object) {
    Lock lock;
    if (!isEmpty()) {
        this->top--;
        *object = (data[this->top]);
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}



template<typename ObjectType, typename Lock>
class StackDynamic: public StackBase {
public:

    StackDynamic(std::size_t size) :
        StackBase(size) {
        data = new ObjectType*[size+1];
    }

    ~StackDynamic() {
    }

    inline bool push(ObjectType* object);
    inline bool pop(ObjectType** object);

private:

    ObjectType* data;
};// class Stack

template<typename ObjectType, typename Lock>
inline bool StackDynamic<ObjectType, Lock>::push(ObjectType* object) {
    Lock lock;
    if (!isFull()) {
        data[this->top] = object;
        this->top++;
        return true;
    } else {
        errorOverflow();
        return false;
    }

}

template<typename ObjectType, typename Lock>
inline bool StackDynamic<ObjectType, Lock>::pop(ObjectType** object) {
    Lock lock;
    if (!isEmpty()) {
        this->top--;
        *object = (data[this->top]);
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}
