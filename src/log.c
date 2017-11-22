#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static FILE *log_stream = NULL;

void
log_open()
{
    log_stream = fopen("/tmp/emp.log", "w");
    if (log_stream == NULL) {
        perror("Couldn't open log file (/tmp/emp.log)");
        exit(1);
    }
}

void
log_close()
{
    if (log_stream != NULL) {
        fclose(log_stream);
    }
}

void
log_write(int level, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(log_stream, fmt, ap);
    fprintf(log_stream, "\n");
    va_end(ap);
}
