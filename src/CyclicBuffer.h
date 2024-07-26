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
        CyclicBuffer& cyclicBuffer;
        size_t index;
    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef ObjectType* pointer;
        typedef ObjectType& reference;
        typedef size_t difference_type;
        typedef ObjectType value_type;

        inline iterator(const iterator& iter);
        inline iterator(CyclicBuffer& cyclicBuffer, size_t index);
        inline bool operator==(const iterator& iter) const;
        inline bool operator>=(const iterator& iter) const;
        inline bool operator<(const iterator& iter) const;
        inline bool operator>(const iterator& iter) const;
        inline bool operator!=(const iterator& iter) const;
        inline size_t operator-(const iterator& iter) const;
        inline iterator& operator=(const iterator& iter);
        inline iterator& operator++();
        inline iterator operator++(int);
        inline iterator& operator--();
        inline iterator operator--(int);
        inline ObjectType& operator*();
        inline ObjectType& operator->();
        iterator operator+(size_t n);
        iterator &operator+=(size_t n);
        iterator operator-(size_t n);
        iterator &operator-=(size_t n);
    };
    inline iterator begin();
    inline iterator end();

private:

    void errorOverflow() {
    }

    void errorUnderflow() {
    }

    inline size_t increment(size_t index);
    inline size_t decrement(size_t index);
    inline size_t increment(size_t index, size_t value);
    inline size_t decrement(size_t index, size_t value);

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

template<typename ObjectType, typename Lock, std::size_t Size> size_t CyclicBuffer<
ObjectType, Lock, Size>::decrement(size_t index) {
    if (index > 0) {
        return (index - 1);
    } else {
        return Size - 1;
    }
}

template<typename ObjectType, typename Lock, std::size_t Size> size_t CyclicBuffer<
ObjectType, Lock, Size>::increment(size_t index, size_t value) {
    index = (index + value) % Size;
    return index;
}

template<typename ObjectType, typename Lock, std::size_t Size> size_t CyclicBuffer<
ObjectType, Lock, Size>::decrement(size_t index, size_t value) {
    if (value < index) {
        index = index - value;
    } else {
        index = (value - index) % Size;
        index = Size - index;
    }

    return index;
}

template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBuffer<ObjectType, Lock, Size>::iterator::iterator(CyclicBuffer& cyclicBuffer, size_t index)
    : cyclicBuffer(cyclicBuffer), index(index) {
    cout << __LINE__ << endl;
}

template<typename ObjectType, typename Lock, std::size_t Size>
CyclicBuffer<ObjectType, Lock, Size>::iterator::iterator(const iterator& iter)
    : cyclicBuffer(iter.cyclicBuffer), index(iter.index) {
    cout << __LINE__ << endl;
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator&
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator=(const iterator& iter) {
    cout << __LINE__ << endl;
    this->index = iter.index;
    this->cyclicBuffer = iter.cyclicBuffer;
    return *this;
}

template<typename ObjectType, typename Lock, std::size_t Size>
bool
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator==(const iterator & iter) const {
    cout << __LINE__ << endl;
    return (iter.index == this->index);
}

template<typename ObjectType, typename Lock, std::size_t Size>
bool
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator>=(const iterator& iter) const {
    cout << __LINE__ << endl;
    return (iter.index >= this->index);
}

template<typename ObjectType, typename Lock, std::size_t Size>
bool
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator<(const iterator& iter) const {
    cout << __LINE__ << endl;
    return (iter.index < this->index);
}

template<typename ObjectType, typename Lock, std::size_t Size>
bool
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator>(const iterator& iter) const {
    cout << __LINE__ << endl;
    return (iter.index > this->index);
}

template<typename ObjectType, typename Lock, std::size_t Size>
bool
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator!=(const iterator & iter) const {
    cout << __LINE__ << " this=" << this->index << " iter.index" << iter.index << endl;
    return (iter.index != this->index);
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator&
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator++() {
    cout << __LINE__ << endl;
    this->index = cyclicBuffer.increment(this->index);
    return *this;
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator++(int) {
    cout << __LINE__ << endl;
    iterator temp(*this);
    temp.operator++();
    return temp;
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator&
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator--() {
    cout << __LINE__ << endl;
    this->index = cyclicBuffer.decrement(this->index);
    return *this;
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator--(int) {
    cout << __LINE__ << endl;
    iterator temp(*this);
    temp.operator--();
    return temp;
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator+(size_t n)
{
    cout << __LINE__ << " n=" << n << endl;
    iterator temp(*this);
    temp.index = cyclicBuffer.increment(temp.index, n);
    return temp;
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator&
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator+=(size_t n)
{
    cout << __LINE__ << endl;
    this->index = cyclicBuffer.increment(this->index, n);
    return *this;
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator-(size_t n)
{
    cout << __LINE__ << endl;
    iterator temp(*this);
    temp.index = cyclicBuffer.decrement(temp.index, n);
    return temp;
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator&
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator-=(size_t n)
{
    cout << __LINE__ << endl;
    this->index = cyclicBuffer.decrement(this->index, n);
    return *this;
}

template<typename ObjectType, typename Lock, std::size_t Size>
ObjectType & CyclicBuffer<ObjectType, Lock, Size>::iterator::operator*() {
    cout << __LINE__ << endl;
    return cyclicBuffer.data[index];
}

template<typename ObjectType, typename Lock, std::size_t Size>
ObjectType& CyclicBuffer<ObjectType, Lock, Size>::iterator::operator->() {
    cout << __LINE__ << endl;
    return cyclicBuffer.data[index];
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::begin() {
    cout << __LINE__ << " head=" << head << endl;
    return iterator(*this, head);
}

template<typename ObjectType, typename Lock, std::size_t Size>
typename CyclicBuffer<ObjectType, Lock, Size>::iterator
CyclicBuffer<ObjectType, Lock, Size>::end() {
    cout << __LINE__ << " tail=" << tail << endl;
    return iterator(*this, tail);
}

template<typename ObjectType, typename Lock, std::size_t Size>
size_t
CyclicBuffer<ObjectType, Lock, Size>::iterator::operator-(const iterator& iter) const {
    if (this->index >= iter.index) {
        cout << __LINE__ << " " << iter.index << " " << this->index << endl;
        return (this->index - iter.index);
    } else {
        cout << __LINE__ << " " << iter.index << " " << this->index << endl;
        return (Size + this->index - iter.index + 1);
    }
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
