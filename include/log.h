#ifndef _LOG_H_
#define _LOG_H_

#define LOG_FILENAME "/tmp/emp.log"

extern void log_write(int, const char *, ...);
extern void log_close();
extern void log_open();

#define log_critical(...) log_write(0, __VA_ARGS__)
#define log_error(...) log_write(0, __VA_ARGS__)

#endif
