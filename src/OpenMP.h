#pragma once



static inline void encrypt(int idx, const uint8_t *src, uint8_t *dst, int size) {
    memcpy(dst, src, size);
    cout << "Completed " << dec << idx << endl;
}

static inline void encryptPacket(const uint8_t *src, uint8_t *dst, int size) {
    int blockSize = size/2;
    #pragma omp parallel sections
    {
        encrypt(0, src, dst, blockSize);
        #pragma omp section
        encrypt(1, src+blockSize, dst+blockSize, blockSize);
    }
}

