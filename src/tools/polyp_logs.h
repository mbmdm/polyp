#ifndef POLYP_LOGS_H
#define POLYP_LOGS_H

#include <cstdio>
#include <cassert>

enum class LogType : uint32_t
{
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
    "Debug:   ", "Log:     ", "Warning: ", "Error:   ", "Fatal:   "
    };

    printf("%s ", project);                             /// Temporal logging solution
    printf("%s", typestr[static_cast<uint32_t>(type)]); /// Temporal logging solution
    vprintf(fmt, args);                                 /// Temporal logging solution
    printf("\n");                                       /// Temporal logging solution

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
#define POLYPERR(...)    polyplog(LogType::Error,   __VA_ARGS__)
#define POLYPFATAL(...)  polyplog(LogType::Fatal,   __VA_ARGS__)
#define POLYPASSERT(...) polyplog(LogType::Log,     __VA_ARGS__); /*assert(false);*/

#endif // POLYP_LOGS_H 
