#pragma once

template<typename Key> class HashObject
{
public:
	Key key;

	const Key &getKey() const
	{
		return key;
	}

	const uint64_t &hash() const
	{
		return 0;
	}

	uint_fast32_t one_at_a_time(uint8_t *key, uint_fast32_t len, uint_fast32_t seed = 0)
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
};

template<typename ObjectType, typename Key, typename Lock> class HashTable
{
public:

    HashTable()
    {

    }

    ~HashTable()
    {
    }


private:
};
