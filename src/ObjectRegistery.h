#pragma once

template<typename Object, int Size>
class ObjectRegistry
{
public:
    void addRegistration(Object o)
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

    void removeRegistration(Object o)
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
    Object registry[Size];
};
