/*
 * \file: slam_log.h
 * \brief: Created by hushouguo at 17:04:42 Aug 07 2019
 */
 
#ifndef __SLAM_LOG_H__
#define __SLAM_LOG_H__

enum {
    LOG_LEVEL_DEBUG = 1, LOG_LEVEL_TRACE, LOG_LEVEL_ALARM, LOG_LEVEL_ERROR
};

#define LOG_LEVEL                       LOG_LEVEL_DEBUG
#define ENABLE_ERROR_LOCATION           1
#define LOG_TIMESTAMP_FORMAT            "%y/%02m/%02d %02H:%02M:%02S"

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define Debug(MESSAGE, ...)	\
    do {\
        char time_buffer[96];\
        uint64_t ms = slam_milliseconds();\
        slam_timestamp(time_buffer, sizeof(time_buffer), ms / 1000, "[" LOG_TIMESTAMP_FORMAT);\
        fprintf(stdout, "%s|%03ld] " MESSAGE "\n", time_buffer, ms % 1000, ##__VA_ARGS__);\
    } while(false)
#else
#define Debug(MESSAGE, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define Trace(MESSAGE, ...)	\
    do {\
        char time_buffer[96];\
        uint64_t ms = slam_milliseconds();\
        slam_timestamp(time_buffer, sizeof(time_buffer), ms / 1000, "[" LOG_TIMESTAMP_FORMAT);\
        fprintf(stdout, "%s|%03ld] " MESSAGE "\n", time_buffer, ms % 1000, ##__VA_ARGS__);\
    } while(false)
#else
#define Trace(MESSAGE, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ALARM
#define Alarm(MESSAGE, ...)	\
    do {\
        char time_buffer[96];\
        uint64_t ms = slam_milliseconds();\
        slam_timestamp(time_buffer, sizeof(time_buffer), ms / 1000, "[" LOG_TIMESTAMP_FORMAT);\
        fprintf(stderr, "%s|%03ld] " "slam.alarm: " MESSAGE "\n", time_buffer, ms % 1000, ##__VA_ARGS__);\
    } while(false)
#else
#define Alarm(MESSAGE, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#if ENABLE_ERROR_LOCATION
#define Error(MESSAGE, ...)	\
    do {\
        char time_buffer[96];\
        uint64_t ms = slam_milliseconds();\
        slam_timestamp(time_buffer, sizeof(time_buffer), ms / 1000, "[" LOG_TIMESTAMP_FORMAT);\
        fprintf(stderr, "%s|%03ld] " "slam.error (%s:%d): " MESSAGE "\n", time_buffer, ms % 1000, __FILE__, __LINE__, ##__VA_ARGS__);\
    } while(false)
#else
#define Error(MESSAGE, ...)	\
    do {\
        char time_buffer[96];\
        uint64_t ms = slam_milliseconds();\
        slam_timestamp(time_buffer, sizeof(time_buffer), ms / 1000, "[" LOG_TIMESTAMP_FORMAT);\
        fprintf(stderr, "%s|%03ld] " "slam.error: " MESSAGE "\n", time_buffer, ms % 1000, ##__VA_ARGS__);\
    } while(false)
#endif
#else
#define Error(MESSAGE, ...)
#endif

#define CHECK_RETURN(RC, RESULT, MESSAGE, ...) do { if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); return RESULT; } } while (false)
#define CHECK_GOTO(RC, SYMBOL, MESSAGE, ...) do { if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); goto SYMBOL; } } while (false)
#define CHECK_BREAK(RC, MESSAGE, ...) if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); break; }
#define CHECK_CONTINUE(RC, MESSAGE, ...) if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); continue; }
#define CHECK_DEBUG(RC, MESSAGE, ...) if (!(RC)) { Debug(MESSAGE, ##__VA_ARGS__); }
#define CHECK_ALARM(RC, MESSAGE, ...) if (!(RC)) { Alarm(MESSAGE, ##__VA_ARGS__); }
#define CHECK_ERROR(RC, MESSAGE, ...) if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); }

struct slam_log_s {
    int fd;
    const char* filename;
};

typedef struct slam_log_s slam_log_t;

extern slam_log_t* slam_log_new(const char* filename);
extern void slam_log_delete(slam_log_t* log);

#endif
