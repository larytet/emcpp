#pragma once

static const char *LOG_LEVEL_NAME[] = {"INFO", "ERROR"};

#undef LOG_INFO
#undef LOG_ERROR
#define LOG_INFO(fmt, ...) log_print(__LINE__, LOG_LEVEL_INFO, fmt, ##__VA_ARGS__ )
#define LOG_ERROR(fmt, ...) log_print(__LINE__, LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__ )

static inline void log_print(int line, int level, const char *fmt, ...)
{
    va_list ap;

    printf("%s: line=%d, msg=", LOG_LEVEL_NAME[level], line);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

#undef LOG_INFO
#undef LOG_ERROR
#define LOG_CAT(fmt, ...) print_log("%s %d" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

static inline void print_log(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}


#if 0
template <int Level> class Log {
public:
    Log(int line, const char *fmt, ...) {
        va_list ap;

        printf("%s: line=%d, msg=", LOG_LEVEL_NAME[Level], line);
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
};
#endif

#undef LOG_INFO
#undef LOG_ERROR
#define LOG_INFO(fmt, ...) Log<LOG_LEVEL_INFO>(__LINE__, fmt, ##__VA_ARGS__ )
#define LOG_ERROR(fmt, ...) Log<LOG_LEVEL_ERROR>(__LINE__, fmt, ##__VA_ARGS__ )


class Log {
public:
    Log(const char *level) : level(level) {}

    void print(int line, const char *fmt, ...) const {
        va_list ap;

        printf("%s: line=%d, msg=", level, line);
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
protected:
    const char *level;
};

const Log LogInfo("INFO");
const Log LogError("ERROR");
#undef LOG_INFO
#undef LOG_ERROR
#define LOG_INFO(fmt, ...) LogInfo.print(__LINE__, fmt, ##__VA_ARGS__ )
#define LOG_ERROR(fmt, ...) LogError.print(__LINE__, fmt, ##__VA_ARGS__ )

void sendData(const int data) {
    cout << hex << data << " ";
}
void sendDataStart() {cout << endl;}
void sendDataEnd() {cout << endl;}
void sendData(const int *data, int count) {
    for (int i = 0;i < count;i++) {
        cout << hex << data[i] << " ";
    }
}

class BinaryLog {
public:
    BinaryLog(int fileId, int line, int count, ...);
    BinaryLog(void *address, int count, ...);
    BinaryLog(int count, ...);
};


BinaryLog::BinaryLog(int fileId, int line, int count, ...) {
    const int HEADER_SIZE = 3;
    int header[HEADER_SIZE];

    header[0] = fileId;
    header[1] = line;
    header[2] = count;
    sendDataStart();
    sendData(header, HEADER_SIZE);
    va_list ap;
    va_start(ap, count);
    for (int j=0; j < count; j++) {
        int arg = va_arg(ap, int);
        sendData(arg);
    }
    va_end(ap);
    sendDataEnd();
}

constexpr int hashData(const char* s, int accumulator) {
    return *s ? hashData(s + 1, (accumulator << 1) | *s) : accumulator;
}

constexpr int hashMetafunction(const char* s) {
    return hashData(s, 0);
}

constexpr int FILE_ID = hashMetafunction(__FILE__);

#define ARGUMENTS_COUNT(...)  (sizeof((int[]){__VA_ARGS__})/sizeof(int))
#undef LOG_INFO
#undef LOG_ERROR
#define LOG_INFO(fmt, ...) BinaryLog(FILE_ID, __LINE__, ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ )
#define LOG_ERROR(fmt, ...) BinaryLog(FILE_ID, __LINE__, ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ )

void testBinaryLog1(void) {
    LOG_INFO("This is info %d %d", 1, 2);
    LOG_ERROR("This is error %d %d %d", 0, 1, 2);
}



BinaryLog::BinaryLog(void *address, int count, ...) {
    const int HEADER_SIZE = 2;
    int header[HEADER_SIZE];

    header[0] = ((uintptr_t)address) & INTMAX_MAX;
    header[1] = count;
    sendDataStart();
    sendData(header, HEADER_SIZE);
    va_list ap;
    va_start(ap, count);
    for (int j=0; j < count; j++) {
        int arg = va_arg(ap, int);
        sendData(arg);
    }
    va_end(ap);
    sendDataEnd();
}

#undef LOG_INFO
#undef LOG_ERROR

#define TOKEN_CAT(x, y) x ## y
#define TOKEN_CAT2(x, y) TOKEN_CAT(x, y)
#define LABEL TOKEN_CAT2 (logLabel_, __LINE__)

#define LOG_INFO(fmt, ...) {  \
        LABEL:\
        BinaryLog(&&LABEL, ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ ); \
}

#define LOG_ERROR(fmt, ...) {\
        LABEL:\
        BinaryLog(&&LABEL, ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ );\
}

void testBinaryLog2(void) {
    LOG_INFO("This is info %d %d", 1, 2);
    LOG_ERROR("This is error %d %d %d", 0, 1, 2);
}


class FastLog {
public:
    FastLog(int count, ...);
};

FastLog::FastLog(int count, ...) {
    const int HEADER_SIZE = 2;
    int header[HEADER_SIZE];

    void *retAddress =
       __builtin_extract_return_addr(
       __builtin_return_address(0));
    header[0] = ((uintptr_t)retAddress) & INTMAX_MAX;
    header[1] = count;
    sendDataStart();
    sendData(header, 2);
    va_list ap;
    va_start(ap, count);
    for (int j=0; j < count; j++) {
        int arg = va_arg(ap, int);
        sendData(arg);
    }
    va_end(ap);
    sendDataEnd();
}

template <typename Lock> class SystemLog {
public:
    SystemLog(int count, ...);
};


#undef LOG_INFO
#undef LOG_ERROR

#define LOG_INFO(fmt, ...) \
        FastLog(ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ );


#define LOG_ERROR(fmt, ...) \
        FastLog(ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ );


void testBinaryLog3(void) {
    LOG_INFO("This is info %d %d", 1, 2);
    LOG_ERROR("This is error %d %d %d", 0, 1, 2);
}
