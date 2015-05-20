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

extern void sendData(const int *data, int count);

template<int MAX_ARGUMENTS_COUNT> class BinaryLog {
public:
    BinaryLog(char *, int fileId, int line, int count, ...);
    BinaryLog(void *address, int line, int count, ...);
    BinaryLog(int count, ...);
};

template<int MAX_ARGUMENTS_COUNT>
BinaryLog<MAX_ARGUMENTS_COUNT>::BinaryLog(char *c, int fileId, int line, int count, ...) {
    const int HEADER_SIZE = 3;
    int args[MAX_ARGUMENTS_COUNT+HEADER_SIZE];

    if (count > MAX_ARGUMENTS_COUNT) {
        count = MAX_ARGUMENTS_COUNT;
    }
    args[0] = fileId;
    args[1] = line;
    args[2] = count;
    va_list ap;
    va_start(ap, count);
    int arguments = count+HEADER_SIZE;
    for (int j=HEADER_SIZE; j < arguments; j++) {
        args[j] = va_arg(ap, int);
    }
    va_end(ap);
    sendData(args, arguments);
}

#define ARGUMENTS_COUNT(...)  (sizeof((int[]){__VA_ARGS__})/sizeof(int))
#undef LOG_INFO
#undef LOG_ERROR
#define LOG_INFO(fmt, ...) BinaryLog<3>(FILE_ID, __LINE__, ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ )
#define LOG_ERROR(fmt, ...) BinaryLog<3>(FILE_ID, __LINE__, ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ )

constexpr int hashData(const char* s, int accumulator) {
    return *s ? hashData(s + 1, (accumulator << 1) | *s) : accumulator;
}

constexpr int hashMetafunction(const char* s) {
    return hashData(s, 0);
}

constexpr int FILE_ID = hashMetafunction(__FILE__);

template<int MAX_ARGUMENTS_COUNT>
BinaryLog<MAX_ARGUMENTS_COUNT>::BinaryLog(void *address, int line, int count, ...) {
    const int HEADER_SIZE = 2;
    int args[MAX_ARGUMENTS_COUNT+HEADER_SIZE];

    if (count > MAX_ARGUMENTS_COUNT) {
        count = MAX_ARGUMENTS_COUNT;
    }
    args[0] = ((uintptr_t)address) & INTMAX_MAX;
    args[1] = count;
    va_list ap;
    va_start(ap, count);
    int arguments = count+HEADER_SIZE;
    for (int j=HEADER_SIZE; j < arguments; j++) {
        args[j] = va_arg(ap, int);
    }
    va_end(ap);
    sendData(args, arguments);
}

#undef LOG_INFO
#undef LOG_ERROR

#define TOKEN_CAT(x, y) x ## y
#define TOKEN_CAT2(x, y) TOKEN_CAT(x, y)
#define LABEL TOKEN_CAT2 (logLabel_, __LINE__)

#define LOG_INFO(fmt, ...) {  \
        LABEL:\
        BinaryLog<3>(&&LABEL, __LINE__, ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ ); \
}

#define LOG_ERROR(fmt, ...) {\
        LABEL:\
        BinaryLog<3>(&&LABEL, __LINE__, ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ );\
}



template<int MAX_ARGUMENTS_COUNT>
BinaryLog<MAX_ARGUMENTS_COUNT>::BinaryLog(int count, ...) {
    const int HEADER_SIZE = 2;
    int args[MAX_ARGUMENTS_COUNT+HEADER_SIZE];

    if (count > MAX_ARGUMENTS_COUNT) {
        count = MAX_ARGUMENTS_COUNT;
    }

    void *retAddress = __builtin_extract_return_addr(__builtin_return_address(0));
    args[0] = ((uintptr_t)retAddress) & INTMAX_MAX;
    args[1] = count;
    va_list ap;
    va_start(ap, count);
    int arguments = count+HEADER_SIZE;
    for (int j=HEADER_SIZE; j < arguments; j++) {
        args[j] = va_arg(ap, int);
    }
    va_end(ap);
    sendData(args, arguments);
}

#undef LOG_INFO
#undef LOG_ERROR

#define LOG_INFO(fmt, ...) {  \
        BinaryLog<3>(ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ ); \
}

#define LOG_ERROR(fmt, ...) {\
        BinaryLog<3>(ARGUMENTS_COUNT(__VA_ARGS__), __VA_ARGS__ );\
}
