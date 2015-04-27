#pragma once

class Container {

public:
    bool isEmpty() {
        bool res = (this->head == this->tail);
        return res;
    }

    bool isFull() {
        size_t tail = increment(this->tail);
        bool res = (this->head == tail);
        return res;
    }

protected:
    Container(size_t size) {
        this->size = size;
        this->head = 0;
        this->tail = 0;
    }

    void errorOverflow() {
    }

    void errorUnderflow() {
    }

    size_t increment(size_t index) {
        if (index < this->size) {
            return (index + 1);
        } else {
            return 0;
        }
    }

    size_t head;
    size_t tail;
    size_t size;

};// Container
