/*
 * option.c
 * 
 * This file contains the functions which support our CLI options; you
 * are both able to parse options and retrieve information from that
 * option parsing.
 */

#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include "option.h"
#include "log.h"

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
 * The default width and height of the window. This is "roughly"
 * 640x480, but because the Apple II has a 4.375:3 aspect ratio, we had
 * to bump up the width to a number that respects that ratio.
 */
static int width = 700;
static int height = 480;

static int flags = 0;

/*
 * These are all of the options we allow in our long-form options. It's
 * a bit faster to identify them by integer symbols than to do string
 * comparisons.
 */
enum options {
    DISK1,
    DISK2,
    HELP,
    SIZE,
    FLASH,
    DISASSEMBLE,
};

/*
 * Here are the options we support for program execution.
 */
static struct option long_options[] = {
    { "disassemble", 0, NULL, DISASSEMBLE },
    { "disk1", 1, NULL, DISK1 },
    { "disk2", 1, NULL, DISK2 },
    { "flash", 0, NULL, FLASH },
    { "help", 0, NULL, HELP },
    { "size", 1, NULL, SIZE },
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
            case DISASSEMBLE:
                flags |= OPTION_DISASSEMBLE;
                break;

            case DISK1:
                input_source = 1;
                break;

            case DISK2:
                input_source = 2;
                break;

            case FLASH:
                flags |= OPTION_FLASH;
                break;

            case HELP:
                option_print_help();
                
                // The help option should terminate normal execution
                return 0;

            case SIZE:
                if (option_set_size(optarg) != OK) {
                    return 0;
                }
                break;
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
  --disassemble               Print assembly notation from CPU memory\n\
  --disk1=FILE                Load FILE into disk drive 1\n\
  --disk2=FILE                Load FILE into disk drive 2\n\
  --flash                     Flash CPU memory with contents of drive 1\n\
  --help                      Print this help message\n\
  --size=WIDTHxHEIGHT         Use WIDTH and HEIGHT for window size\n\
                              (only 700x480 and 875x600 are supported)\n");
}

/*
 * Set the size of our window in visible width and height. The size
 * string is expected to be in the form of WIDTHxHEIGHT, where WIDTH
 * and HEIGHT are integers representing the window width and height
 * respectively. Arbitrary values for width and height are not
 * necessarily allowed; generally, we can only support values which
 * respect the aspect ratio used by the machine we are emulating.
 */
int
option_set_size(const char *size)
{
    if (strcmp(size, "700x480") == 0) {
        return OK;
    } else if (strcmp(size, "875x600") == 0) {
        width = 875;
        height = 600;
        return OK;
    }

    snprintf(error_buffer, ERRBUF_SIZE, "Ignoring bad window size: %s", size);
    return ERR_BADOPT;
}

/*
 * Return the window width we've been configured to use.
 */
int
option_get_width()
{
    return width;
}

/*
 * Return the window height we want.
 */
int
option_get_height()
{
    return height;
}

/*
 * Return true if the given option flag is set.
 */
bool
option_flag(int flag)
{
    return flags & flag;
}
