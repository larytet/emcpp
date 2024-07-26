/**
 * Following assumptions:
 * - There are not many different memory allocators in the system. It is possible to implement an
 * allocator as a class with static methods
 * - Hash table should be allocated dynamically in the initialization time. No statically allocated
 * objects are allowed. For example application can not create a hash table which is an automatic
 * variable in the function main().
 * - There not many different mutex objects. A mutex can be implemented as a constructor/destructor
 * - Hash function is good and the hash table is large enough to avoid (mostly) collisions of the hash
 *
 * Example of usage:
 *
 *   struct MyHashObject
 *   {
 *       static bool equal(struct MyHashObject *object, const char *name);
 *       static const char* getKey(const struct MyHashObject *object);
 *       static const uint_fast32_t hash(const char *name);
 *   };
 *
 *   typedef HashTable<struct MyHashObject*, const char*, LockDummy,
 *                      AllocatorTrivial, struct MyHashObject,
 *                      struct MyHashObject> MyHashTable;
 *   MyHashTable *hashTable = MyHashTable::create("myHashTable", 3);
 *   MyHashObject o1("o1");
 *   hashTable->insert(o1.getKey(&o1), &o1);
 *   uint_fast32_t index = 0;
 *   struct MyHashObject *pMyHashObject;
 *   while (hashTable->getNext(index, &pMyHashObject) != MyHashTable::GETNEXT_END_TABLE)
 *   {
 *       cout << MyHashObject::getKey(pMyHashObject) << endl;
 *       index++;
 *   }
 *
 *
 *   MyHashTable::destroy(hashTable);
 *
 */

#pragma once

#include "ObjectRegistry.h"

class HashTableBase;

/**
 * I am keeping an array of objects of this type for debug purposes
 * All hash tables in the system are supposed to inherit this class and call
 * registerTable() API
 */
class HashTableBase : ObjectRegistry<HashTableBase*, 32>
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
        uint64_t searchSkipCompare;

        uint64_t removeTotal;
        uint64_t removeOk;
        uint64_t removeCollision;
        uint64_t removeFailed;

        uint64_t rehashTotal;
        uint64_t rehashFailed;
        uint64_t rehashDone;
        uint64_t rehashCollision;
    };

    /**
     * Maximum number of elements that can be stored in the table
     */
    uint_fast32_t getSize() const
    {
        return size;
    }

    /**
     * Number of occupied entries in the table
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

    /**
     * After a number of inserts it makes sense to check the number of
     * collisions in the table. Large number of collisions will
     * impact search performance and can cause an insert to fail
     */
    const uint_fast32_t getCollisionsInTheTable() const
    {
        return collisionsInTheTable;
    }

    void resetStatistics()
    {
        memset(&statistics, 0, sizeof(statistics));
    }

    /**
     * Set a resize factor used by the insert with automatic resize.
     * Default value is 50(%). Factor zero (0%) means that insert()
     * will try to add one entry to the table size.
     * Trade off here is memory usage efficiency and number of iterations
     * of rehashing.
     */
    void setResizeFactor(uint_fast32_t factor)
    {
        this->resizeFactor = factor;
    }

protected:
    const char *name;
    uint_fast32_t size;
    uint_fast32_t count;
    Statistics statistics;
    uint_fast32_t collisionsInTheTable; // Number of collisions in the table
    uint_fast32_t resizeFactor;

    HashTableBase(const char *name)
        : name(name), size(0), count(0), resizeFactor(50)
    {
        addRegistration(this);
        resetStatistics();
    }

    ~HashTableBase()
    {
        removeRegistration(this);
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
 */
template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
class HashTable: public HashTableBase
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
     * Add a new entry (a pointer to) to the hash table.
     * The function can fail if a collision happens and simple linear search fails as well
     * If the function fails often the application is expected to call rehash for a larger
     * table/different hash function
     */
    enum InsertResult insert(const Key &key, const Object &object)
    {
        InsertResult insertResult = insert(key, object, this->table, getSize(), *this);
        return insertResult;
    }


    /**
     * Insert with automatic call to rehash if the space is running low. Tries to avoid
     * collisions at cost of larger table. See setResizeFactor()
     *
     * @param maxSize - maximum size for the table
     */
    enum InsertResult insert(const Key &key, const Object &object, uint_fast32_t maxSize);

    bool remove(const Key &key);

    void removeAll();

    /**
     * Get a stored pointer from the hash table
     * @param skipKeyCompare - set to true to save CPU cycles. Works well if the hash table
     * does not have collisions (this->collisionsInTheTable == 0) and comparison is not a
     * trivial function
     */
    bool search(const Key &key, Object *object, bool skipKeyCompare=false);

    /**
     * The function is not safe. Making the table smaller can cause dropping
     * of stored elements
     *
     * Call the function if size/count ratio is below 2
     * or you are getting collisions often or you tune the hash function
     * in run-time. The function will allocate space for the
     * hash table, rehash the objects and copy the data to the new table.
     * If you are planning to use the function often it makes sense to
     * cache the hash of the object or use very fast hash function
     *
     * The function copies the data from an old table to a newly allocated
     * table. It means that for the time of resizing there are two
     * tables are allocated in the same time. The tables keep only
     * pointers. Even a table which contains lot of elements has relatively
     * small footprint. Typical size/count ratio is between 3 and 5
     *
     * @param size - new size of the hash table
     */
    enum InsertResult rehash(const uint_fast32_t size);

    /**
     * Safe version of the rehash() - copy data from the table "src"
     * to the table "dst"
     */
    static enum InsertResult rehash(const HashTable *src, HashTable *dst);

    enum InsertResult rehash()
    {
        enum InsertResult result = rehash(this->getSize());
        return result;
    }

    enum GetNextResult
    {
        GETNEXT_FAILED,
        GETNEXT_OK,
        GETNEXT_END_TABLE
    };

    /**
     * Access the objects stored in the table. The elements in the hash table are not
     * stored in any particular order. See also setIllegalValue()
     *
     * Performance of the function depends on the number of removed elements (empty entry)
     * in the table. Skipping an empty entry takes time - branch, cache miss, etc.
     * If the table is empty the function will run through all the elements before it
     * returns GETNEXT_FAILED
     *
     * @param index - use zero to get the first stored object
     * @param object - the next stored object if getNext returns Ok
     */
    enum GetNextResult getNext(uint_fast32_t &index, Object *object) const;


    /**
     * By default the table assumes that nullptr means that the entry is not
     * occupied
     * Frequently used values are -1, zero, nullptr
     */
    void setIllegalValue(const Object value)
    {
        this->illegalValue = value;
    }

    /**
     * Hash tables can be allocated in different types of memory. For example paged memory,
     * non paged memory, in cache, etc.
     * It is impossible to declare a static object of type HashTable. All objects
     * should be allocated dynamically using create() API.
     * @param name - used for debug to distinguish between objects of the same hash table class
     * @param size - initial size of the table
     */
    static HashTable *create(const char *name, uint_fast32_t size)
    {
        Table table = allocateTable(size);
        if (table == nullptr)
        {
            return nullptr;
        }
        void *hashTableMemory = Allocator::alloc(sizeof(HashTable));
        if (hashTableMemory == nullptr)
        {
            Allocator::free((void *)table);
            return nullptr;
        }
        HashTable *hashTable = new (hashTableMemory) HashTable(name, size, table);
        return hashTable;
    }

    /**
     * Because the hash table is created using a placement operator new[]
     * I need function destroy which takes care of the cleanup
     */
    static void destroy(HashTable *hashTable)
    {
        hashTable->~HashTable();
        freeTable(hashTable->table);
        Allocator::free((void *)hashTable);
    }

protected:

    static const int MAX_COLLISIONS = 3;

    static uint_fast32_t getIndex(const Key &key, uint_fast32_t size)
    {
        uint_fast32_t hash = Hash::hash(key);
        uint_fast32_t index = hash % size;
        return index;
    }

    typedef Object TableEntry;
    typedef TableEntry *Table;

    HashTable(const char *name, uint_fast32_t size, Table table) : HashTableBase(name)
    {
        static_assert(sizeof(Object) <= sizeof(uintptr_t), "HashTable is intended to work only with integral types or pointers");
        this->name = name;
        this->size = size;
        this->table = table;
        this->illegalValue = nullptr;
        resetStatistics();
    }

    ~HashTable()
    {
    }

    static uint_fast32_t getAllocatedSize(uint_fast32_t size)
    {
        return size + MAX_COLLISIONS;
    }

    static Table allocateTable(uint_fast32_t size)
    {
        uint_fast32_t bytes = getAllocatedSize(size) * sizeof(TableEntry);
        Table table = (Table)Allocator::alloc(bytes);
        memset(table, 0, bytes);
        return table;
    }

    static void freeTable(Table table)
    {
        Allocator::free((void*)table);
    }

    static uint_fast32_t applyResizeFactor(uint_fast32_t size, uint_fast32_t maxSize, uint_fast32_t resizeFactor);

    static enum InsertResult insert(const Key &key, const Object &object,
            Table table, uint_fast32_t size,
            HashTable &hashTable);

    Table table;
    TableEntry illegalValue;
};


template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
uint_fast32_t
HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::applyResizeFactor(uint_fast32_t size, uint_fast32_t maxSize, uint_fast32_t resizeFactor)
{
    uint_fast32_t newSize = (size * (100+resizeFactor))/100;
    if (newSize == size)
    {
        newSize++;
    }
    if (newSize > maxSize)
    {
        newSize = maxSize;
    }
    return newSize;
}


template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
enum HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::InsertResult
HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::insert(const Key &key, const Object &object,
        uint_fast32_t maxSize)
{
    InsertResult insertResult;
    bool inserted = false;
    /**
     * If a table does not have collisions it does not mean that I can insert an entry.
     * I will try to insert an entry first and check the return code.
     * Resize the table if needed. Ensure that after resizing there is no collisions in
     * the table.
     */
    do
    {
        if (!inserted)
        {
            insertResult = insert(key, object);
            inserted = (insertResult == INSERT_DONE);
        }
        if (this->collisionsInTheTable > 0)
        {
            if (getSize() < maxSize)
            {
                uint_fast32_t newSize = applyResizeFactor(getSize(), maxSize, this->resizeFactor);
                InsertResult rehashResult = rehash(newSize);
                if (rehashResult != INSERT_DONE)
                {
                    insertResult = rehashResult;
                }
                if (rehashResult == INSERT_FAILED)  // something very wrong, allocation problems?
                {
                    break;
                }
            }
            else // Reached maximum size
            {
                break;
            }
        }
    }
    while ((insertResult != INSERT_DONE) || (this->collisionsInTheTable > 0));

    return insertResult;
}


template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
enum HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::InsertResult
HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::insert(const Key &key, const Object &object,
        Table table, uint_fast32_t size,
        HashTable &hashTable)
{
    InsertResult insertResult = INSERT_FAILED;

    // The hash table size is a variable and not a constant - another complication
    // There is a modulus inside the method
    uint_fast32_t index = getIndex(key, size);
    Statistics *statistics = &hashTable.statistics;
    TableEntry *tableEntry = &table[index];

    Lock lock;
    statistics->insertTotal++;

    // The following code is driven by necessity to release the lock before return from the
    // function. I want a single return point
    bool result = (*tableEntry == hashTable.illegalValue);
    if (!result)  // this is not a likely outcome
    {
        insertResult = INSERT_COLLISION;
        tableEntry++;
        const TableEntry *tableEntryLast = &table[index+MAX_COLLISIONS];
        for (;tableEntry <= tableEntryLast;tableEntry++)
        {
            statistics->insertHashCollision++;
            hashTable.collisionsInTheTable++;
            if (*tableEntry == hashTable.illegalValue)
            {
                result = true;
                break;
            }

            result = Comparator::equal(*tableEntry, key);
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
        *tableEntry = object;
        hashTable.count++;
    }
    else
    {
        statistics->insertHashMaxCollision++;
    }

    return insertResult;
}

template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
void HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::removeAll()
{
    Lock lock;

    uint_fast32_t bytes = getAllocatedSize(size) * sizeof(TableEntry);
    memset(table, 0, bytes);
    this->count = 0;
    this->collisionsInTheTable = 0;
}

template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
bool HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::remove(const Key &key)
{
    bool result = false;

    Lock lock;

    statistics.removeTotal++;
    uint_fast32_t index = getIndex(key, getSize());
    TableEntry *tableEntry = &this->table[index];
    for (int collisions = 0;collisions < MAX_COLLISIONS;collisions++)
    {
        if (*tableEntry != this->illegalValue)
        {
            result = Comparator::equal(*tableEntry, key);
            if (result)
            {
                statistics.removeOk++;
                this->count--;
                *tableEntry = this->illegalValue;
                result = true;
                this->collisionsInTheTable -= collisions;
            }
            else
            {
                statistics.removeCollision++;
            }
        }
        tableEntry++;                   // I can do this - table contains (size+MAX_COLLISIONS) entries
    }

    if (!result)
    {
        statistics.removeFailed++;
    }


    return result;
}

template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
bool HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::search(const Key &key, Object *object, bool skipKeyCompare)
{
    bool result = false;

    Lock lock;
    statistics.searchTotal++;

    uint_fast32_t index = getIndex(key, getSize());
    TableEntry *tableEntry = &this->table[index];
    for (int collisions = 0;collisions < MAX_COLLISIONS;collisions++)
    {
        if (*tableEntry != this->illegalValue)
        {
            result = skipKeyCompare;
            if (!result)
                result = Comparator::equal(*tableEntry, key);
            else
                statistics.searchSkipCompare++;
            if (result)
            {
                statistics.searchOk++;
                *object = *tableEntry;
                result = true;
                break;
            }
            else
            {
                statistics.removeCollision++;
            }
        }
        tableEntry++;                   // I can do this - table contains (size+MAX_COLLISIONS) entries
    }

    if (!result)
    {
        statistics.searchFailed++;
    }

    return result;
}

template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
enum HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::GetNextResult
HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::getNext(uint_fast32_t &index, Object *object) const
{
    enum GetNextResult result = GETNEXT_END_TABLE;
    TableEntry *tableEntry = &table[index];
    for (uint_fast32_t i = index;i < getAllocatedSize(getSize());i++)
    {
        if (*tableEntry != this->illegalValue)
        {
            *object = *tableEntry;
            index = i;
            result = GETNEXT_OK;
            break;
        }
        tableEntry++;
    }
    return result;
}

template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
enum HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::InsertResult
HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::rehash(const HashTable *src, HashTable *dst)
{
    enum InsertResult rehashResult = INSERT_DONE;
    uint_fast32_t index = 0;
    TableEntry entry;
    while (src->getNext(index, &entry) != HashTable::GETNEXT_END_TABLE)
    {
        enum InsertResult insertResult = dst->insert(Hash::getKey(entry), entry);
        if (insertResult != INSERT_DONE)
        {
            rehashResult = INSERT_FAILED;
            break;
        }
        index++;
    }
    return rehashResult;
}

template<typename Object, typename Key, typename Lock, typename Allocator, typename Hash, typename Comparator>
enum HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::InsertResult
HashTable<Object, Key, Lock, Allocator, Hash, Comparator>::rehash(const uint_fast32_t size)
{
    enum InsertResult rehashResult = INSERT_DONE;
    Object *newTable = allocateTable(size);
    if (newTable == nullptr)
    {
        Lock lock;
        statistics.rehashTotal++;
        statistics.rehashFailed++;
        return INSERT_FAILED;
    }

    Lock lock;
    statistics.rehashTotal++;

    this->collisionsInTheTable = 0;
    this->count = 0;
    TableEntry *tableEntry = &table[0];
    for (int i = 0;i < getAllocatedSize(getSize());i++)
    {
        if (*tableEntry != this->illegalValue)
        {
            const Key &key = Hash::getKey(*tableEntry);
            InsertResult insertResult = insert(key, *tableEntry, newTable, size, *this);
            if (insertResult != INSERT_DONE)
            {
                rehashResult = INSERT_FAILED;
                statistics.rehashCollision++;
            }
            else
                statistics.rehashDone++;
        }
        tableEntry++;
    }
    freeTable(this->table);
    this->table = newTable;
    this->size = size;

    return rehashResult;
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

    static bool equal(HashObject *object, const Key key)
    {
        return (key == object->getKey(object));
    }

    static const uint_fast32_t hash(const Key &key)
    {
        uint_fast32_t result = one_at_a_time(&key, sizeof(Key));
        return result;
    }

    static const Key &getKey(HashObject *object)
    {
        return object->key;
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


/**
 * GCC atomic builtins
 */
#define hashtable_likely(x)      __builtin_expect(!!(x), 1)   // !!(x) will return 1 for any x != 0
#define hashtable_unlikely(x)    __builtin_expect(!!(x), 0)
#define hashtable_cmpxchg(key, val, new_val) __sync_val_compare_and_swap(key, val, new_val)
#define hashtable_barrier() __sync_synchronize()

/**
 * Saves some typing
 */
#define hashtable_sync_access(x) (*(volatile __typeof__(*x) *) (x))

/**
 * Implementation of lockfree linear probing hashtable
 * The hashtable targets scenarios where a typical key is 32 or 64 bits variable
 * The number of probes is limited by a constant. The index is not wrapping around,
 * but instead the hashtable allocates enough memory to handle linear probing in the end
 * of the table
 *
 * Limitation: a specific entry (a specific key) can be inserted and deleted by one thread.
 *
 * Performance: a core can make above 30M add&remove operations per second, cost of a
 * single operation is under 20 nanos which is an equivalent of 50-100 opcodes.
 */
#define LockfreeHashTableTemplateTypes typename Object, Object IllegalData, typename Key, Key IllegalKey, typename Allocator, typename Hash
#define LockfreeHashTableTemplateArgs Object, IllegalData, Key, IllegalKey, Allocator, Hash

template<LockfreeHashTableTemplateTypes>
class LockfreeHashTable: public HashTableBase
{
public:
    enum InsertResult
    {
        INSERT_DONE,
        INSERT_COLLISION,
        INSERT_DUPLICATE,
        INSERT_FAILED
    };

    static LockfreeHashTable* create(const char *name, int sizeBits)
    {
        size_t sizeEntries = (1 << sizeBits);
        size_t sizeBytes = memorySize(sizeBits);
        TableEntry* table = (TableEntry*)Allocator::alloc(sizeBytes);
        if (!table)
        {
        	return nullptr;
        }
        for (size_t i = 0;i < sizeEntries+MAX_COLLISIONS;i++)
        {
            initSlot(&table[i]);
        }

        void *locklessHashTableMemory = Allocator::alloc(sizeof(LockfreeHashTable));
        if (locklessHashTableMemory == nullptr)
        {
            Allocator::free((void *)table);
            return nullptr;
        }
        LockfreeHashTable *locklessHashTable = new (locklessHashTableMemory) LockfreeHashTable(name, sizeBits, table);
        return locklessHashTable;
    }

    /**
     * Because the hash table is created using a placement operator new[]
     * I need function destroy which takes care of the cleanup
     */
    static void destroy(LockfreeHashTable *hashTable)
    {
        hashTable->~LockfreeHashTable();
        Allocator::free(hashTable->table);
        Allocator::free((void *)hashTable);
    }

    ~LockfreeHashTable()
    {
    }

    InsertResult insert(Key key, const Object &o);
    bool remove(Key key, Object *o);
    bool search(Key key, Object *o);

protected:

    static const int MAX_COLLISIONS = 3;

    typedef struct
    {
        volatile Key key;
        Object data;
    } TableEntry;

    static void initSlot(TableEntry *entry)
    {
    	entry->key = IllegalKey;
    	entry->data = IllegalData;
    }

    static size_t memorySize(const int bits)
    {
        size_t entries = (1 << bits) + MAX_COLLISIONS;
        return (sizeof(TableEntry) * entries);
    }

    LockfreeHashTable(const char *name, int sizeBits, TableEntry* table) : HashTableBase(name)
	{
        sizeEntries = (1 << sizeBits);
        sizeBytes = memorySize(sizeBits);
        this->table = table;
        size = sizeEntries + MAX_COLLISIONS;
	}

    inline size_t getIndex( const uint32_t hash)
    {
        return hash & (sizeEntries - 1);
    }

    size_t sizeEntries;
    size_t sizeBytes;
    TableEntry *table;
};


/**
 * Hash the key, get an index in the hashtable, try compare-and-set.
 * If fails (not likely) try again with the next slot (linear probing)
 * continue until success or max_tries is hit
 */
template<LockfreeHashTableTemplateTypes>
enum LockfreeHashTable<LockfreeHashTableTemplateArgs>::InsertResult
LockfreeHashTable<LockfreeHashTableTemplateArgs>::insert(Key key, const Object &o)
{
    const uint_fast32_t hash = Hash::hash(key);
	const uint_fast32_t index = getIndex(hash);
	/* I can do this for the last slot too - I allocated max_tries more slots */
	const uint_fast32_t indexMax = index+MAX_COLLISIONS;
    statistics.insertTotal++;
	for (TableEntry *entry = &table[index];entry < &table[indexMax];entry++)
	{
	    Key oldKey = hashtable_cmpxchg(&entry->key, IllegalKey, key);
	    if (hashtable_likely(oldKey == IllegalKey)) /* Success */
	    {
	    	entry->data = o;
	    	statistics.insertOk++;
	    	this->count++;
	        return INSERT_DONE;
	    }
	    else if (oldKey == key)
		{
	    	entry->data = o;
	    	statistics.insertDuplicate++;
	        return INSERT_DUPLICATE;
		}
	    else
	    {
	    	statistics.insertHashCollision++;
	    }
	}

	statistics.insertFailed++;
	return INSERT_FAILED;
}


/**
 * Hash the key, get an index in the hashtable, find the relevant entry,
 * read the pointer, remove using atomic operation
 * Only one context is allowed to remove a specific entry
 */
template<LockfreeHashTableTemplateTypes>
bool
LockfreeHashTable<LockfreeHashTableTemplateArgs>::remove(Key key, Object *o)
{
    const uint_fast32_t hash = Hash::hash(key);
	const uint_fast32_t index = getIndex(hash);
	/* I can do this for the last slot too - I allocated max_tries more slots */
	const uint_fast32_t indexMax = index+MAX_COLLISIONS;
    statistics.removeTotal++;
	for (TableEntry *entry = &table[index];entry < &table[indexMax];entry++)
	{
	    Key oldKey = entry->key;
	    if (hashtable_unlikely(oldKey == key))
	    {
	        if (o)
	        {
	            *o = entry->data;
	        }
	        hashtable_sync_access(&entry->data) = IllegalData;
	        hashtable_barrier();
	        hashtable_sync_access(&entry->key) = IllegalKey;
	    	this->count--;
	        return true;
	    }
	}

    statistics.removeFailed++;
	return false;
}

/**
 * Hash the key, get an index in the hashtable, find the relevant entry,
 * read the pointer
 */
template<LockfreeHashTableTemplateTypes>
bool
LockfreeHashTable<LockfreeHashTableTemplateArgs>::search(Key key, Object *o)
{
    const uint_fast32_t hash = Hash::hash(key);
	const uint_fast32_t index = getIndex(hash);
	/* I can do this for the last slot too - I allocated max_tries more slots */
	const uint_fast32_t indexMax = index+MAX_COLLISIONS;
    statistics.searchTotal++;
	for (TableEntry *entry = &table[index];entry < &table[indexMax];entry++)
	{
	    Key oldKey = entry->key;
	    if (hashtable_likely(oldKey == key))
	    {
	        if (o)
	        {
	            *o = entry->data;
	        }
	        statistics.searchOk++;
	        return true;
	    }
	}

    statistics.searchFailed++;
	return false;
}

struct HashTrivial
{
    static const uint_fast32_t hash(uint32_t key)
    {
        return key;
    }
};
