#pragma once

template<typename Object, int Size>
class ObjectRegistry
{
public:
    static void addRegistration(Object o)
    {
        static_assert(sizeof(Object) <= sizeof(uintptr_t), "ObjectRegistry is intended to work only with pointers");
        for (int i = 0; i < Size; i++)
        {
            if (registry[i] == nullptr)
            {
                registry[i] = o;
                break;
            }
        }
    }

    static void removeRegistration(Object o)
    {
        for (int i = 0; i < Size; i++)
        {
            if (registry[i] == o)
            {
                registry[i] = nullptr;
            }
        }
    }
    enum GetNextResult
    {
        GETNEXT_FAILED,
        GETNEXT_OK,
        GETNEXT_END_TABLE
    };


    static enum GetNextResult getNext(uint_fast32_t &index, Object *o)
    {
        enum GetNextResult result = GETNEXT_END_TABLE;
        Object *entry = &registry[index];
        for (uint_fast32_t i = index;i < Size;i++)
        {
            if (*entry != nullptr)
            {
                *o = *entry;
                index = i;
                result = GETNEXT_OK;
                break;
            }
            entry++;
        }
        return result;
    }

protected:
    static Object registry[Size];
};


template<typename Object, int Size> Object ObjectRegistry<Object, Size>::registry[Size];
