/**
 * Plain vanilla intrusive double linked list
 */

#pragma once

#include "ObjectRegistry.h"

/**
 * I am keeping an array of objects of this type for debug purposes
 * All linked lisst in the system are supposed to inherit this class and call
 * registerTable() API
 */
class LinkedListBase : ObjectRegistry<LinkedListBase*, 32>
{
public:

    struct Statistics
    {
        uint64_t insertTotal;
        uint64_t removeTotal;
    };

    /**
     * Size of the list
     */
    uint_fast32_t getCount() const
    {
        return count;
    }

    bool isEmpty() const
    {
        return (getCount() == 0);
    }

    const struct Statistics *getStatistics() const
    {
        return &statistics;
    }

    void resetStatistics()
    {
        memset(&statistics, 0, sizeof(statistics));
    }


protected:
    const char *name;
    uint_fast32_t count;
    Statistics statistics;

    LinkedListBase(const char *name)
        : name(name), scount(0)
    {
        addRegistration(this);
        resetStatistics();
    }

    ~LinkedListBase()
    {
        removeRegistration(this);
    }
};

/**
 * This class contains the core logic for the linked list
 *
 */
template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
class LinkedList: public LinkedListBase
{
public:

    enum InsertResult
    {
        INSERT_DONE,
        INSERT_COLLISION,
        INSERT_DUPLICATE,
        INSERT_FAILED
    };

};
