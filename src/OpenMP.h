#pragma once



static inline void encrypt(const uint8_t *src, uint8_t *dst, int size) {
    memcpy(dst, src, size);
}

static inline void encryptPacket(const uint8_t *src, uint8_t *dst, int size) {
    int blockSize = size/2;
    #pragma omp parallel sections
    {
        encrypt(src, dst, blockSize);
        #pragma omp section
        encrypt(src+blockSize, dst+blockSize, blockSize);
    }
}

