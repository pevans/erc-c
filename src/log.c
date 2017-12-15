/*
 * log.c
 *
 * The functions here support our logging mechanism.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "log.h"

/*
 * This is the file stream we will write to for our logging. It can be
 * any number of things, from a literal file on the filesystem, to
 * stdout.
 */
static FILE *log_stream = NULL;

/*
 * This function will assign the log_stream variable to either a given
 * file stream, or if none given, to the default location (which is a
 * filename defined by the `LOG_FILENAME` macro).
 */
void
log_open(FILE *stream)
{
    // Oh, you're telling _me_ what the stream is? Neat!
    if (stream != NULL) {
        log_stream = stream;
        return;
    }

    // Oh, well. I'll just open this dang thing.
    log_stream = fopen(LOG_FILENAME, "w");
    if (log_stream == NULL) {
        perror("Couldn't open log file (" LOG_FILENAME ")");
        exit(1);
    }
}

/*
 * Close the file stream we opened (or were given) in `log_open()`.
 * Nota bene: if you passed stdout into log_open(), this will actually
 * _close_ the stdout stream (!).
 */
void
log_close()
{
    if (log_stream != NULL) {
        fclose(log_stream);
    }
}

/*
 * Write to the log stream. This function can accept variable arguments,
 * ala `printf()`. There is some support for different log levels, but
 * as you may note, we do not use them at present.
 */
void
log_write(int level, const char *fmt, ...)
{
    va_list ap;

    // Oh. Well, we can't write something if we have no log stream,
    // right?
    if (log_stream == NULL) {
        return;
    }

    va_start(ap, fmt);
    vfprintf(log_stream, fmt, ap);
    fprintf(log_stream, "\n");
    va_end(ap);
}
