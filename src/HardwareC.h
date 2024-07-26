#pragma once


#define TOKEN_CAT(x, y) x ## y
#define TOKEN_CAT2(x, y) TOKEN_CAT(x, y)
#define RESERVED TOKEN_CAT2(reserved, __COUNTER__)

#pragma pack(push)
#pragma pack(1)
struct PIO {
    volatile uint32_t PIO_PER  ;
    volatile uint32_t PIO_PDR  ;
    volatile uint32_t PIO_PSR  ;
    volatile uint32_t RESERVED ;
    volatile uint32_t PIO_OER  ;
    volatile uint32_t PIO_ODR  ;
    volatile uint32_t PIO_OSR  ;
    volatile uint32_t RESERVED ;
    volatile uint32_t RESERVED ;
    volatile uint32_t RESERVED ;
    volatile uint32_t RESERVED ;
    volatile uint32_t RESERVED ;
    volatile uint32_t PIO_SODR ;
    volatile uint32_t PIO_CODR ;
};
#pragma pack(pop)

typedef enum {
    PIO_A,
    PIO_B,
    PIO_C,
    PIO_D,
    PIO_E,
    PIO_F,
} PIO_NAME;

