#pragma once

template<typename ObjectType, typename Lock, std::size_t Size> class CyclicBuffer {
public:

    inline CyclicBuffer();

    ~CyclicBuffer() {
    }

    inline bool isEmpty();
    inline bool isFull();
    inline bool add(const ObjectType object);
    inline bool remove(ObjectType &object);
    inline bool getHead(ObjectType &object);

    class iterator
    {
        size_t index;
    public:
        inline iterator(size_t index);
        inline bool operator==(const iterator & iter) const;
        inline bool operator!=(const iterator & iter) const;
        inline iterator & operator++();
        inline iterator operator++(int);
        inline ObjectType & operator*();
        inline ObjectType operator*() const;
        inline ObjectType operator->();
        inline const ObjectType operator->() const;
    };
    inline iterator begin();
    inline iterator begin() const;
    inline iterator end();
    inline iterator end() const;
private:
    void errorOverflow() {
    }

    void errorUnderflow() {
    }

    inline size_t increment(size_t index);

    ObjectType data[Size + 1];
    size_t head;
    size_t tail;
};

template<typename ObjectType, typename Lock, std::size_t Size> inline CyclicBuffer<
        ObjectType, Lock, Size>::CyclicBuffer() {

    this->head = 0;
    this->tail = 0;
    static_assert(sizeof(ObjectType) <= sizeof(uintptr_t), "CyclicBuffer is intended to work only with integer types or pointers");
}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBuffer<
        ObjectType, Lock, Size>::isEmpty() {
    bool res = (this->head == this->tail);
    return res;
}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBuffer<
        ObjectType, Lock, Size>::isFull() {
    size_t tail = increment(this->tail);
    bool res = (this->head == tail);
    return res;
}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBuffer<
        ObjectType, Lock, Size>::add(const ObjectType object) {
    Lock lock;
    if (!isFull()) {
        data[this->tail] = object;
        this->tail = increment(this->tail);
        return true;
    } else {
        errorOverflow();
        return false;
    }

}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBuffer<
        ObjectType, Lock, Size>::remove(ObjectType &object) {
    Lock lock;
    if (!isEmpty()) {
        object = data[this->head];
        this->head = this->increment(this->head);
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBuffer<
        ObjectType, Lock, Size>::getHead(ObjectType &object) {
    Lock lock;
    if (!isEmpty()) {
        object = data[this->head];
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}

template<typename ObjectType, typename Lock, std::size_t Size> size_t CyclicBuffer<
ObjectType, Lock, Size>::increment(size_t index) {
    if (index < Size) {
        return (index + 1);
    } else {
        return 0;
    }
}


template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBuffer<ObjectType, Lock, Size>::iterator::iterator(size_t index) {
}

template<typename ObjectType, typename Lock, std::size_t Size>
bool CyclicBuffer<
ObjectType, Lock, Size>::iterator::operator==(const iterator & iter) const {

}

template<typename ObjectType, typename Lock, std::size_t Size>
bool CyclicBuffer<
ObjectType, Lock, Size>::iterator::operator!=(const iterator & iter) const {

}

template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBuffer<ObjectType, Lock, Size>::iterator
& CyclicBuffer<ObjectType, Lock, Size>::iterator::operator++() {

}

template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator++(int) {

}

template<typename ObjectType, typename Lock, std::size_t Size>
ObjectType & CyclicBuffer<ObjectType, Lock, Size>::iterator::operator*() {
}

template<typename ObjectType, typename Lock, std::size_t Size>
ObjectType CyclicBuffer<ObjectType, Lock, Size>::iterator::operator*() const {
}

template<typename ObjectType, typename Lock, std::size_t Size>
ObjectType CyclicBuffer<ObjectType, Lock, Size>::iterator::operator->() {
}

template<typename ObjectType, typename Lock, std::size_t Size>
const ObjectType CyclicBuffer<ObjectType, Lock, Size>::iterator::operator->() const {
}

template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::begin() {
    return iterator(head);
}

template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::begin() const {
    return iterator(head);
}

template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::end() {
    return iterator(tail);
}

template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::end() const {
    return iterator(tail);
}

template<typename ObjectType, typename Lock> class CyclicBufferDynamic {
public:

    inline CyclicBufferDynamic(size_t size, void *address=nullptr);

    ~CyclicBufferDynamic() {
    }

    inline bool isEmpty();
    inline bool isFull();
    inline bool add(ObjectType object);
    inline bool remove(ObjectType *object);
    inline bool getHead(ObjectType *object);

private:
    void errorOverflow() {
    }

    void errorUnderflow() {
    }

    size_t increment(size_t index);

    ObjectType *data;
    size_t head;
    size_t tail;
    size_t size;
};

template<typename ObjectType, typename Lock> inline CyclicBufferDynamic<
        ObjectType, Lock>::CyclicBufferDynamic(size_t size, void *address) {

    this->head = 0;
    this->tail = 0;
    this->size = size;
    if (address != nullptr) {
        this->data = new (address) ObjectType[size];
    }
    else {
        this->data = new ObjectType[size];
    }

    static_assert(sizeof(ObjectType) <= sizeof(uintptr_t), "CyclicBuffer is intended to work only with integer types or pointers");
}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::isEmpty() {
    bool res = (this->head == this->tail);
    return res;
}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::isFull() {
    size_t tail = increment(this->tail);
    bool res = (this->head == tail);
    return res;
}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::add(ObjectType object) {
    Lock lock;
    if (!isFull()) {
        data[this->tail] = object;
        this->tail = increment(this->tail);
        return true;
    } else {
        errorOverflow();
        return false;
    }

}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::remove(ObjectType *object) {
    Lock lock;
    if (!isEmpty()) {
        *object = data[this->head];
        this->head = this->increment(this->head);
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}

template<typename ObjectType, typename Lock> inline bool CyclicBufferDynamic<
        ObjectType, Lock>::getHead(ObjectType *object) {
    Lock lock;
    if (!isEmpty()) {
        *object = data[this->head];
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}

template<typename ObjectType, typename Lock> size_t CyclicBufferDynamic<
ObjectType, Lock>::increment(size_t index) {
    if (index < this->size) {
        return (index + 1);
    } else {
        return 0;
    }
}

template<typename ObjectType, typename Lock, std::size_t Size> class CyclicBufferFast {
public:

    inline CyclicBufferFast();

    ~CyclicBufferFast() {
    }

    inline bool isEmpty();
    inline bool isFull();
    inline bool add(const ObjectType object);
    inline bool remove(ObjectType &object);
    inline bool getHead(ObjectType &object);

private:
    void errorOverflow() {
    }

    void errorUnderflow() {
    }

    inline ObjectType *increment(ObjectType *entry);

    ObjectType data[Size + 1];
    ObjectType *head;
    ObjectType *tail;
};

template<typename ObjectType, typename Lock, std::size_t Size> inline CyclicBufferFast<
        ObjectType, Lock, Size>::CyclicBufferFast() {

    this->head = &data[0];
    this->tail = &data[0];
    static_assert(sizeof(ObjectType) <= sizeof(uintptr_t), "CyclicBuffer is intended to work only with integer types or pointers");
}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBufferFast<
        ObjectType, Lock, Size>::isEmpty() {
    bool res = (this->head == this->tail);
    return res;
}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBufferFast<
        ObjectType, Lock, Size>::isFull() {
    ObjectType *tail = increment(this->tail);
    bool res = (this->head == tail);
    return res;
}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBufferFast<
        ObjectType, Lock, Size>::add(const ObjectType object) {
    Lock lock;
    if (!isFull()) {
        *this->tail = object;
        this->tail = increment(this->tail);
        return true;
    } else {
        errorOverflow();
        return false;
    }

}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBufferFast<
        ObjectType, Lock, Size>::remove(ObjectType &object) {
    Lock lock;
    if (!isEmpty()) {
        object = *(this->head);
        this->head = this->increment(this->head);
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}

template<typename ObjectType, typename Lock, std::size_t Size> inline bool CyclicBufferFast<
        ObjectType, Lock, Size>::getHead(ObjectType &object) {
    Lock lock;
    if (!isEmpty()) {
        object = *(this->head);
        return true;
    } else {
        errorUnderflow();
        return false;
    }
}

template<typename ObjectType, typename Lock, std::size_t Size> ObjectType *CyclicBufferFast<
ObjectType, Lock, Size>::increment(ObjectType *entry) {
    if (entry < &data[Size]) {
        return (entry + 1);
    } else {
        return &data[0];
    }
}
