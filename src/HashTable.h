#pragma once

/**
 * I am keeping an array of objects of this type for debug purposes
 * All hash tables in the system are supposed to inherit this class and call
 * registerTable() API
 */
class HashTableBase
{
public:

    struct Statistics
    {
        uint64_t insertTotal;
        uint64_t insertHashCollision;
        uint64_t insertDuplicate;
        uint64_t insertHashMaxCollision;
        uint64_t insertMaxSearch;
        uint64_t insertOk;
        uint64_t insertFailed;

        uint64_t searchTotal;
        uint64_t searchOk;
        uint64_t searchFailed;

        uint64_t removeTotal;
        uint64_t removeOk;
        uint64_t removeCollision;
        uint64_t removeFailed;
    };

    uint_fast32_t getSize() const
    {
        return size;
    }

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

    /**
     * Maximum number of hash tables objects accessible by debug
     */
    static const int HASH_TABLES_COUNT = 32;

    /**
     * List of all hash tables created in the system. I am not protecting access
     * to this global variable with a mutex. It is not crucial for execution
     * and should be modified only on power up
     * Check this object to see the debug information in one convenient place
     */
    static HashTableBase *HashTables[HASH_TABLES_COUNT];

protected:
    const char *name;
    uint_fast32_t size;
    uint_fast32_t count;
    Statistics statistics;

    HashTableBase() : count(0)
    {
        registerTable(this);
        resetStatistics();
    }

    ~HashTableBase()
    {
        unregisterTable(this);
    }

    static void registerTable(HashTableBase *hashTable)
    {
        for (int i = 0; i < HASH_TABLES_COUNT; i++)
        {
            if (HashTables[i] == nullptr)
            {
                HashTables[i] = hashTable;
                break;
            }
        }
    }

    static void unregisterTable(HashTableBase *hashTable)
    {
        for (int i = 0; i < HASH_TABLES_COUNT; i++)
        {
            if (HashTables[i] == hashTable)
            {
                HashTables[i] = nullptr;
            }
        }
    }

};

/**
 * This class contains the core logic for the hash table
 *
 * Object is a type of the objects (pointers or integral types) stored in the hash table,
 * Class Object shall implement static methods equal(const Object&, const Object&), hash(const Key &key).
 * The API will fail compilation for objects larger than the system pointer size. The application
 * is supposed to insert pointers to objects and not the objects themselves, The idea is that
 * correct and efficient deep copy of an object requires a copy constructor. This would prevent
 * using of the hash table for simple C structures.
 *
 * A memory allocator is a template argument. The hash table requires that the Allocator implements
 * static methods alloc() and free(). For example, AllocatorTrivial
 *
 * Mutual exclusion is a template argument. The hash table calls constructor of the Lock() class
 * for the critical sections. This is tricky - see example of a dummy lock further in this file.
 * If you do not need mutual exclusion you can use LockDummy class.
 *
 * Key is a type of the key. An example of a key type is PCWCHAR. Hash table hashes the key and the
 * result is used as an index in the hash table. The hash table can not store two objects with the
 * same key. If a hash collision happens the insert/search APIs will try next entry in the hash table.
 * The assumption is that collisions are very rare. For example, hash function for file paths can be
 * tested and, if necessary, tuned, in the initialization time.
 *
 * Following assumptions:
 * - There are not many different memory allocators in the system. It is possible to implement an
 * allocator as a classes with static methods
 * - Hash table should be allocated dynamically in the initialization time. No statically allocated
 * objects are allowed.
 * - There not many different mutex objects and they can be implemented as a constructor/destructor
 * - Hash function is good and the hash table is large enough to avoid collisions of the hash
 *
 * Example of usage:
 *
 *   struct MyHashObject
 *   {
 *       MyHashObject(const char *name);
 *       static bool equal(const char *s1, const char *s2);
 *       static const char* getKey(const struct MyHashObject &object);
 *       static const uint_fast32_t hash(const char *s);
 *   };
 *   typedef HashTable<struct MyHashObject, const char*, LockDummy, AllocatorTrivial> MyHashTable;
 *   MyHashTable *hashTable = MyHashTable::create("myHashTable", 3);
 *   MyHashObject o1("o1");
 *   hashTable->insert(o1.getKey(o1), o1);
 *   MyHashTable::destroy(hashTable);
 *
 */
template<typename Object, typename Key, typename Lock, typename Allocator> class HashTable: HashTableBase
{
public:

    enum InsertResult
    {
        INSERT_DONE,
        INSERT_COLLISION,
        INSERT_DUPLICATE,
        INSERT_FAILED
    };

    /**
     * Add a new entry to the hash table.
     * The function can fail if a collision happens and simple linear search fails as well
     * If the function fails often the application is expected to call rehash for a larger
     * table/different hash function
     */
    enum InsertResult insert(const Key &key, const Object &object)
    {
        InsertResult insertResult = insert(key, object, this->table, getSize(), &statistics, &count);
        return insertResult;
    }

    bool remove(const Key &key);

    bool search(const Key &key, Object &object) const;

    /**
     * Call the function if size/count ratio is below 2
     * or you are getting collisions often or you tune the hash function
     * in run-time. The function will allocate space for the
     * hash table, rehash the objects and copy the data to the new table.
     * If you are planning to use the function often it makes sense to
     * cache the hash of the object or use very fast hash function
     * @param size - new size of the hash table
     */
    bool rehash(const uint_fast32_t size);

    bool rehash()
    {
        bool result = rehash(this->getSize());
        return result;
    }

    /**
     * Hash tables can be allocated in different types of memory. For example paged memory,
     * non paged memory, pined in cache, etc.
     * It is impossible to declare a static object of type HashTable. All objects
     * should be allocated dynamically using create() API.
     * @param name - used for debug to distinguish between objects of the same hash table class
     * @param size - initial size of the table
     */
    static HashTable *create(const char *name, uint_fast32_t size)
    {
        void *hashTableMemory = Allocator::alloc(sizeof(HashTable));
        HashTable *hashTable = new (hashTableMemory) HashTable(name, size);
        return hashTable;
    }

    /**
     * Because the hash table is created using a placement operator new[]
     * I need function destroy which takes care of the cleanup
     */
    static void destroy(HashTable *hashTable)
    {
        hashTable->~HashTable();
        Allocator::free((void *)hashTable);
    }

protected:

    static const int MAX_COLLISIONS = 3;

    static uint_fast32_t getIndex(const Key &key, uint_fast32_t size)
    {
        uint_fast32_t hash = Object::hash(key);
        uint_fast32_t index = hash % size;
        return index;
    }

    /**
     * Dynamic allocation of the hash table
     */
    HashTable(const char *name, uint_fast32_t size)
    {
        this->name = name;
        this->size = size;
        resetStatistics();
        table = allocateTable(size);
    }

    ~HashTable()
    {
        freeTable(table);
    }

    static uint_fast32_t getAllocatedSize(uint_fast32_t size)
    {
        return size + MAX_COLLISIONS;
    }

    typedef Object *TableEntry;
    typedef TableEntry *Table;

    static Table allocateTable(uint_fast32_t size)
    {
        Table table = (Table)Allocator::alloc(sizeof(Object*) * getAllocatedSize(size));
        return table;
    }

    static void freeTable(Table table)
    {
        Allocator::free((void*)table);
    }

    static enum InsertResult insert(const Key &key, const Object &object, Table table, uint_fast32_t size, Statistics *statistics, uint_fast32_t *count);

    Table table;
};

template<typename Object, typename Key, typename Lock, typename Allocator>
enum HashTable<Object, Key, Lock, Allocator>::InsertResult
HashTable<Object, Key, Lock, Allocator>::insert(const Key &key, const Object &object, Table table, uint_fast32_t size, Statistics *statistics, uint_fast32_t *count)
{
    InsertResult insertResult = INSERT_FAILED;
    bool result = false;

    Lock lock;

    statistics->insertTotal++;
    uint_fast32_t index = getIndex(key, size);
    Object *storedObject = table[index];
    result = (storedObject == nullptr);
    if (!result)
    {
        insertResult = INSERT_COLLISION;
        for (int collisions = 1;collisions < MAX_COLLISIONS;collisions++)
        {
            statistics->insertHashCollision++;
            storedObject++;                   // I can do this - table contains (size+MAX_COLLISIONS) entries
            if (storedObject == nullptr)
            {
                result = true;
                break;
            }

            result = Object::equal(Object::getKey(*storedObject), Object::getKey(*storedObject));
            if (result)
            {
                insertResult = INSERT_DUPLICATE;
                statistics->insertDuplicate++;
                result = false;
                break;
            }
        }
    }

    if (result)
    {
        insertResult = INSERT_DONE;
        *storedObject = object;
        (*count)++;
    }
    else
    {
        statistics->insertHashMaxCollision++;
    }

    return insertResult;
}

template<typename Object, typename Key, typename Lock, typename Allocator>
bool HashTable<Object, Key, Lock, Allocator>::remove(const Key &key)
{
    bool result = false;

    Lock lock;

    statistics.removeTotal++;
    uint_fast32_t index = getIndex(key);
    Object *storedObject = &this->table[index];
    for (int collisions = 0;collisions < MAX_COLLISIONS;collisions++)
    {
        if (*storedObject != nullptr)
        {
            result = Object::equal(storedObject->getKey(), key);
            if (result)
            {
                statistics.removeOk++;
                this->count--;
                *storedObject = nullptr;
                result = true;
            }
            else
            {
                statistics.removeCollision++;
            }
        }
        storedObject++;                   // I can do this - table contains (size+MAX_COLLISIONS) entries
    }

    if (!result)
    {
        statistics.removeFailed++;
    }


    return result;
}

template<typename Object, typename Key, typename Lock, typename Allocator>
bool HashTable<Object, Key, Lock, Allocator>::search(const Key &key, Object &object) const
{
    bool result = true;

    Lock lock;
    statistics.searchTotal++;

    uint_fast32_t index = getIndex(key);
    Object *storedObject = &this->table[index];
    for (int collisions = 0;collisions < MAX_COLLISIONS;collisions++)
    {
        if (*storedObject != nullptr)
        {
            result = Object::equal(storedObject->getKey(), key);
            if (result)
            {
                statistics.searchOk++;
                result = true;
            }
            else
            {
                statistics.removeCollision++;
            }
        }
        storedObject++;                   // I can do this - table contains (size+MAX_COLLISIONS) entries
    }

    if (!result)
    {
        statistics.searchFailed++;
    }

    return result;
}

template<typename Object, typename Key, typename Lock, typename Allocator>
bool HashTable<Object, Key, Lock, Allocator>::rehash(const uint_fast32_t size)
{
    bool result = true;
    Object *newTable = allocateTable(size);

    Lock lock;

    Object *object = &table[0];
    for (int i = 0;i < getAllocatedSize(getSize());i++)
    {
        if (object != nullptr)
        {
            const Key key = object->getKey();
            InsertResult insertResult = insert(key, object, newTable, size, statistics);
            result = (insertResult == INSERT_DONE) && result;
        }
        object++;
    }
    freeTable(this->table);
    this->table = newTable;
    this->size = size;

    return result;
}

/**
 * Bob Jenkins hash function
 * http://burtleburtle.net/bob/hash/doobs.html
 */
static uint_fast32_t one_at_a_time(uint8_t *key, uint_fast32_t len,
        uint_fast32_t seed = 0)
{
    uint_fast32_t hash = seed;

    for (uint_fast32_t i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

/**
 * A trivial allocator for testing
 */
class AllocatorTrivial
{
public:
    static void *alloc(uint_fast32_t size)
    {
        return new uint8_t[size];
    }

    static void free(void *ptr)
    {
        delete[] (uint8_t*)ptr;
    }
};

/**
 * A simple generic hashable object for testing purposes
 * Type Data can be uint32_t or a UNICODE_STRING
 */
template<typename Data, typename Key> class HashObject
{
public:

    static bool equal(const Key &key1, const Key &key2)
    {
        return (key1 == key2);
    }

    static const uint_fast32_t hash(const Key &key)
    {
        uint_fast32_t result = one_at_a_time(&key, sizeof(Key));
        return result;
    }

    static const Key getKey(HashObject object)
    {
        return object.key;
    }

    const uint_fast32_t getKeySize() const
    {
        return sizeof(Key);
    }


private:
    Data data;
    Key key;
};

/**
 * Simple synchronization object for testing
 */
class SynchroObjectDummy
{

    SynchroObjectDummy()
    {
    }
    ;

public:

    static inline void get()
    {
    }

    static inline void release()
    {
    }

};

/**
 * Simple lock object for testing purposes
 */
template<typename Mutex> class Lock
{

public:
    inline Lock()
    {
        Mutex::get();
    }

    inline ~Lock()
    {
        Mutex::release();
    }
};

/**
 * Instantiate a new type - lock which does nothing
 */
typedef Lock<SynchroObjectDummy> LockDummy;
