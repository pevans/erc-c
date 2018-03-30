#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/syslog.h>

#define LOG_FILENAME "/tmp/emp.log"

enum log_errcode {
    OK = 1,
    ERR_OOM,            // out of memory
    ERR_OOB,            // out of bounds
    ERR_BADFILE,
    ERR_BADOPT,         // bad option (e.g. from getopt)
    ERR_INVALID,        // invalid operation
    ERR_GFXINIT,        // couldn't initialize graphics
    ERR_GFXOP,          // we couldn't execute a specific graphic operation
};

extern FILE *log_stream();
extern int log_close();
extern void log_open(FILE *);
extern void log_write(int, const char *, ...);

/*
 * Here we have a couple of convenience macros that abstracts the log
 * level number.
 */
#define log_critical(...) log_write(0, __VA_ARGS__)
#define log_error(...) log_write(0, __VA_ARGS__)

#define log_alert(...) log_write(LOG_ALERT, __VA_ARGS__)
#define log_crit(...) log_write(LOG_CRIT, __VA_ARGS__)
#define log_debug(...) log_write(LOG_DEBUG, __VA_ARGS__)
#define log_emerg(...) log_write(LOG_EMERG, __VA_ARGS__)
#define log_err(...) log_write(LOG_ERR, __VA_ARGS__)
#define log_info(...) log_write(LOG_INFO, __VA_ARGS__)
#define log_notice(...) log_write(LOG_NOTICE, __VA_ARGS__)
#define log_warning(...) log_write(LOG_WARNING, __VA_ARGS__)

#endif
