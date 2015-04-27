#pragma once

template<typename ObjectType, typename Lock, std::size_t Size> class CyclicBuffer {
public:

    CyclicBuffer() {

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

    ~CyclicBuffer() {
    }

    /**
     * add element to the tail of the buffer
     */
    inline void add(const ObjectType object) {
        Lock();
        if (!isFull()) {
            data[this->tail] = object;
            this->tail = increment(this->tail);
        } else {
            errorOverflow();
        }

    }

    /**
     * Remove element from the head of the buffer
     */
    inline void remove(ObjectType &object) {
        Lock();
        if (!isEmpty()) {
            object = data[this->head];
            this->head = this->increment(this->head);
        } else {
            errorUnderflow();
        }
    }

    inline bool isEmpty() {
        bool res = (this->head == this->tail);
        return res;
    }

    inline bool isFull() {
        size_t tail = increment(this->tail);
        bool res = (this->head == tail);
        return res;
    }

private:

    inline size_t increment(size_t index) {
        if (index < Size) {
            return (index + 1);
        } else {
            return 0;
        }

    }

    inline void errorOverflow() {
    }
    inline void errorUnderflow() {
    }

    ObjectType data[Size + 1];
    size_t head;
    size_t tail;
};// class CyclicBuffer
