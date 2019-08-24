/*
 * \file: slam_log.h
 * \brief: Created by hushouguo at 17:04:42 Aug 07 2019
 */
 
#ifndef __SLAM_LOG_H__
#define __SLAM_LOG_H__

enum {
    LOG_LEVEL_DEBUG = 1, 
    LOG_LEVEL_TRACE, 
    LOG_LEVEL_ALARM, 
    LOG_LEVEL_ERROR, 
    LOG_LEVEL_MAX
};

enum {
    BLACK   =   30, 
    RED     =   31, 
    GREEN   =   32, 
    BROWN   =   33, 
    BLUE    =   34, 
    MAGENTA =   35, 
    CYAN    =   36, 
    GREY    =   37, 
    LRED    =   41, 
    LGREEN  =   42, 
    YELLOW  =   43, 
    LBLUE   =   44, 
    LMAGENTA=   45, 
    LCYAN   =   46, 
    WHITE   =   47  	
};

#define LOG_LEVEL                       LOG_LEVEL_DEBUG
#define ENABLE_ERROR_LOCATION           0
#define ENABLE_BACKTRACE				0	
#define ENABLE_CONSOLE_COLOR            1
#define LOG_TIMESTAMP_FORMAT            "%y/%02m/%02d %02H:%02M:%02S"
#define BACKTRACE_DEPTH	                30

#if ENABLE_BACKTRACE
#define log_backtrace\
	do {\
		void* array[BACKTRACE_DEPTH];\
		int i, size = backtrace(array, BACKTRACE_DEPTH);\
		char** symbols = (char**) backtrace_symbols(array, size);\
		for (i = 0; i < size; ++i) { \
			fprintf(stderr, "\t%d %s\n", i, symbols[i]);\
		}\
	}while(0)
#else
#define log_backtrace
#endif

#if ENABLE_CONSOLE_COLOR
#define log_set_console_color(COLOR)\
    if (!__slam_main->runasdaemon) {\
        fprintf(stderr, "\x1b[%d%sm", (COLOR >= LRED ? (COLOR - 10) : COLOR), (COLOR >= LRED ? ";1" : ""));\
    }
#define log_reset_console_color\
    if (!__slam_main->runasdaemon) {\
        fprintf(stderr, "\x1b[0m");\
    }
#else
#define log_set_console_color(COLOR)
#define log_reset_console_color
#endif

#define log_time_prefix \
    char time_buffer[96];\
    uint64_t ms = slam_milliseconds();\
    slam_timestamp(time_buffer, sizeof(time_buffer), ms / 1000, "[" LOG_TIMESTAMP_FORMAT);    

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define log_debug(MESSAGE, ...)	\
    do {\
        log_time_prefix\
        fprintf(stdout, "%s|%03ld] " MESSAGE "\n", time_buffer, ms % 1000, ##__VA_ARGS__);\
        fflush(stdout);\
    } while(false)
#else
#define log_debug(MESSAGE, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define log_trace(MESSAGE, ...)	\
    do {\
        log_time_prefix\
        fprintf(stdout, "%s|%03ld] " MESSAGE "\n", time_buffer, ms % 1000, ##__VA_ARGS__);\
		fflush(stdout);\
    } while(false)
#else
#define log_trace(MESSAGE, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ALARM
#define log_alarm(MESSAGE, ...)	\
    do {\
        log_time_prefix\
        log_set_console_color(YELLOW)\
        fprintf(stderr, "%s|%03ld] " "slam.alarm: " MESSAGE "\n", time_buffer, ms % 1000, ##__VA_ARGS__);\
        log_reset_console_color\
        log_backtrace;\
        fflush(stderr);\
    } while(false)
#else
#define log_alarm(MESSAGE, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#if ENABLE_ERROR_LOCATION
#define log_error(MESSAGE, ...)	\
    do {\
        log_time_prefix\
        log_set_console_color(LRED)\
        fprintf(stderr, "%s|%03ld] " "slam.error (%s:%d): " MESSAGE "\n", time_buffer, ms % 1000, __FILE__, __LINE__, ##__VA_ARGS__);\
        log_reset_console_color\
        log_backtrace;\
        fflush(stderr);\
    } while(false)
#else
#define log_error(MESSAGE, ...)	\
    do {\
        log_time_prefix\
        log_set_console_color(LRED)\
        fprintf(stderr, "%s|%03ld] " "slam.error: " MESSAGE "\n", time_buffer, ms % 1000, ##__VA_ARGS__);\
        log_reset_console_color\
        log_backtrace;\
        fflush(stderr);\
    } while(false)
#endif
#else
#define log_error(MESSAGE, ...)
#endif

#define CHECK_RETURN(RC, RESULT, MESSAGE, ...) do { if (!(RC)) { log_error(MESSAGE, ##__VA_ARGS__); return RESULT; } } while (false)
#define CHECK_GOTO(RC, SYMBOL, MESSAGE, ...) do { if (!(RC)) { log_error(MESSAGE, ##__VA_ARGS__); goto SYMBOL; } } while (false)
#define CHECK_BREAK(RC, MESSAGE, ...) if (!(RC)) { log_error(MESSAGE, ##__VA_ARGS__); break; }
#define CHECK_CONTINUE(RC, MESSAGE, ...) if (!(RC)) { log_error(MESSAGE, ##__VA_ARGS__); continue; }

#define CHECK_DEBUG(RC, MESSAGE, ...) if (!(RC)) { log_debug(MESSAGE, ##__VA_ARGS__); }
#define CHECK_TRACE(RC, MESSAGE, ...) if (!(RC)) { log_trace(MESSAGE, ##__VA_ARGS__); }
#define CHECK_ALARM(RC, MESSAGE, ...) if (!(RC)) { log_alarm(MESSAGE, ##__VA_ARGS__); }
#define CHECK_ERROR(RC, MESSAGE, ...) if (!(RC)) { log_error(MESSAGE, ##__VA_ARGS__); }

struct slam_log_s {
    int fd;
    const char* filename;
};

typedef struct slam_log_s slam_log_t;

extern slam_log_t* slam_log_new(const char* filename);
extern void slam_log_delete(slam_log_t* log);

#endif
