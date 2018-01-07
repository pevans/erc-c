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
static FILE *_stream = NULL;

/*
 * Close the file stream we opened (or were given) in `log_open()`.
 * Nota bene: if you passed stdout into log_open(), this will actually
 * _close_ the stdout stream (!).
 */
int
log_close()
{
    int rval = 0;

    if (_stream != NULL) {
        rval = fclose(_stream);
        _stream = NULL;
    }

    return rval;
}

/*
 * This function will assign the _stream variable to either a given
 * file stream, or if none given, to the default location (which is a
 * filename defined by the `LOG_FILENAME` macro).
 */
void
log_open(FILE *stream)
{
    // Oh, you're telling _me_ what the stream is? Neat!
    if (stream != NULL) {
        _stream = stream;
        return;
    }

    // Oh, well. I'll just open this dang thing.
    _stream = fopen(LOG_FILENAME, "w");
    if (_stream == NULL) {
        perror("Couldn't open log file (" LOG_FILENAME ")");
        exit(1);
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

    if (_stream == NULL) {
        _stream = stdout;
    }

    va_start(ap, fmt);
    vfprintf(_stream, fmt, ap);
    fprintf(_stream, "\n");
    va_end(ap);
}

/*
 * Return the file stream we're currently using for logging.
 */
FILE *
log_stream()
{
    return _stream;
}
