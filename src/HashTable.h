#pragma once


class HashTableBase
{
public:

	struct Statistics
	{

	};

    uint_fast32_t getSize() const
    {
    	return size;
    }

    uint_fast32_t getCount() const;

    bool isEmpty() const
    {
    	return (getCount() == 0);
    }

    struct Statistics *getStatistics() const
    {
    	return &statistics;
    }

    void resetStatistics()
    {
    	memset(&statistics, 0, sizeof(statistics));
    }


protected:
    const char *name;
    uint_fast32_t size;
    Statistics statistics;
};

/**
 * Following assumptions:
 * - There are not many different memory allocators in the system. Allocators can be classes with static methods
 * - Hash table can be allocated dynamically in the initialization time
 */
template<typename Object, typename Key, typename Lock, typename Allocator> class HashTable : HashTableBase
{
public:

	struct Statistics
	{

	};


    ~HashTable()
    {
    	table = Allocator::free(table);
    }

    /**
     * Add a new entry to the hash table.
     * The function can fail if a collision happens and simple linear search fails as well
     */
    bool insert(const Key &key, const Object object);
    bool remove(const Key &key);
    bool find(const Key &key, Object &object) const;

    /**
     * Call the function if size/count ration is below two
     * or you are getting collisions often. Will allocate space for the
     * hash table, copy the data to the new table.
     * @param size - new size of the hash table
     */
    bool rehash(const uint_fast32_t size);

    /**
     * Hash tables can be allocated in different types of memory. For example paged memory,
     * non paged memory, pined in cache, etc.
     * It is impossible to declare a static object of type HashTable. All objects
     * should be allocated dynamically using create() API.
     */
    static HashTable *create(const char *name, uint_fast32_t size)
    {
    	void *hashTableMemory = Allocator::alloc(sizeof(HashTable));
    	HashTable *hashTable = new(hashTableMemory) HashTable(name, size);
    	return hashTable;
    }

    static void destroy(HashTable *hashTable)
    {
    	hashTable->~HashTable();
    	Allocator::free(hashTable);
    }

private:

	/**
	 * Dynamic allocation of the hash table
	 */
    HashTable(const char *name, uint_fast32_t size) :
    	name(name),
		size(size)
    {
        static_assert(sizeof(Object) <= sizeof(uintptr_t), "HashTable is intended to work only with integral types or pointers");
    	resetStatistics();
    	table = Allocator::alloc(sizeof(Object)*size);
    }

    Object *table;
};

#if 0
template<typename Object, typename Key, typename Lock, typename Allocator> inline HashTable<
        Object, Key, Lock, Allocator>::HashTable()
{
}
#endif

/**
 * Bob Jenkins hash function
 * http://burtleburtle.net/bob/hash/doobs.html
 */
static inline uint_fast32_t one_at_a_time(uint8_t *key, uint_fast32_t len, uint_fast32_t seed = 0)
{
	uint_fast32_t   hash, i;

	for (uint_fast32_t hash=0, i=0; i<len; ++i)
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
 * A simple implementation of an allocator for testing
 * purposes
 */
class TrivialAllocator
{
	void *alloc(uint_fast32_t size)
	{
		return new uint8_t[](size);
	}

	void free(void *ptr)
	{
		delete[] ptr;
	}
};

/**
 * A simple generic hashable object for testing purposes
 * Type Data can be uint32_t or a UNICODE_STRING
 */
template<typename Data, typename Key> class HashObject
{
public:
	Data data;
	Key key;

	const Key &getKey() const
	{
		return key;
	}

	const uint_fast32_t getKeySize() const
	{
		return sizeof(Key);
	}

	const uint_fast32_t &hash() const
	{
		uint_fast32_t result = one_at_a_time(&key, sizeof(Key));
		return result;
	}

};


