/**
 * Plain vanilla intrusive double linked list
 */

#pragma once

/**
 * Maximum number of LinkedList objects accessible by debug
 */
static const int LINKED_LISTS_COUNT = 32;
class LinkedListBase;

/**
 * List of all hash tables created in the system. I am not protecting access
 * to this global variable with a mutex. It is not crucial for execution
 * and should be modified only on power up
 * Check this object to see the debug information in one convenient place
 */
static LinkedListBase *LinkedLists[LINKE_LISTS_COUNT];

/**
 * I am keeping an array of objects of this type for debug purposes
 * All linked lisst in the system are supposed to inherit this class and call
 * registerTable() API
 */
class LinkedListBase
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
        registerTable(this);
        resetStatistics();
    }

    ~LinkedListBase()
    {
        unregisterTable(this);
    }

    static void registerList(LinkedListBase *linkedList)
    {
        for (int i = 0; i < LINKED_LISTS_COUNT; i++)
        {
            if (LinkedLists[i] == nullptr)
            {
                LinkedLists[i] = linkedList;
                break;
            }
        }
    }

    static void unregisterList(LinkedListBase *linkedList)
    {
        for (int i = 0; i < LINKED_LISTS_COUNT; i++)
        {
            if (LinkedLists[i] == linkedList)
            {
                LinkedLists[i] = nullptr;
            }
        }
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
