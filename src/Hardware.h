#pragma once

class HardwareModule {
protected:
    HardwareModule() {
    }
    ~HardwareModule() {
    }
};

template<typename IntegerType> class HardwareRegister {
protected:
    HardwareRegister() {}

    inline uint32_t get() const {
        return value;
    }
    inline void set(uint32_t value) {
        this->value = value;
    }

    volatile IntegerType value;

    static_assert(std::numeric_limits<IntegerType>::is_integer,
            "HardwareRegister works only with integer types");
};

class HardwareRegister32: public HardwareRegister<uint32_t> {
protected:
    HardwareRegister32() {}
};
static_assert((sizeof(HardwareRegister32) == sizeof(uint32_t)),
        "HardwareRegister32 is not 32 bits");

class HardwareRegister32RO: public HardwareRegister32 {
public:
    uint32_t operator=(const HardwareRegister32RO& r) const {
        return get();
    }
};
static_assert((sizeof(HardwareRegister32RO) == sizeof(uint32_t)),
        "HardwareRegister32RO is not 32 bits");

class HardwareRegister32WO: public HardwareRegister32 {
public:
    uint32_t operator=(uint32_t value) {
        set(value);
        return value;
    }
};
static_assert((sizeof(HardwareRegister32WO) == sizeof(uint32_t)),
        "HardwareRegister32WO is not 32 bits");

class HardwareRegister32RW: public HardwareRegister32 {
public:
    uint32_t operator=(const HardwareRegister32RO& r) const {
        return get();
    }
    uint32_t operator=(uint32_t value) {
        set(value);
        return value;
    }
};
static_assert((sizeof(HardwareRegister32RW) == sizeof(uint32_t)),
        "HardwareRegister32RW is not 32 bits");

class HardwareRegister32NotUsed: HardwareRegister32 {
public:
    HardwareRegister32NotUsed() {
    }
    ~HardwareRegister32NotUsed() {
    }
};
static_assert((sizeof(HardwareRegister32NotUsed) == sizeof(uint32_t)),
        "HardwareRegister32NotUsed is not 32 bits");

