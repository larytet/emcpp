#pragma once


template<typename Object, typename Key, typename Lock> class HashTable
{
public:

    HashTable()
    {
    }

    ~HashTable()
    {
    }

    bool insert(const Key &key, const Object object);
    bool remove(const Key &key);
    bool find(const Key &key, Object &object) const;

    uint_fast32_t size() const;
    uint_fast32_t count() const;
    bool isEmpty() const
    {
    	return (count() == 0);
    }

private:
};

template<typename Object, typename Key, typename Lock> inline HashTable<
        Object, Key, Lock>::HashTable()
{
    static_assert(sizeof(Object) <= sizeof(uintptr_t), "HashTable is intended to work only with integer types or pointers");
}


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


