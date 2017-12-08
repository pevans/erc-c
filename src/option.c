/*
 * options.c
 */

#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include "option.h"

/*
 * These are the file inputs we may have to the system. What their
 * contents are will vary based on emulation context, and these values
 * may change through the course of the program's execution.
 */
static FILE *input1 = NULL;
static FILE *input2 = NULL;

/*
 * The size of our error buffer for anything we want to record while our
 * option parsing goes on.
 */
#define ERRBUF_SIZE 2048

/*
 * The alluded-to error buffer.
 */
static char error_buffer[ERRBUF_SIZE] = "";

/*
 * These are all of the options we allow in our long-form options. It's
 * a bit faster to identify them by integer symbols than to do string
 * comparisons.
 */
enum options {
    HELP,
    DISK1,
    DISK2,
};

/*
 * Here are the options we support for program execution.
 */
static struct option long_options[] = {
    { "disk1", 1, NULL, DISK1 },
    { "disk2", 1, NULL, DISK2 },
    { "help", 0, NULL, HELP },
};

/*
 * This simply returns the pointer of our error buffer, with the
 * qualification that the caller cannot modify the error buffer once
 * they get the pointer back.
 */
const char *
option_get_error()
{
    return error_buffer;
}

/*
 * Directly set the error buffer with something (that has to be less
 * than ERRBUF_SIZE). This function is not itself hugely useful, but
 * does allow for some better testing on option_error().
 */
void
option_set_error(const char *str)
{
    // Use `- 1` so that we can ensure error_buffer is NUL-terminated.
    // Otherwise, strncpy will copy all of src but leave the dst
    // unterminated. Not that we're _likely_ to so self-injure
    // ourselves, but, ya know.
    strncpy(error_buffer, str, ERRBUF_SIZE - 1);
}

/*
 * Parse our command-line arguments from the given argc and argv. This
 * may or may not be what the kernel passes into the main() function!
 * We return 1 if we can continue beyond the option parse phase, and 0
 * if something came up to cause us to exit early. But whether you
 * actually exit early is left up to the caller.
 */
int
option_parse(int argc, char **argv)
{
    int index;
    int opt = -1;

    // To begin with, let's (effectively) NUL-out the error buffer.
    error_buffer[0] = '\0';

    do {
        int input_source = 0;

        opt = getopt_long_only(argc, argv, "", long_options, &index);

        switch (opt) {
            case DISK1:
                input_source = 1;
                break;

            case DISK2:
                input_source = 2;
                break;

            case HELP:
                option_print_help();
                
                // The help option should terminate normal execution
                return 0;
        }

        // We seem to have a request to load a file, so let's do so.
        if (input_source) {
            if (!option_read_file(input_source, optarg)) {
                return 0;
            }
        }
    } while (opt != -1);

    return 1;
}

/*
 * Given a file path, we open a FILE stream and set our given input
 * source to that stream. If we cannot do so, we will set an error
 * string in the buffer. Assuming all goes well, we will return 1, and
 * 0 if not.
 */
int
option_read_file(int source, const char *file)
{
    FILE *stream;

    if (!file) {
        snprintf(error_buffer,
                 ERRBUF_SIZE, 
                 "No file given for --disk%d\n", 
                 source);
        return 0;
    }

    stream = fopen(file, "r+");
    if (stream == NULL) {
        snprintf(error_buffer,
                 ERRBUF_SIZE,
                 "--disk%d: %s", 
                 source, 
                 strerror(errno));
        return 0;
    }

    option_set_input(source, stream);

    return 1;
}

/*
 * Return the FILE stream for a given input, or NULL if none can be
 * found. NULL may also be returned if the input has not previously been
 * assigned.
 */
FILE *
option_get_input(int source)
{
    switch (source) {
        case 1: return input1;
        case 2: return input2;
    }

    return NULL;
}

/*
 * Set the given input source to a given FILE stream. If the input
 * source is invalid, then nothing is done.
 */
void
option_set_input(int source, FILE *stream)
{
    switch (source) {
        case 1: input1 = stream;
        case 2: input2 = stream;
    }
}

/*
 * Print out a help message. You'll note this is not automatically
 * generated; it must be manually updated as we add other options.
 */
void
option_print_help()
{
    fprintf(stderr, "Usage: erc [options...]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\
  --disk1=FILE                Load FILE into disk drive 1\n\
  --disk2=FILE                Load FILE into disk drive 2\n\
  --help                      Print this help message\n");
}
