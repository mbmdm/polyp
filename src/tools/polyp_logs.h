#ifndef POLYP_LOGS_H
#define POLYP_LOGS_H

#include <cstdio>
#include <cassert>
#include <stdint.h>

enum class LogType
{
    ToDo,
    Debug,
    Log,
    Warning,
    Error,
    Fatal,
    Count,
};

inline void polyp_direct(LogType type, const char* project, const char* file, unsigned int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    const char* typestr[] = {
        "TODO:    ", "Debug:   ", "Log:     ", "Warning: ", "Error:   ", "Fatal:   "
    };

    printf("%s ",  project);                             /// Temporal logging solution
    printf("%s",  typestr[static_cast<int>(type)]);      /// Temporal logging solution
    if (type == LogType::ToDo || type == LogType::Fatal || type == LogType::Error ) {
        printf("[%s:%d] ", file, line); /// Temporal logging solution
    }
    if (type == LogType::ToDo && strlen(fmt) == 0) {
        printf("Not implemented yet!");                  /// Temporal logging solution
    }
    vprintf(fmt,  args);                                 /// Temporal logging solution
    printf("\n");                                        /// Temporal logging solution

    va_end(args);

    if (type == LogType::Fatal) {
        exit(1);
    }
}

#if !defined(POLYPLOG_PROJECT)
#define POLYPLOG_PROJECT "POLYP"
#endif

#define polyplog(type, ...)                                            \
polyp_direct(type, POLYPLOG_PROJECT, __FILE__, __LINE__, __VA_ARGS__)

#define POLYPINFO(...)   polyplog(LogType::Log,     __VA_ARGS__)
#define POLYPDEBUG(...)  polyplog(LogType::Debug,   __VA_ARGS__)
#define POLYPWARN(...)   polyplog(LogType::Warning, __VA_ARGS__)
#define POLYPERROR(...)  polyplog(LogType::Error,   __VA_ARGS__)
#define POLYPFATAL(...)  polyplog(LogType::Fatal,   __VA_ARGS__)
#define POLYPTODO(...)   polyplog(LogType::ToDo,    __VA_ARGS__);

#define POLYPASSERT(...)                     \
if (!__VA_ARGS__)                            \
polyplog(LogType::Warning, #__VA_ARGS__);    \
assert(__VA_ARGS__);

#define POLYPASSERTNOTEQUAL(lhv, rhv)        \
if (lhv == rhv)                              \
polyplog(LogType::Warning, #lhv" is "#rhv);  \
assert(lhv != rhv);

#ifndef DEBUG
#undef POLYPDEBUG
#undef POLYPASSERT
#define POLYPDEBUG(...)
#define POLYPASSERT(...)
#endif // !DEBUG

#endif // POLYP_LOGS_H 
