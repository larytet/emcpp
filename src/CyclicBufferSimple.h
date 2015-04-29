#pragma once


template<typename ObjectType, typename Lock, std::size_t Size>
class CyclicBufferSimple {
public:

    CyclicBufferSimple();

    ~CyclicBufferSimple() {}

    inline bool add(const ObjectType object);

    inline bool remove(ObjectType &object);

    inline bool isEmpty();

    inline bool isFull();

private:

    inline size_t increment(size_t index);

    inline void errorOverflow() {}
    inline void errorUnderflow() {}

    ObjectType data[Size + 1];
    size_t head;
    size_t tail;
};



template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBufferSimple<ObjectType, Lock, Size>::CyclicBufferSimple() {

    /**
     * I want to fail compilation if the ObjectType not an integer
     * I check the type traits
     * In C++11 I have 'static_assert'
     */
#if (__cplusplus >= 201103)
    static_assert(std::numeric_limits<ObjectType>::is_integer, "CyclicBuffer is intended to work only with integer types");
#elif defined(__GNUC____)
    __attribute__((unused)) ObjectType val1 = 1;
#else
    volatile ObjectType val1;
    *(&val1) = 1;
#endif
    this->head = 0;
    this->tail = 0;
}


template<typename ObjectType, typename Lock, std::size_t Size>
inline bool CyclicBufferSimple<ObjectType, Lock, Size>::add(const ObjectType object) {
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

template<typename ObjectType, typename Lock, std::size_t Size>
inline bool CyclicBufferSimple<ObjectType, Lock, Size>::remove(ObjectType &object) {
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

template<typename ObjectType, typename Lock, std::size_t Size>
inline bool CyclicBufferSimple<ObjectType, Lock, Size>::isEmpty() {
    bool res = (this->head == this->tail);
    return res;
}

template<typename ObjectType, typename Lock, std::size_t Size>
inline bool CyclicBufferSimple<ObjectType, Lock, Size>::isFull() {
    size_t tail = increment(this->tail);
    bool res = (this->head == tail);
    return res;
}

template<typename ObjectType, typename Lock, std::size_t Size>
inline size_t CyclicBufferSimple<ObjectType, Lock, Size>::increment(size_t index) {
    if (index < Size) {
        return (index + 1);
    } else {
        return 0;
    }
}
