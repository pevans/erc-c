#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define LOG_FILENAME "/tmp/emp.log"

enum log_errcode {
    OK = 1,
    ERR_OOM,            // out of memory
    ERR_OOB,            // out of bounds
    ERR_BADFILE,
};

extern void log_write(int, const char *, ...);
extern void log_close();
extern void log_open(FILE *);

/*
 * Here we have a couple of convenience macros that abstracts the log
 * level number.
 */
#define log_critical(...) log_write(0, __VA_ARGS__)
#define log_error(...) log_write(0, __VA_ARGS__)

#endif
