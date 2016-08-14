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

protected:
    static Object registry[Size];
};

template<typename Object, int Size> Object ObjectRegistry<Object, Size>::registry[Size];

